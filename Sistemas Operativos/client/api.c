#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include "api.h"
#include "../common/constants.h"
#include "../common/protocol.h"
#include "../common/io.h"

static int request_pipe_fd = -1;
static int resposta_pipe_fd = -1;
static int notifications_pipe_fd = -1;
static int server_pipe_fd = -1;

// Funções para expor os FDs
int get_request_pipe_fd() {
    return request_pipe_fd;
}

int get_response_pipe_fd() {
    return resposta_pipe_fd;
}

int get_notifications_pipe_fd() {
    return notifications_pipe_fd;
}

int kvs_connect(char const* req_pipe_path, char const* resp_pipe_path,
                char const* server_pipe_path, char const* notif_pipe_path) {

    // Remove os pipes existentes, ignorando erros caso eles não existam
    if (unlink(req_pipe_path) == -1 && errno != ENOENT) {
        perror("Erro ao remover FIFO de requisição");
        return 1;
    }

    if (unlink(resp_pipe_path) == -1 && errno != ENOENT) {
        perror("Erro ao remover FIFO de resposta");
        return 1;
    }

    if (unlink(notif_pipe_path) == -1 && errno != ENOENT) {
        perror("Erro ao remover FIFO de notificações");
        return 1;
    }

    // Criação dos pipes
    if (mkfifo(req_pipe_path, 0777) == -1) {
        perror("Erro ao criar FIFO de requisição");
        return 1;
    }

    if (mkfifo(resp_pipe_path, 0666) == -1) {
        perror("Erro ao criar FIFO de resposta");
        unlink(req_pipe_path);
        return 1;
    }

    if (mkfifo(notif_pipe_path, 0666) == -1) {
        perror("Erro ao criar FIFO de notificações");
        unlink(req_pipe_path);
        unlink(resp_pipe_path);
        return 1;
    }

    // Formata o caminho para o pipe do servidor
    char server_pipe_path_aux[256] = "/tmp/";
    strncat(server_pipe_path_aux, server_pipe_path, strlen(server_pipe_path) * sizeof(char));

    // Abre o pipe do servidor
    server_pipe_fd = open(server_pipe_path_aux, O_WRONLY);
    if (server_pipe_fd < 0) {
        perror("Erro ao abrir ficheiro");
        goto cleanup; // Salta para a limpeza caso ocorra um erro
        return 1;
    }

    memset((char *)(req_pipe_path + strlen(req_pipe_path)), '\0', MAX_STRING_SIZE - strlen(req_pipe_path));
    memset((char *)(resp_pipe_path + strlen(resp_pipe_path)), '\0', MAX_STRING_SIZE - strlen(resp_pipe_path));
    memset((char *)(notif_pipe_path + strlen(notif_pipe_path)), '\0', MAX_STRING_SIZE - strlen(notif_pipe_path));

    if (write_all(server_pipe_fd, "1", 1) == -1) {
        perror("Erro ao enviar mensagem de conexão");
        close(server_pipe_fd);
        goto cleanup;
    }

    // Envia informações ao servidor para estabelecer a conexão
    if (write_all(server_pipe_fd, req_pipe_path, MAX_STRING_SIZE) == -1) {
        perror("Erro ao enviar mensagem de conexão");
        close(server_pipe_fd);
        goto cleanup;
    }
    
    if (write_all(server_pipe_fd, resp_pipe_path, MAX_STRING_SIZE) == -1) {
        perror("Erro ao enviar mensagem de conexão");
        close(server_pipe_fd);
        goto cleanup;
    }
    
    if (write_all(server_pipe_fd, notif_pipe_path, MAX_STRING_SIZE) == -1) {
        perror("Erro ao enviar mensagem de conexão");
        close(server_pipe_fd);
        goto cleanup;
    }

    // Abre os pipes criados
    request_pipe_fd = open(req_pipe_path, O_WRONLY);

    if (request_pipe_fd < 0) {
        perror("Erro ao abrir o pipe de requisição");
        return 1;
    }

    resposta_pipe_fd = open(resp_pipe_path, O_RDONLY);
    if (resposta_pipe_fd < 0) {
        perror("Erro ao abrir o pipe de resposta");
        close(request_pipe_fd);
        return 1;
    }

    notifications_pipe_fd = open(notif_pipe_path, O_RDONLY);
    if (notifications_pipe_fd < 0) {
        perror("Erro ao abrir o pipe de notificações");
        close(request_pipe_fd);
        close(resposta_pipe_fd);
        return 1;
    }

    // Lê a resposta do servidor
    char resposta[2];
    read_all(resposta_pipe_fd, resposta, 2, NULL);
    printf("Server returned %c for operation: connect\n", resposta[1]);
    
    if (resposta[1] == '1') goto cleanup;

    return 0;

cleanup:
    // Fecha e remove os pipes criados em caso de erro
    if (request_pipe_fd != -1) close(request_pipe_fd);
    if (resposta_pipe_fd != -1) close(resposta_pipe_fd);
    if (server_pipe_fd != -1) close(server_pipe_fd);

    unlink(req_pipe_path);
    unlink(resp_pipe_path);
    unlink(notif_pipe_path);

    return 1;
}
 
int kvs_disconnect(char const* req_pipe_path, char const* resp_pipe_path, char const* notif_pipe_path) {
    if (request_pipe_fd == -1 || resposta_pipe_fd == -1 || notifications_pipe_fd == -1) {
        fprintf(stderr, "Erro: Nenhuma sessão ativa para desconectar\n");
        return 1;
    }

    // Envia mensagem de desconexão
    char disconnect_message = '2';
    write_all(request_pipe_fd, &disconnect_message, 1);
    char resposta[2];
    read_all(resposta_pipe_fd, resposta, 2, NULL);

    printf("Server returned %c for operation: disconnect\n", resposta[1]);

    // Fecha e remove os pipes se a desconexão foi bem-sucedida
    if (resposta[1] == '0') {
        close(request_pipe_fd);
        close(resposta_pipe_fd);
        close(notifications_pipe_fd);
        close(server_pipe_fd);

        unlink(req_pipe_path);
        unlink(resp_pipe_path);
        unlink(notif_pipe_path);

        request_pipe_fd = -1;
        resposta_pipe_fd = -1;
        notifications_pipe_fd = -1;
        return 0;
    }
    return 1;
}

int kvs_subscribe(char const* key) {
    if (request_pipe_fd == -1) {
    fprintf(stderr, "Erro: Cliente não ativo\n");
    return 1;
    }

    char subscribe_message[42];
    subscribe_message[0] = '3';

    strncpy(subscribe_message + 1, key, strlen(key));

    size_t key_length = strlen(key);
    memset(subscribe_message + 1 + key_length, '\0', sizeof(subscribe_message) - 1 - key_length);

    if (write_all(request_pipe_fd, subscribe_message, sizeof(subscribe_message)) == -1) {
    perror("Erro ao enviar mensagem de subscrição");
    return 1;
    }

    char resposta[2];
    if (read_all(resposta_pipe_fd, resposta, 2, NULL) <= 0) {
    perror("Erro ao ler resposta do servidor");
    return 1;
    }

    if (resposta[0] != '3') {
    fprintf(stderr, "Resposta inválida do servidor para subscribe\n");
    return 1;
    }

    printf("Server returned %c for operation: subscribe\n", resposta[1]);

    if (resposta[1] == '0') return 1;

    return 0;
}

int kvs_unsubscribe(char const* key) {
    if (request_pipe_fd == -1) {
    fprintf(stderr, "Erro: Nenhuma sessão ativa\n");
    return 1;
    }

    char unsubscribe_message[42];
    unsubscribe_message[0] = '4';

    strncpy(unsubscribe_message + 1, key, strlen(key));

    size_t key_length = strlen(key);
    memset(unsubscribe_message + 1 + key_length, '\0', sizeof(unsubscribe_message) - 1 - key_length);

    if (write_all(request_pipe_fd, unsubscribe_message, sizeof(unsubscribe_message)) == -1) {
    perror("Erro ao enviar mensagem de cancelamento de subscrição");
    return 1;
    }

    char resposta[2];
    if (read_all(resposta_pipe_fd, resposta, 2, NULL) <= 0) {
    perror("Erro ao ler resposta do servidor");
    return 1;
    }

    if (resposta[0] != '4') {
    fprintf(stderr, "Resposta inválida do servidor para unsubscribe\n");
    return 1;
    }

    printf("Server returned %c for operation: unsubscribe\n", resposta[1]);

    return 0;
}