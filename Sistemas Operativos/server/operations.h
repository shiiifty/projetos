#ifndef KVS_OPERATIONS_H
#define KVS_OPERATIONS_H

#include <stddef.h>

#include "constants.h"

/// Checks if a key exists in the KVS.
/// @param key Key to check.
/// @return 0 if the key exists, 1 otherwise.
int key_exists(const char* key);

/// Adds a client ID to a given key in the KVS.
/// @param key Key to which the client ID should be added.
/// @param client_id The client ID to add.
/// @return 0 if the client ID was added successfully, 1 otherwise.
int kvs_add_client(const char *key, const char *client_id);

/// Removes a client ID from a given key in the KVS.
/// @param key Key from which the client ID should be removed.
/// @param client_id The client ID to remove.
/// @return 0 if the client ID was removed successfully, 1 otherwise.
int kvs_remove_client(const char *key, const char *client_id);

/// Removes all client IDs from all keys in the KVS.
void kvs_remove_all_clients();

/// Removes all subscriptions associated with a specific client ID.
/// This function iterates through the KVS table and removes the given client ID
/// from the list of subscriptions for every key where it is found.
/// @param client_id The ID of the client whose subscriptions are to be removed.
void kvs_remove_client_subscriptions(const char *client_id);

/// Initializes the KVS state.
/// @return 0 if the KVS state was initialized successfully, 1 otherwise.
int kvs_init();

/// Destroys the KVS state.
/// @return 0 if the KVS state was terminated successfully, 1 otherwise.
int kvs_terminate();

/// Writes a key value pair to the KVS. If key already exists it is updated.
/// @param num_pairs Number of pairs being written.
/// @param keys Array of keys' strings.
/// @param values Array of values' strings.
/// @return 0 if the pairs were written successfully, 1 otherwise.
int kvs_write(size_t num_pairs, char keys[][MAX_STRING_SIZE],
              char values[][MAX_STRING_SIZE]);

/// Reads values from the KVS.
/// @param num_pairs Number of pairs to read.
/// @param keys Array of keys' strings.
/// @param fd File descriptor to write the (successful) output.
/// @return 0 if the key reading, 1 otherwise.
int kvs_read(size_t num_pairs, char keys[][MAX_STRING_SIZE], int fd);

/// Deletes key value pairs from the KVS.
/// @param num_pairs Number of pairs to read.
/// @param keys Array of keys' strings.
/// @return 0 if the pairs were deleted successfully, 1 otherwise.
int kvs_delete(size_t num_pairs, char keys[][MAX_STRING_SIZE], int fd);

/// Writes the state of the KVS.
/// @param fd File descriptor to write the output.
void kvs_show(int fd);

/// Creates a backup of the KVS state and stores it in the correspondent
/// backup file
/// @return 0 if the backup was successful, 1 otherwise.
int kvs_backup(size_t num_backup, char *job_filename, char *directory);

/// Waits for the last backup to be called.
void kvs_wait_backup();

/// Waits for a given amount of time.
/// @param delay_us Delay in milliseconds.
void kvs_wait(unsigned int delay_ms);

// Setter for max_backups
// @param _max_backups
void set_max_backups(int _max_backups);

// Setter for n_current_backups
// @param _n_current_backups
void set_n_current_backups(int _n_current_backups);

// Getter for n_current_backups
// @return n_current_backups
int get_n_current_backups();

#endif // KVS_OPERATIONS_H
