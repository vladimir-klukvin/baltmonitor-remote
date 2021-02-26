/**
 * @file session.c
 * @author V.K.
 * @brief
 * @date 2021-02-26
 *
 * @copyright Copyright. All rights reserved.
 *
 */
#include "session.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 20

struct key_value_pair {
    struct session_info value;
    int16_t key;
};

struct key_value_pair *hash_array[SIZE];
struct key_value_pair *dummy_item;
struct key_value_pair *item;

int16_t hash_code(int16_t key)
{
    return key % SIZE;
}

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
        hash_index %= SIZE;
    }

    return NULL;
}

static void insert_item(int key, struct session_info data)
{
    struct key_value_pair *item = malloc(sizeof(struct key_value_pair));
    item->value = data;
    item->key = key;

    /* Get the hash */
    int hash_index = hash_code(key);

    /* Move in array until an empty or deleted cell */
    while (hash_array[hash_index] != NULL &&
           hash_array[hash_index]->key != -1) {
        /* Go to next cell */
        ++hash_index;

        /* Wrap around the table */
        hash_index %= SIZE;
    }

    hash_array[hash_index] = item;
}

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
        hash_index %= SIZE;
    }

    return NULL;
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
