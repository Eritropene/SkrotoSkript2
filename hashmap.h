// hashmap.h
#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "skrotoskript2.h"

#define TABLE_SIZE 32  // Number of buckets in the hash table

// Hash map node
typedef struct Node {
    char* key;
    Code* value;
    struct Node* next;
} Node;

// Hash map structure
typedef struct HashMap {
    Node* table[TABLE_SIZE];
} HashMap;

// Hash function to generate an index from the key
unsigned int hash(char* key) {
    unsigned int hash = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        hash = hash * 31 + key[i];  // A basic hash function using prime multiplier 31
    }
    return hash % TABLE_SIZE;
}

// Create a new node with the given key and value
Node* create_node(char* key, Code* value) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) {
        return NULL;  // Memory allocation failed
    }
    new_node->key = strdup(key);   // Allocate memory for the key
    new_node->value = value;       // Allocate memory for the value
    new_node->next = NULL;
    return new_node;
}

// Initialize the hash map
HashMap* create_hashmap() {
    HashMap* map = (HashMap*)malloc(sizeof(HashMap));
    if (!map) {
        return NULL;  // Memory allocation failed
    }
    for (int i = 0; i < TABLE_SIZE; i++) {
        map->table[i] = NULL;
    }
    return map;
}

// Insert key-value pair into the hash map
void insert(HashMap* map, char* key, Code* value) {
    unsigned int index = hash(key);  // Get the hash index for the key
    Node* current = map->table[index];

    // Check if the key already exists and update the value if it does
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            current->value = value;  // Update with the new value
            return;
        }
        current = current->next;
    }

    // If key does not exist, create a new node and insert at the beginning of the list
    Node* new_node = create_node(key, value);
    if (!new_node) {
        fprintf(stderr, "Error: Could not create a new node\n");
        return;
    }
    new_node->next = map->table[index];
    map->table[index] = new_node;
}

// Search for a value by key in the hash map
Code* search(HashMap* map, char* key) {
    unsigned int index = hash(key);  // Get the hash index for the key
    Node* current = map->table[index];

    // Traverse the linked list to find the key
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return current->value;  // Key found, return the value
        }
        current = current->next;
    }

    // Key not found
    return NULL;
}

// Free the memory used by the hash map
void free_hashmap(HashMap* map) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Node* current = map->table[i];
        while (current != NULL) {
            Node* next = current->next;
            free(current->key);
            free(current->value->jumps);
            free(current->value);
            free(current);
            current = next;
        }
    }
    free(map);
}

#endif // HASHMAP_H
