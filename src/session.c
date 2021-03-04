/**
 * @file session.c
 * @author V.K.
 * @brief This file contains all the definitions for storing server sessions.
 * @date 2021-02-26
 *
 * @copyright Copyright Balt-System Ltd. <info@bsystem.ru>
 * All rights reserved.
 *
 */
#include "session.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"

/**
 * @brief Internal struct witch represents hash table item
 */
struct key_value_pair {
    struct session_info value;
    int16_t key;
};

/**
 * @brief Internal hash table array
 */
static struct key_value_pair **hash_array;

/**
 * @brief Placeholder for removed items
 */
static struct key_value_pair *dummy_item;

/**
 * @brief Hash table internal array size.
 */
static int16_t hash_array_size;

/**
 * @brief Generates a hash code for specified key.
 * Values are used to index a hash table.
 * @param key Specified key
 * @return int16_t Hash
 */
int16_t hash_code(int16_t key)
{
    return key % hash_array_size;
}

/**
 * @brief Search key_value_pair in hash table by key
 * @param key Specified key
 * @return struct key_value_pair* Pointer to hash table item
 */
struct key_value_pair *find_item(int16_t key)
{
    /* Get the hash */
    int hash_index = hash_code(key);

    /* Move in array until an empty */
    while (hash_array[hash_index] != NULL) {
        if (hash_array[hash_index]->key == key)
            return hash_array[hash_index];

        /* Go to next cell */
        ++hash_index;

        /* Wrap around the table */
        hash_index %= hash_array_size;
    }

    return NULL;
}

/**
 * @brief Put new key_value_pair item to hash table
 * @param key Item key.
 * @param data Item Value.
 */
static void insert_item(int key, struct session_info data)
{
    struct key_value_pair *item = malloc(sizeof(struct key_value_pair));
    item->value = data;
    item->key = key;

    /* Get the hash */
    int hash_index = hash_code(key);

    /* Move in array until an empty or deleted cell */
    while (hash_array[hash_index] != NULL &&
           hash_array[hash_index] != dummy_item) {
        /* Go to next cell */
        ++hash_index;

        /* Wrap around the table */
        hash_index %= hash_array_size;
    }

    hash_array[hash_index] = item;
}

/**
 * @brief Remove key_value_pair from hash table
 * @param item Pointer to key_value_pair which must be removed from hash table
 * @return struct key_value_pair* Pointer to same key_value_pair or
 * NULL if pair not exists in hash table
 */
static struct key_value_pair *remove_item(struct key_value_pair *item)
{
    int key = item->key;

    /* Get the hash */
    int hash_index = hash_code(key);

    /* Move in array until an empty */
    while (hash_array[hash_index] != NULL) {
        if (hash_array[hash_index]->key == key) {
            struct key_value_pair *temp = hash_array[hash_index];

            /* Assign a dummy item at deleted position */
            hash_array[hash_index] = dummy_item;
            return temp;
        }

        /* Go to next cell */
        ++hash_index;

        /* Wrap around the table */
        hash_index %= hash_array_size;
    }

    return NULL;
}

bool_t session_is_exist(uint16_t id)
{
    return session_get(id) != NULL;
}

struct session_info *session_get(uint16_t id)
{
    struct key_value_pair *pair = find_item(id);

    if (pair == NULL) {
        return NULL;
    }
    return &(pair->value);
}

void session_add(struct session_info session, uint16_t id)
{
    insert_item(id, session);
}

void session_remove(uint16_t id)
{
    struct key_value_pair *pair = find_item(id);
    free(remove_item(pair));
}

void session_init_table(int16_t max_sessions)
{
    hash_array_size = max_sessions;
    hash_array = calloc(hash_array_size, sizeof(struct key_value_pair *));

    dummy_item = malloc(sizeof(struct key_value_pair));
    dummy_item->key = -1;
}
