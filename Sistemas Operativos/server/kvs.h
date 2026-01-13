#ifndef KEY_VALUE_STORE_H
#define KEY_VALUE_STORE_H
#define TABLE_SIZE 26

#include <pthread.h>
#include <stddef.h>

/// Estrutura que representa um nó na tabela hash.
/// Cada nó armazena:
/// - Uma chave (`key`) e seu valor (`value`).
/// - Um array dinâmico de IDs de clientes (`client_ids`) associados à chave.
/// - O número de IDs de clientes (`num_ids`).
/// - Um ponteiro para o próximo nó na lista encadeada (`next`).
typedef struct KeyNode {
  char *key;          ///< Chave armazenada no nó.
  char *value;        ///< Valor correspondente à chave.
  char **client_ids;  ///< Lista de IDs de clientes associados à chave.
  int num_ids;        ///< Número de IDs associados ao nó.
  struct KeyNode *next; ///< Próximo nó na lista encadeada.
} KeyNode;

typedef struct HashTable {
  KeyNode *table[TABLE_SIZE];
  pthread_rwlock_t tablelock;
} HashTable;

/// Adiciona um ID de cliente ao nó correspondente.
/// Realoca memória para armazenar o novo ID na lista de IDs do nó.
/// @param node O nó ao qual o ID será adicionado.
/// @param id O ID do cliente a ser adicionado.
void add_client_id(KeyNode *node, const char *id);

/// Creates a new KVS hash table.
/// @return Newly created hash table, NULL on failure
struct HashTable *create_hash_table();

int hash(const char *key);

// Writes a key value pair in the hash table.
// @param ht The hash table.
// @param key The key.
// @param value The value.
// @return 0 if successful.
int write_pair(HashTable *ht, const char *key, const char *value);

// Reads the value of a given key.
// @param ht The hash table.
// @param key The key.
// return the value if found, NULL otherwise.
char *read_pair(HashTable *ht, const char *key);

/// Deletes a pair from the table.
/// @param ht Hash table to read from.
/// @param key Key of the pair to be deleted.
/// @return 0 if the node was deleted successfully, 1 otherwise.
int delete_pair(HashTable *ht, const char *key);

/// Frees the hashtable.
/// @param ht Hash table to be deleted.
void free_table(HashTable *ht);

#endif // KVS_H
