#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>

#include "parser.h"
#include "../client/api.h"
#include "../common/constants.h"
#include "../common/io.h"

// Estrutura usada para passar argumentos para a thread de notificações.
typedef struct {
  int notif_pipe_fd;       // Descritor de arquivo do pipe de notificações.
  int *aux;                // Ponteiro para uma variável auxiliar para controlo.
  pthread_mutex_t *mutex;  // Mutex para sincronização.
} NotifArgs;


// Função que é executada pela thread para lidar com notificações do servidor.
void* handle_notif(void* args) {
  // Configura a thread para que possa ser cancelada de forma assíncrona.
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcancelstate(PTHREAD_CANCEL_DEFERRED, NULL);

  // Faz o cast dos argumentos recebidos para a estrutura `NotifArgs`.
  NotifArgs *notif_args = (NotifArgs *)args;
  int notif_pipe_fd = notif_args->notif_pipe_fd;  // Obtém o descritor do pipe de notificações.
  int *aux = notif_args->aux;                    // Ponteiro para a variável auxiliar.
  pthread_mutex_t *mutex = notif_args->mutex;    // Mutex para sincronização.

  while (1) {
    // Bloqueia o mutex para verificar o valor da variável auxiliar.
    pthread_mutex_lock(mutex);
    if (*aux == -1) {  // Verifica se recebeu sinal para encerrar a thread.
      pthread_mutex_unlock(mutex);
      break;         // Sai do loop se o sinal for detectado.
    }
    pthread_mutex_unlock(mutex);  // Liberta o mutex.

    char buffer[256];  // Buffer para armazenar a notificação.
    // Lê dados do pipe de notificações.
    ssize_t bytes_read = read(notif_pipe_fd, buffer, sizeof(buffer));
    if (bytes_read <= 0) {  // Verifica se houve erro ou se o pipe foi fechado.
      if (bytes_read == 0 || errno == EPIPE) {
        fprintf(stderr, "Server disconnected (notifications pipe closed)\n");
      } else {
        perror("Error reading from notifications pipe");
      }
      // Fecha o pipe de notificações.
      if (close(notif_pipe_fd) == -1) {
        perror("Error closing request pipe");
      }
      pthread_exit(NULL);  // Encerra a thread.
    }
    // Imprime a notificação recebida.
    printf("Notification: %s\n", buffer);
  }
  // Liberta os recursos alocados para os argumentos.
  free(notif_args);
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <client_unique_id> <register_pipe_path>\n", argv[0]);
    return 1;
  }

  int aux_variable = 0;
  int *aux = &aux_variable;
  pthread_mutex_t aux_mutex;
  pthread_mutex_init(&aux_mutex, NULL);
  
  pthread_t thread;

  char req_pipe_path[256] = "/tmp/req";
  char resp_pipe_path[256] = "/tmp/resp";
  char notif_pipe_path[256] = "/tmp/notif";

  char keys[MAX_NUMBER_SUB][MAX_STRING_SIZE] = {0};
  unsigned int delay_ms;
  size_t num;

  strncat(req_pipe_path, argv[1], strlen(argv[1]) * sizeof(char));
  strncat(resp_pipe_path, argv[1], strlen(argv[1]) * sizeof(char));
  strncat(notif_pipe_path, argv[1], strlen(argv[1]) * sizeof(char));
  
  // Tenta conectar-se ao servidor usando os caminhos especificados.
  if (kvs_connect(req_pipe_path, resp_pipe_path, argv[2], notif_pipe_path) != 0) {
    fprintf(stderr, "Failed to connect to the server\n");
    return 1;
  }

  // Obtém os file descriptors (FDs) dos pipes criados durante a conexão.
  int request_pipe_fd = get_request_pipe_fd();
  int resp_pipe_fd = get_response_pipe_fd();
  int notifications_pipe_fd = get_notifications_pipe_fd();

  while (1) {
    fd_set read_fds;  // Estrutura para monitorar múltiplos FDs.
    FD_ZERO(&read_fds); // Inicializa a estrutura.

    FD_SET(STDIN_FILENO, &read_fds);// Adiciona o FD do stdin para monitorar entrada do usuário.
    FD_SET(resp_pipe_fd, &read_fds); // Adiciona o FD do pipe de resposta para monitorar respostas do servidor.

    // Determina o maior FD para a chamada `select`.
    int max_fd = resp_pipe_fd > STDIN_FILENO ? resp_pipe_fd : STDIN_FILENO;

    // Espera atividade nos FDs especificados.
    int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

    // Verifica se houve erro na chamada `select`.
    if (activity < 0) {
      if (errno == EINTR) { // Interrupção por sinal.
        fprintf(stderr, "Select interrupted by signal\n");
        continue;  // Continue o loop
      }
      perror("Error in select");
      break;
    }

    // Verifica se houve atividade no pipe de resposta.
    if (FD_ISSET(resp_pipe_fd, &read_fds)) {
      char buffer[256];
      ssize_t bytes_read = read(resp_pipe_fd, buffer, sizeof(buffer));
      if (bytes_read <= 0) {  // Verifica se houve erro ou se o pipe foi fechado.
        if (bytes_read == 0 || errno == EPIPE) {
          fprintf(stderr, "Server disconnected (response pipe closed)\n");
          close(request_pipe_fd);
          close(resp_pipe_fd);
          close(notifications_pipe_fd);
          return 1;
        } else {
          perror("Error reading from response pipe");
        }
      } else {
        printf("Received from server: %s\n", buffer);
      }
    }

    // Verifica se houve entrada do usuário.
    if (FD_ISSET(STDIN_FILENO, &read_fds)) {
      switch (get_next(STDIN_FILENO)) {
        case CMD_DISCONNECT:
          if (kvs_disconnect(req_pipe_path, resp_pipe_path, notif_pipe_path) != 0) {
            fprintf(stderr, "Failed to disconnect to the server\n");
            return 1;
          }

          printf("Disconnected from server\n");
          return 0;

        case CMD_SUBSCRIBE:
          num = parse_list(STDIN_FILENO, keys, 1, MAX_STRING_SIZE);
          if (num == 0) {
            fprintf(stderr, "Invalid command. See HELP for usage\n");
            continue;
          }
          if (kvs_subscribe(keys[0])) {
            fprintf(stderr, "Command subscribe failed\n");
          }
          // Aloca memória para os argumentos da thread.
          NotifArgs *notif_args = malloc(sizeof(NotifArgs));
          if (notif_args == NULL) {
            perror("Erro ao alocar memória para os argumentos da thread");
            break;
          }
          // Abre o pipe de notificações para leitura.
          notif_args->notif_pipe_fd = open(notif_pipe_path, O_RDONLY);
          if (notif_args->notif_pipe_fd == -1) {
            perror("Error opening notification pipe");
            free(notif_args);
            break;
          }
          notif_args->aux = aux;
          notif_args->mutex = &aux_mutex;

          // Cria a thread para lidar com notificações.
          if (pthread_create(&thread, NULL, handle_notif, notif_args) != 0) {
            perror("Error creating notification thread");
            close(notif_args->notif_pipe_fd);
            free(notif_args);
            break;
          }

          break;

        case CMD_UNSUBSCRIBE:
          num = parse_list(STDIN_FILENO, keys, 1, MAX_STRING_SIZE);
          if (num == 0) {
            fprintf(stderr, "Invalid command. See HELP for usage\n");
            continue;
          }

          if (kvs_unsubscribe(keys[0])) {
            fprintf(stderr, "Command unsubscribe failed\n");
            continue;
          }

          // Sinaliza para a thread de notificações encerrar.
          pthread_mutex_lock(&aux_mutex);
          aux_variable = -1;
          pthread_mutex_unlock(&aux_mutex);

          // Cancela e aguarda a finalização da thread de notificações.
          if (pthread_cancel(thread) != 0) {
            fprintf(stderr, "Failed to cancel thread\n");
            continue;
          }

          pthread_join(thread, NULL); 
            
          aux_variable = 0;

          break;

        case CMD_DELAY:
          if (parse_delay(STDIN_FILENO, &delay_ms) == -1) {
            fprintf(stderr, "Invalid command. See HELP for usage\n");
            continue;
          }

          if (delay_ms > 0) {
            printf("Waiting...\n");
            sleep(delay_ms * 1000);
          }
          break;

        case CMD_INVALID:
          fprintf(stderr, "Invalid command. See HELP for usage\n");
          break;

        case CMD_EMPTY:
          break;

        case EOC:
          break;
      }
    }
  }
}
