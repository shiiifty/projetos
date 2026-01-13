#include "kvs.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h> 
#include <string.h> 
#include <fcntl.h>
#include <unistd.h>

#include "string.h"
#include "../common/io.h"

void add_client_id(KeyNode *node, const char *id) {
  // Realoca memória para armazenar mais um ID de cliente no array de client_ids.
  // O tamanho do array é incrementado em 1.
  node->client_ids = realloc(node->client_ids, ((size_t)node->num_ids + 1) * sizeof(char *));
  if (node->client_ids == NULL) {
    // Caso a realocação falhe, exibe uma mensagem de erro e encerra o programa.
    perror("Erro ao realocar memória para client_ids");
    exit(EXIT_FAILURE);
  }

  // Duplica a string `id` e a adiciona ao array de client_ids na nova posição disponível.
  node->client_ids[node->num_ids] = strdup(id);
  if (node->client_ids[node->num_ids] == NULL) {
    // Caso a alocação falhe, exibe uma mensagem de erro e encerra o programa.
    perror("Erro ao alocar memória para ID");
    exit(EXIT_FAILURE);
  }

  // Incrementa o contador de IDs armazenados no nó.
  node->num_ids++;
}


// Hash function based on key initial.
// @param key Lowercase alphabetical string.
// @return hash.
// NOTE: This is not an ideal hash function, but is useful for test purposes of
// the project
int hash(const char *key) {
  int firstLetter = tolower(key[0]);
  if (firstLetter >= 'a' && firstLetter <= 'z') {
    return firstLetter - 'a';
  } else if (firstLetter >= '0' && firstLetter <= '9') {
    return firstLetter - '0';
  }
  return -1; // Invalid index for non-alphabetic or number strings
}

struct HashTable *create_hash_table() {
  HashTable *ht = malloc(sizeof(HashTable));
  if (!ht)
    return NULL;
  for (int i = 0; i < TABLE_SIZE; i++) {
    ht->table[i] = NULL;
  }
  pthread_rwlock_init(&ht->tablelock, NULL);
  return ht;
}

int write_pair(HashTable *ht, const char *key, const char *value) {
  int index = hash(key);

  // Search for the key node
  KeyNode *keyNode = ht->table[index];
  KeyNode *previousNode;

  while (keyNode != NULL) {
    if (strcmp(keyNode->key, key) == 0) {
      // overwrite value
      free(keyNode->value);
      keyNode->value = strdup(value);
      int client_atual = 0;
      while (client_atual < keyNode->num_ids) {
        const char* client_id = keyNode->client_ids[client_atual];
        char notif_pipe_path[256] = "/tmp/notif";
        strncat(notif_pipe_path, client_id, strlen(client_id) * sizeof(char));
        int notif_pipe_fd = open(notif_pipe_path, O_WRONLY);
        char formatted_message[256];
        snprintf(formatted_message, sizeof(formatted_message), "(%s,%s)", keyNode->key, keyNode->value);
        write_all(notif_pipe_fd, &formatted_message, strlen(formatted_message));
        close(notif_pipe_fd);
      }
      return 0;
    }
    previousNode = keyNode;
    keyNode = previousNode->next; // Move to the next node
  }
  // Key not found, create a new key node
  keyNode = malloc(sizeof(KeyNode));
  keyNode->key = strdup(key);       // Allocate memory for the key
  keyNode->value = strdup(value);   // Allocate memory for the value
  keyNode->next = ht->table[index]; // Link to existing nodes
  ht->table[index] = keyNode; // Place new key node at the start of the list
  return 0;
}

char *read_pair(HashTable *ht, const char *key) {
  int index = hash(key);

  KeyNode *keyNode = ht->table[index];
  KeyNode *previousNode;
  char *value;

  while (keyNode != NULL) {
    if (strcmp(keyNode->key, key) == 0) {
      value = strdup(keyNode->value);
      return value; // Return the value if found
    }
    previousNode = keyNode;
    keyNode = previousNode->next; // Move to the next node
  }

  return NULL; // Key not found
}

int delete_pair(HashTable *ht, const char *key) {
  int index = hash(key);

  // Search for the key node
  KeyNode *keyNode = ht->table[index];
  KeyNode *prevNode = NULL;

  while (keyNode != NULL) {
    if (strcmp(keyNode->key, key) == 0) {
      int client_atual = 0;
      while (client_atual < keyNode->num_ids) {
        const char* client_id = keyNode->client_ids[client_atual];
        char notif_pipe_path[256] = "/tmp/notif";
        strncat(notif_pipe_path, client_id, strlen(client_id) * sizeof(char));
        int notif_pipe_fd = open(notif_pipe_path, O_WRONLY);
        char formatted_message[256];
        snprintf(formatted_message, sizeof(formatted_message), "(%s,DELETED)", keyNode->key);
        write_all(notif_pipe_fd, &formatted_message, strlen(formatted_message));
        close(notif_pipe_fd);
      }
      // Key found; delete this node
      if (prevNode == NULL) {
        // Node to delete is the first node in the list
        ht->table[index] =
        keyNode->next; // Update the table to point to the next node
      } else {
        // Node to delete is not the first; bypass it
        prevNode->next =
        keyNode->next; // Link the previous node to the next node
      }
      // Free the memory allocated for the key and value
      free(keyNode->key);
      free(keyNode->value);
      free(keyNode); // Free the key node itself
      return 0;      // Exit the function
    }
    prevNode = keyNode;      // Move prevNode to current node
    keyNode = keyNode->next; // Move to the next node
  }

  return 1;
}

void free_table(HashTable *ht) {
  for (int i = 0; i < TABLE_SIZE; i++) {
    KeyNode *keyNode = ht->table[i];
    while (keyNode != NULL) {
      KeyNode *temp = keyNode;
      keyNode = keyNode->next;
      free(temp->key);
      free(temp->value);
      free(temp);
    }
  }
  pthread_rwlock_destroy(&ht->tablelock);
  free(ht);
}
