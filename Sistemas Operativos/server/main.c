#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>

#include "../common/io.h"
#include "kvs.h"
#include "constants.h"
#include "io.h"
#include "operations.h"
#include "parser.h"
#include "pthread.h"


struct SharedData {
  DIR *dir;
  char *dir_name;
  pthread_mutex_t directory_mutex;
};

typedef struct ClientArgs{
  int req_pipe_fd;
  int resp_pipe_fd;
  int notif_pipe_fd;
  char client_id[MAX_STRING_SIZE];
  struct ClientArgs *next;
} ClientArgs;

ClientArgs *client_list_head = NULL;

pthread_mutex_t client_list_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t n_current_backups_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sessions_lock = PTHREAD_MUTEX_INITIALIZER;

size_t active_backups = 0; // Number of active backups
size_t max_backups;        // Maximum allowed simultaneous backups
size_t max_threads;        // Maximum allowed simultaneous threads
char *jobs_directory = NULL;


sem_t session_semaphore;

// Fecha todos os clientes atualmente conectados e limpa a lista de clientes.
void close_all_clients() {
  pthread_mutex_lock(&client_list_mutex); // Bloqueia o mutex para garantir exclusão mútua.

  ClientArgs *current = client_list_head;
  while (current != NULL) {
    close(current->resp_pipe_fd); // Fecha o pipe de resposta do cliente.
    close(current->notif_pipe_fd);  // Fecha o pipe de notificações do cliente.

    ClientArgs *to_free = current;  // Armazena o nó atual para libertar a memória.
    current = current->next;  // Avança para o próximo cliente na lista.
    free(to_free);  // Liberta a memória do nó atual.
  }

  client_list_head = NULL;  // Define a lista de clientes como vazia.

  pthread_mutex_unlock(&client_list_mutex);  // Liberta o mutex após concluir a operação.
}

// Manipula o sinal SIGUSR1. Fecha todos os clientes e remove as associações da KVS.
void handle_sigusr1(int signum) {
  (void)signum;  // Ignora o parâmetro `signum`, pois ele não é utilizado.
  close_all_clients();  // Fecha todos os clientes e limpa a lista.
  kvs_remove_all_clients();  // Remove todas as associações de clientes na KVS.
}

// Adiciona um novo cliente à lista de clientes conectados.
void add_client(int req_fd, int resp_fd, int notif_fd, const char *client_id) {
  pthread_mutex_lock(&client_list_mutex);  // Bloqueia o mutex para garantir exclusão mútua.

  // Aloca memória para um novo cliente.
  ClientArgs *new_client = (ClientArgs *)malloc(sizeof(ClientArgs));
  if (new_client == NULL) {  // Verifica se a alocação falhou.
    perror("Erro ao alocar memória para cliente");
    pthread_mutex_unlock(&client_list_mutex);  // Libera o mutex antes de retornar.
    return;
  }

  // Inicializa os campos do novo cliente.
  new_client->req_pipe_fd = req_fd;  // Salva o descritor do pipe de requisição.
  new_client->resp_pipe_fd = resp_fd;  // Salva o descritor do pipe de resposta.
  new_client->notif_pipe_fd = notif_fd;  // Salva o descritor do pipe de notificações.
  strncpy(new_client->client_id, client_id, MAX_STRING_SIZE);  // Copia o ID do cliente.
  new_client->next = client_list_head;  // Define o próximo cliente como o início da lista atual.

  client_list_head = new_client;  // Atualiza o início da lista com o novo cliente.

  pthread_mutex_unlock(&client_list_mutex);  // Libera o mutex após adicionar o cliente.
}

// Remove um cliente específico da lista de clientes conectados.
void remove_client(const char *client_id) {
  pthread_mutex_lock(&client_list_mutex);  // Bloqueia o mutex para garantir exclusão mútua.

  ClientArgs *current = client_list_head;  // Aponta para o início da lista.
  ClientArgs *previous = NULL;  // Ponteiro para o nó anterior, usado para reencadear a lista.

  while (current != NULL) {
    // Verifica se o cliente atual tem o ID correspondente.
    if (strcmp(current->client_id, client_id) == 0) {
      if (previous == NULL) {
        // Se o cliente está no início da lista, ajusta o ponteiro da cabeça.
        client_list_head = current->next; 
      } else {
        // Caso contrário, ajusta o ponteiro `next` do cliente anterior.
        previous->next = current->next;
      }

      // Fecha os descritores de pipe associados ao cliente.
      close(current->req_pipe_fd);
      close(current->resp_pipe_fd);
      close(current->notif_pipe_fd);

      // Liberta a memória alocada para o cliente.
      free(current);

      pthread_mutex_unlock(&client_list_mutex);  // Liberta o mutex.
      return;  // Encerra a função, pois o cliente foi removido.
    }
    previous = current;  // Atualiza o nó anterior para o nó atual.
    current = current->next;  // Avança para o próximo nó.
  }

  pthread_mutex_unlock(&client_list_mutex);  // Liberta o mutex se o cliente não foi encontrado.
}

// Extrai o ID do cliente de um caminho de pipe baseado em um prefixo.
const char *extract_client_id(const char *path, const char *prefix) {
  size_t prefix_len = strlen(prefix);  // Calcula o tamanho do prefixo.
  
  // Verifica se o caminho começa com o prefixo especificado.
  if (strncmp(path, prefix, prefix_len) == 0) {
    // Retorna o restante do caminho após o prefixo.
    return path + prefix_len;
  }

  return NULL;  // Retorna NULL se o prefixo não for encontrado no início do caminho.
}

int filter_job_files(const struct dirent *entry) {
  const char *dot = strrchr(entry->d_name, '.');
  if (dot != NULL && strcmp(dot, ".job") == 0) {
    return 1; // Keep this file (it has the .job extension)
  }
  return 0;
}

static int entry_files(const char *dir, struct dirent *entry, char *in_path, char *out_path) {
  const char *dot = strrchr(entry->d_name, '.');
  if (dot == NULL || dot == entry->d_name || strlen(dot) != 4 ||
      strcmp(dot, ".job")) {
    return 1;
  }

  if (strlen(entry->d_name) + strlen(dir) + 2 > MAX_JOB_FILE_NAME_SIZE) {
    fprintf(stderr, "%s/%s\n", dir, entry->d_name);
    return 1;
  }

  strcpy(in_path, dir);
  strcat(in_path, "/");
  strcat(in_path, entry->d_name);

  strcpy(out_path, in_path);
  strcpy(strrchr(out_path, '.'), ".out");

  return 0;
}

static int run_job(int in_fd, int out_fd, char *filename) {
  size_t file_backups = 0;
  while (1) {
    char keys[MAX_WRITE_SIZE][MAX_STRING_SIZE] = {0};
    char values[MAX_WRITE_SIZE][MAX_STRING_SIZE] = {0};
    unsigned int delay;
    size_t num_pairs;

    switch (get_next(in_fd)) {
    case CMD_WRITE:
      num_pairs = parse_write(in_fd, keys, values, MAX_WRITE_SIZE, MAX_STRING_SIZE);
      if (num_pairs == 0) {
        write_str(STDERR_FILENO, "Invalid command. See HELP for usage\n");
        continue;
      }

      if (kvs_write(num_pairs, keys, values)) {
        write_str(STDERR_FILENO, "Failed to write pair\n");
      }
      break;

    case CMD_READ:
      num_pairs = parse_read_delete(in_fd, keys, MAX_WRITE_SIZE, MAX_STRING_SIZE);

      if (num_pairs == 0) {
        write_str(STDERR_FILENO, "Invalid command. See HELP for usage\n");
        continue;
      }

      if (kvs_read(num_pairs, keys, out_fd)) {
        write_str(STDERR_FILENO, "Failed to read pair\n");
      }
      break;

    case CMD_DELETE:
      num_pairs =
          parse_read_delete(in_fd, keys, MAX_WRITE_SIZE, MAX_STRING_SIZE);

      if (num_pairs == 0) {
        write_str(STDERR_FILENO, "Invalid command. See HELP for usage\n");
        continue;
      }

      if (kvs_delete(num_pairs, keys, out_fd)) {
        write_str(STDERR_FILENO, "Failed to delete pair\n");
      }
      break;

    case CMD_SHOW:
      kvs_show(out_fd);
      break;

    case CMD_WAIT:
      if (parse_wait(in_fd, &delay, NULL) == -1) {
        write_str(STDERR_FILENO, "Invalid command. See HELP for usage\n");
        continue;
      }

      if (delay > 0) {
        printf("Waiting %d seconds\n", delay / 1000);
        kvs_wait(delay);
      }
      break;

    case CMD_BACKUP:
      pthread_mutex_lock(&n_current_backups_lock);
      if (active_backups >= max_backups) {
        wait(NULL);
      } else {
        active_backups++;
      }
      pthread_mutex_unlock(&n_current_backups_lock);
      int aux = kvs_backup(++file_backups, filename, jobs_directory);

      if (aux < 0) {
        write_str(STDERR_FILENO, "Failed to do backup\n");
      } else if (aux == 1) {
        return 1;
      }
      break;

    case CMD_INVALID:
      write_str(STDERR_FILENO, "Invalid command. See HELP for usage\n");
      break;

    case CMD_HELP:
      write_str(STDOUT_FILENO,
                "Available commands:\n"
                "  WRITE [(key,value)(key2,value2),...]\n"
                "  READ [key,key2,...]\n"
                "  DELETE [key,key2,...]\n"
                "  SHOW\n"
                "  WAIT <delay_ms>\n"
                "  BACKUP\n"
                "  HELP\n");

      break;

    case CMD_EMPTY:
      break;

    case EOC:
      printf("EOF\n");
      return 0;
    }
  }
}

static void *get_file(void *arguments) {
  struct SharedData *thread_data = (struct SharedData *)arguments;
  DIR *dir = thread_data->dir;
  char *dir_name = thread_data->dir_name;

  if (pthread_mutex_lock(&thread_data->directory_mutex) != 0) {
    fprintf(stderr, "Thread failed to lock directory_mutex\n");
    return NULL;
  }

  struct dirent *entry;
  char in_path[MAX_JOB_FILE_NAME_SIZE], out_path[MAX_JOB_FILE_NAME_SIZE];
  while ((entry = readdir(dir)) != NULL) {
    if (entry_files(dir_name, entry, in_path, out_path)) {
      continue;
    }

    if (pthread_mutex_unlock(&thread_data->directory_mutex) != 0) {
      fprintf(stderr, "Thread failed to unlock directory_mutex\n");
      return NULL;
    }

    int in_fd = open(in_path, O_RDONLY);
    if (in_fd == -1) {
      write_str(STDERR_FILENO, "Failed to open input file: ");
      write_str(STDERR_FILENO, in_path);
      write_str(STDERR_FILENO, "\n");
      pthread_exit(NULL);
    }

    int out_fd = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (out_fd == -1) {
      write_str(STDERR_FILENO, "Failed to open output file: ");
      write_str(STDERR_FILENO, out_path);
      write_str(STDERR_FILENO, "\n");
      pthread_exit(NULL);
    }

    int out = run_job(in_fd, out_fd, entry->d_name);

    close(in_fd);
    close(out_fd);

    if (out) {
      if (closedir(dir) == -1) {
        fprintf(stderr, "Failed to close directory\n");
        return 0;
      }

      exit(0);
    }

    if (pthread_mutex_lock(&thread_data->directory_mutex) != 0) {
      fprintf(stderr, "Thread failed to lock directory_mutex\n");
      return NULL;
    }
  }

  if (pthread_mutex_unlock(&thread_data->directory_mutex) != 0) {
    fprintf(stderr, "Thread failed to unlock directory_mutex\n");
    return NULL;
  }

  pthread_exit(NULL);
}

void* handle_client(void *args) {
  // Converte o argumento genérico para o tipo esperado (ClientArgs*).
  ClientArgs *client_args = (ClientArgs *)args;

  sigset_t set;
  // Inicializa um conjunto vazio de sinais.
  sigemptyset(&set);     
  // Adiciona SIGUSR1 ao conjunto de sinais.
  sigaddset(&set, SIGUSR1); 
  // Bloqueia o sinal SIGUSR1 para a thread atual.
  if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0) {
    perror("Erro ao bloquear SIGUSR1 na thread de atendimento");
    pthread_exit(NULL);  // Encerra a thread se o bloqueio falhar.
  }

  // Extrai os descritores de pipe e o ID do cliente dos argumentos.
  int req_pipe_fd = client_args->req_pipe_fd;
  int resp_pipe_fd = client_args->resp_pipe_fd;
  int notif_pipe_fd = client_args->notif_pipe_fd;
  const char *client_id = client_args->client_id;

  // Liberta a memória alocada para os argumentos da thread, já que eles não são mais necessários.
  free(client_args);

  while(1) {
    char pedido;   
    // Lê o tipo de pedido do cliente através do pipe de requisição.
    read_all(req_pipe_fd, &pedido, 1, NULL);
    if (pedido == '2') {
      char disconnect_message[3];
      // Prepara a mensagem de resposta para o cliente.
      strcpy(disconnect_message, "20");
      // Envia a resposta indicando sucesso na desconexão.
      write_all(resp_pipe_fd, &disconnect_message, 3);

      // Remove o cliente da lista de clientes conectados.
      remove_client(client_id);

      // Remove todas as subscrições que o cliente disconectado tinha.
      kvs_remove_client_subscriptions(client_id);

      // Fecha os pipes associados ao cliente.
      close(req_pipe_fd);
      close(resp_pipe_fd);
      close(notif_pipe_fd);
      break;
    }
    else if (pedido == '3') { 
      char buffer[41];
      // Lê a chave a ser subscrita enviada pelo cliente.
      read_all(req_pipe_fd, buffer, 41, NULL);

      char subscribeMessage[3];
      // Verifica se a chave já existe no KVS.
      if (key_exists(buffer) != 0) {
        // Chave não encontrada; envia mensagem de erro.
        strcpy(subscribeMessage, "30");
      } else {
        // Adiciona o cliente como subscritor da chave.
        kvs_add_client(buffer, client_id);
        // Envia mensagem de sucesso.
        strcpy(subscribeMessage, "31");
      }
      // Envia a mensagem de resposta ao cliente.
      write_all(resp_pipe_fd, subscribeMessage, strlen(subscribeMessage));
    }
    else if (pedido == '4') {
      char unsubscribeMessage[3];
      char buffer[41];
      // Lê a chave a ser removida da subscrição.
      read_all(req_pipe_fd, buffer, 41, NULL);

      // Remove o cliente como subscritor da chave no KVS.
      if (kvs_remove_client(buffer, client_id) != 0) {
        // Chave ou cliente não encontrado; envia mensagem de erro.
        strcpy(unsubscribeMessage, "41");
      }
      // Subscrição removida com sucesso; envia mensagem de sucesso.
      else strcpy(unsubscribeMessage, "40");
      // Envia a mensagem de resposta ao cliente.
      write_all(resp_pipe_fd, unsubscribeMessage, strlen(unsubscribeMessage));
    }
  }
  // Liberta um recurso do semáforo, indicando que o cliente foi desconectado.
  sem_post(&session_semaphore);
  // Encerra a thread.

  pthread_exit(NULL);
}

static void dispatch_threads(DIR *dir, char* string) {
  pthread_t *threads = malloc(max_threads * sizeof(pthread_t));

  if (threads == NULL) {
    fprintf(stderr, "Failed to allocate memory for threads\n");
    return;
  }

  struct SharedData thread_data = {dir, jobs_directory,
                                   PTHREAD_MUTEX_INITIALIZER};

  for (size_t i = 0; i < max_threads; i++) {
    if (pthread_create(&threads[i], NULL, get_file, (void *)&thread_data) != 0) {
      fprintf(stderr, "Failed to create thread %zu\n", i);
      pthread_mutex_destroy(&thread_data.directory_mutex);
      free(threads);
      return;
    }
  }

  char server_pipe_path[256] = "/tmp/";
  // Concatena o nome do FIFO de registo (armazenado na variável 'string') ao caminho base "/tmp/".
  strncat(server_pipe_path, string, strlen(string) * sizeof(char));

  // Remove o FIFO de registo, caso já exista, para evitar erros ao criá-lo novamente.
  unlink(server_pipe_path);

  // Cria o FIFO de registo no servidor com permissões 0666 (leitura e escrita para todos os usuários).
  if (mkfifo(server_pipe_path, 0666) == -1) {
    perror("Erro ao criar FIFO de registo");
    return;
  }

  int req_fd, resp_fd, notif_fd;

  while(1) {  // Loop principal do servidor para atender conexões de clientes.
    // Abre o FIFO de registo no modo somente leitura para esperar novas conexões de clientes.
    int server_fd = open(server_pipe_path, O_RDONLY);
    char req_pipe_path[MAX_STRING_SIZE];
    char resp_pipe_path[MAX_STRING_SIZE];
    char notif_pipe_path[MAX_STRING_SIZE];

    char op_code[1];
    read_all(server_fd, op_code, 1, NULL);
    read_all(server_fd, req_pipe_path, MAX_STRING_SIZE, NULL);
    read_all(server_fd, resp_pipe_path, MAX_STRING_SIZE, NULL);
    read_all(server_fd, notif_pipe_path, MAX_STRING_SIZE, NULL);

    // Extrai o ID único do cliente com base no caminho do FIFO de requisição.
    const char* client_id = extract_client_id(req_pipe_path, "/tmp/req");

    // Extrai o ID único do cliente com base no caminho do FIFO de requisição.
    sem_wait(&session_semaphore);


    // Thread que será criada para gerenciar o cliente.
    pthread_t client_thread;
    req_fd = open(req_pipe_path, O_RDONLY);
    resp_fd = open(resp_pipe_path, O_WRONLY);
    notif_fd = open(notif_pipe_path, O_WRONLY);

    if (req_fd == -1) {
      write_all(resp_fd, "11", strlen("11"));
    }
    if (resp_fd == -1) {
      write_all(resp_fd, "11", strlen("11"));
      close(req_fd);
    }
    if(notif_fd == -1) {
      write_all(resp_fd, "11", strlen("11"));
      close(req_fd);
      close(resp_fd);
      continue;
    }

    // Aloca memória para os argumentos do cliente que serão passados à thread.
    ClientArgs *client_args = malloc(sizeof(ClientArgs));
    if (!client_args) {
        perror("Failed to allocate memory for client arguments");
        sem_post(&session_semaphore);
        close(req_fd);
        close(resp_fd);
        close(notif_fd);
        continue;
    }

    client_args->req_pipe_fd = req_fd;
    client_args->resp_pipe_fd = resp_fd;
    client_args->notif_pipe_fd = notif_fd;
    strncpy(client_args->client_id, client_id, MAX_STRING_SIZE);

    // Adiciona o cliente à lista de clientes ativos.
    add_client(req_fd, resp_fd, notif_fd, client_id);

    // Envia uma mensagem de sucesso para o cliente confirmando a conexão.
    write_all(resp_fd, "10", strlen("10"));

    // Cria uma thread para gerenciar as operações do cliente.
    if (pthread_create(&client_thread, NULL, handle_client, (void *)client_args) != 0) {
      perror("Failed to create client thread");
      free(client_args);
      sem_post(&session_semaphore);
      close(req_fd);
      close(resp_fd);
      close(notif_fd);
      continue; 
    }

    // A thread foi criada com sucesso, então desconecta-a do processo principal.
    // Isso permite que o gerenciamento da thread seja feito de forma independente.
    pthread_detach(client_thread);
    close(server_fd);
  }

  for (unsigned int i = 0; i < max_threads; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      fprintf(stderr, "Failed to join thread %u\n", i);
      pthread_mutex_destroy(&thread_data.directory_mutex);
      free(threads);
      return;
    }
  }

  if (pthread_mutex_destroy(&thread_data.directory_mutex) != 0) {
    fprintf(stderr, "Failed to destroy directory_mutex\n");
  }

  free(threads);
}

int main(int argc, char **argv) {
  if (argc < 5) {
    write_str(STDERR_FILENO, "Usage: ");
    write_str(STDERR_FILENO, argv[0]);
    write_str(STDERR_FILENO, " <jobs_dir>");
    write_str(STDERR_FILENO, " <max_threads>");
    write_str(STDERR_FILENO, " <max_backups>");
    write_str(STDERR_FILENO, " <name_FIFO_registo> \n");
    return 1;
  }

  // Define uma estrutura sigaction para configurar o tratamento do sinal SIGUSR1.
  struct sigaction sa;

  // Especifica a função que será chamada quando o sinal SIGUSR1 for recebido.
  sa.sa_handler = handle_sigusr1;

  // Inicializa a máscara de sinais com um conjunto vazio, ou seja, não bloqueia outros sinais enquanto o SIGUSR1 é tratado.
  sigemptyset(&sa.sa_mask);

  // Define flags adicionais para o comportamento do sinal. 
  // Neste caso, 0 significa que não há opções adicionais configuradas.
  sa.sa_flags = 0;

  // Configura o tratamento do sinal SIGUSR1 com as definições acima.
  // Caso ocorra um erro, exibe uma mensagem de erro e encerra o programa.
  if (sigaction(SIGUSR1, &sa, NULL) == -1) {
      perror("Erro ao configurar SIGUSR1");
      exit(EXIT_FAILURE);
  }

  // Inicializa o semáforo session_semaphore com um valor inicial igual a MAX_SESSIONS.
  // O segundo argumento (0) indica que o semáforo é compartilhado entre threads do mesmo processo.
  sem_init(&session_semaphore, 0, MAX_SESSIONS);

  jobs_directory = argv[1];

  char *endptr;
  max_backups = strtoul(argv[3], &endptr, 10);

  if (*endptr != '\0') {
    fprintf(stderr, "Invalid max_proc value\n");
    return 1;
  }

  max_threads = strtoul(argv[2], &endptr, 10);

  if (*endptr != '\0') {
    fprintf(stderr, "Invalid max_threads value\n");
    return 1;
  }

  if (max_backups <= 0) {
    write_str(STDERR_FILENO, "Invalid number of backups\n");
    return 0;
  }

  if (max_threads <= 0) {
    write_str(STDERR_FILENO, "Invalid number of threads\n");
    return 0;
  }

  if (kvs_init()) {
    write_str(STDERR_FILENO, "Failed to initialize KVS\n");
    return 1;
  }

  DIR *dir = opendir(argv[1]);
  if (dir == NULL) {
    fprintf(stderr, "Failed to open directory: %s\n", argv[1]);
    return 0;
  }

  dispatch_threads(dir, argv[4]);

  if (closedir(dir) == -1) {
    fprintf(stderr, "Failed to close directory\n");
    return 0;
  }

  while (active_backups > 0) {
    wait(NULL);
    active_backups--;
  }

  kvs_terminate();

  // Destroi o semáforo session_semaphore, libertando os recursos associados a ele.
  sem_destroy(&session_semaphore);

  return 0;
}
