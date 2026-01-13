#include "operations.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "constants.h"
#include "io.h"
#include "kvs.h"

static struct HashTable *kvs_table = NULL;

/// Calculates a timespec from a delay in milliseconds.
/// @param delay_ms Delay in milliseconds.
/// @return Timespec with the given delay.
static struct timespec delay_to_timespec(unsigned int delay_ms) {
  return (struct timespec){delay_ms / 1000, (delay_ms % 1000) * 1000000};
}

int key_exists(const char* key) {
  if (kvs_table == NULL) {
    fprintf(stderr, "KVS state must be initialized\n");
    return 1;
  }

  int index = hash(key); // Calcula o índice da tabela hash baseado na chave
  KeyNode *node = kvs_table->table[index];

  // Percorre a lista no índice calculado
  while (node != NULL) {
    if (strcmp(node->key, key) == 0) {
      return 0; // Retorna 0 se a chave foi encontrada
    }
    node = node->next;
  }
  return 1; // Retorna 1 se a chave não foi encontrada
}

int kvs_add_client(const char *key, const char *client_id) {
  if (kvs_table == NULL) {
    fprintf(stderr, "KVS state must be initialized\n");
    return 1;
  }

  pthread_rwlock_wrlock(&kvs_table->tablelock); // Bloqueia a tabela para operações de escrita

  int index = hash(key); // Calcula o índice da tabela hash baseado na chave
  KeyNode *node = kvs_table->table[index];

  // Percorre a lista para encontrar a chave correspondente
  while (node != NULL) {
    if (strcmp(node->key, key) == 0) {
      // Se a chave for encontrada, adiciona o ID do cliente à lista de subscrições
      add_client_id(node, client_id);
      pthread_rwlock_unlock(&kvs_table->tablelock);
      return 0; // Sucesso
    }
    node = node->next;
  }

  // Caso a chave não seja encontrada, exibe uma mensagem de erro
  fprintf(stderr, "Key '%s' not found in KVS\n", key);
  pthread_rwlock_unlock(&kvs_table->tablelock);
  return 1; // Falha
}

int kvs_remove_client(const char *key, const char *client_id) {
  if (kvs_table == NULL) {
    fprintf(stderr, "KVS state must be initialized\n");
    return 1;
  }
  pthread_rwlock_wrlock(&kvs_table->tablelock); // Bloqueia a tabela para operações de escrita

  int index = hash(key); // Calcula o índice da tabela hash baseado na chave
  KeyNode *node = kvs_table->table[index];

  // Percorre a lista para encontrar a chave correspondente
  while (node != NULL) {
    if (strcmp(node->key, key) == 0) {
      // Se a chave for encontrada, procura o client ID na lista associada
      for (int i = 0; i < node->num_ids; i++) {
        if (strcmp(node->client_ids[i], client_id) == 0) {
          free(node->client_ids[i]); // Liberta a memória associada ao ID do cliente
          // Realoca os IDs restantes para preencher o espaço vazio
          for (int j = i; j < node->num_ids - 1; j++) {
            node->client_ids[j] = node->client_ids[j + 1];
          }
          node->num_ids--; // Decrementa o número de IDs associados
          pthread_rwlock_unlock(&kvs_table->tablelock);
          return 0; // Sucesso
        }
      }
      // Caso o client ID não seja encontrado
      pthread_rwlock_unlock(&kvs_table->tablelock);
      return 1; // Falha
    }
    node = node->next;
  }
  // Caso a chave não seja encontrada
  fprintf(stderr, "Key '%s' not found in KVS\n", key);
  pthread_rwlock_unlock(&kvs_table->tablelock);
  return 1; // Falha
}

void kvs_remove_all_clients() {
  if (kvs_table == NULL) {
    fprintf(stderr, "KVS state must be initialized\n");
    return;
  }

  pthread_rwlock_wrlock(&kvs_table->tablelock); // Bloqueia a tabela para operações de escrita

  // Percorre todos os índices da tabela hash
  for (int i = 0; i < TABLE_SIZE; i++) {
    KeyNode *node = kvs_table->table[i];
    while (node != NULL) {
        // Remove todos os IDs de clientes associados ao nó atual
        for (int j = 0; j < node->num_ids; j++) {
          free(node->client_ids[j]); // Liberta a memória associada ao client ID
          node->client_ids[j] = NULL;
        }
        node->num_ids = 0; // Reseta o número de IDs associados

        node = node->next; // Avança para o próximo nó na lista ligada
    }
  }

  pthread_rwlock_unlock(&kvs_table->tablelock); // Liberta o bloqueio da tabela
}

void kvs_remove_client_subscriptions(const char *client_id) {
  if (kvs_table == NULL) {
    fprintf(stderr, "KVS state must be initialized\n");
    return;
  }

  pthread_rwlock_wrlock(&kvs_table->tablelock); // Bloquear a tabela para escrita

  // Percorrer todos os índices da tabela hash
  for (int i = 0; i < TABLE_SIZE; i++) {
    KeyNode *node = kvs_table->table[i];
    while (node != NULL) {
      // Verificar se o client_id está associado à chave atual
      for (int j = 0; j < node->num_ids; j++) {
        if (strcmp(node->client_ids[j], client_id) == 0) {
          // Remover o client_id da lista
          free(node->client_ids[j]);
          for (int k = j; k < node->num_ids - 1; k++) {
            node->client_ids[k] = node->client_ids[k + 1];
          }
          node->num_ids--; // Atualizar o número de IDs associados

          // Parar a verificação desta chave, pois removemos o ID
          break;
        }
      }
      node = node->next; // Avançar para o próximo nó
    }
  }

  pthread_rwlock_unlock(&kvs_table->tablelock); // Liberar o bloqueio da tabela
}

int kvs_init() {
  if (kvs_table != NULL) {
    fprintf(stderr, "KVS state has already been initialized\n");
    return 1;
  }

  kvs_table = create_hash_table();
  return kvs_table == NULL;
}

int kvs_terminate() {
  if (kvs_table == NULL) {
    fprintf(stderr, "KVS state must be initialized\n");
    return 1;
  }

  free_table(kvs_table);
  kvs_table = NULL;
  return 0;
}

int kvs_write(size_t num_pairs, char keys[][MAX_STRING_SIZE],
              char values[][MAX_STRING_SIZE]) {
  if (kvs_table == NULL) {
    fprintf(stderr, "KVS state must be initialized\n");
    return 1;
  }

  pthread_rwlock_wrlock(&kvs_table->tablelock);

  for (size_t i = 0; i < num_pairs; i++) {
    if (write_pair(kvs_table, keys[i], values[i]) != 0) {
      fprintf(stderr, "Failed to write key pair (%s,%s)\n", keys[i], values[i]);
    }
    
  }

  pthread_rwlock_unlock(&kvs_table->tablelock);
  return 0;
}

int kvs_read(size_t num_pairs, char keys[][MAX_STRING_SIZE], int fd) {
  if (kvs_table == NULL) {
    fprintf(stderr, "KVS state must be initialized\n");
    return 1;
  }

  pthread_rwlock_rdlock(&kvs_table->tablelock);

  write_str(fd, "[");
  for (size_t i = 0; i < num_pairs; i++) {
    char *result = read_pair(kvs_table, keys[i]);
    char aux[MAX_STRING_SIZE];
    if (result == NULL) {
      snprintf(aux, MAX_STRING_SIZE, "(%s,KVSERROR)", keys[i]);
    } else {
      snprintf(aux, MAX_STRING_SIZE, "(%s,%s)", keys[i], result);
    }
    write_str(fd, aux);
    free(result);
  }
  write_str(fd, "]\n");

  pthread_rwlock_unlock(&kvs_table->tablelock);
  return 0;
}

int kvs_delete(size_t num_pairs, char keys[][MAX_STRING_SIZE], int fd) {
  if (kvs_table == NULL) {
    fprintf(stderr, "KVS state must be initialized\n");
    return 1;
  }

  pthread_rwlock_wrlock(&kvs_table->tablelock);

  int aux = 0;
  for (size_t i = 0; i < num_pairs; i++) {
    if (delete_pair(kvs_table, keys[i]) != 0) {
      if (!aux) {
        write_str(fd, "[");
        aux = 1;
      }
      char str[MAX_STRING_SIZE];
      snprintf(str, MAX_STRING_SIZE, "(%s,KVSMISSING)", keys[i]);
      write_str(fd, str);
    }
  }
  if (aux) {
    write_str(fd, "]\n");
  }

  pthread_rwlock_unlock(&kvs_table->tablelock);
  return 0;
}

void kvs_show(int fd) {
  if (kvs_table == NULL) {
    fprintf(stderr, "KVS state must be initialized\n");
    return;
  }

  pthread_rwlock_rdlock(&kvs_table->tablelock);
  char aux[MAX_STRING_SIZE];

  for (int i = 0; i < TABLE_SIZE; i++) {
    KeyNode *keyNode = kvs_table->table[i]; // Get the next list head
    while (keyNode != NULL) {
      snprintf(aux, MAX_STRING_SIZE, "(%s, %s)\n", keyNode->key,
               keyNode->value);
      write_str(fd, aux);
      keyNode = keyNode->next; // Move to the next node of the list
    }
  }

  pthread_rwlock_unlock(&kvs_table->tablelock);
}

int kvs_backup(size_t num_backup, char *job_filename, char *directory) {
  pid_t pid;
  char bck_name[50];
  snprintf(bck_name, sizeof(bck_name), "%s/%s-%ld.bck", directory,
           strtok(job_filename, "."), num_backup);

  pthread_rwlock_rdlock(&kvs_table->tablelock);
  pid = fork();
  pthread_rwlock_unlock(&kvs_table->tablelock);
  if (pid == 0) {
    // functions used here have to be async signal safe, since this
    // fork happens in a multi thread context (see man fork)
    int fd = open(bck_name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    for (int i = 0; i < TABLE_SIZE; i++) {
      KeyNode *keyNode = kvs_table->table[i]; // Get the next list head
      while (keyNode != NULL) {
        char aux[MAX_STRING_SIZE];
        aux[0] = '(';
        size_t num_bytes_copied = 1; // the "("
        // the - 1 are all to leave space for the '/0'
        num_bytes_copied += strn_memcpy(aux + num_bytes_copied, keyNode->key,
                                        MAX_STRING_SIZE - num_bytes_copied - 1);
        num_bytes_copied += strn_memcpy(aux + num_bytes_copied, ", ",
                                        MAX_STRING_SIZE - num_bytes_copied - 1);
        num_bytes_copied += strn_memcpy(aux + num_bytes_copied, keyNode->value,
                                        MAX_STRING_SIZE - num_bytes_copied - 1);
        num_bytes_copied += strn_memcpy(aux + num_bytes_copied, ")\n",
                                        MAX_STRING_SIZE - num_bytes_copied - 1);
        aux[num_bytes_copied] = '\0';
        write_str(fd, aux);
        keyNode = keyNode->next; // Move to the next node of the list
      }
    }
    exit(1);
  } else if (pid < 0) {
    return -1;
  }
  return 0;
}

void kvs_wait(unsigned int delay_ms) {
  struct timespec delay = delay_to_timespec(delay_ms);
  nanosleep(&delay, NULL);
}
