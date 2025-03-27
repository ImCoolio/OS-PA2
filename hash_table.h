#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdint.h>
#include <pthread.h>
#include "rwlock.h"

// Hash table record structure
typedef struct hash_struct {
    uint32_t hash;
    char name[50];
    uint32_t salary;
    struct hash_struct *next;
} hashRecord;

// Function declarations
void init_hash_table(void);
void cleanup_hash_table(void);
void insert_record(const char *name, uint32_t salary);
void delete_record(const char *name);
hashRecord* search_record(const char *name);
void print_all_records(void);
uint32_t jenkins_hash(const char *key);
void sort_and_print_records(void);

#endif // HASH_TABLE_H 