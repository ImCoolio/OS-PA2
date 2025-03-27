#include "hash_table.h"
#include "shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Global variables
static hashRecord *head = NULL;
static rwlock_t table_lock;
static int lock_acquisitions = 0;
static int lock_releases = 0;

// Get current timestamp in milliseconds - defined in chash.c

void init_hash_table(void) {
    rwlock_init(&table_lock);
}

void cleanup_hash_table(void) {
    hashRecord *current = head;
    while (current != NULL) {
        hashRecord *next = current->next;
        free(current);
        current = next;
    }
    rwlock_cleanup(&table_lock);
}

// Jenkins one-at-a-time hash function
uint32_t jenkins_hash(const char *key) {
    uint32_t hash = 0;
    for (size_t i = 0; key[i] != '\0'; i++) {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

void insert_record(const char *name, uint32_t salary) {
    uint32_t hash = jenkins_hash(name);
    fprintf(output_file, "%llu,INSERT,%s,%u\n", get_timestamp(), name, salary);
    
    rwlock_acquire_writelock(&table_lock);
    lock_acquisitions++;
    
    // Check if record exists
    hashRecord *current = head;
    while (current != NULL) {
        if (current->hash == hash) {
            current->salary = salary;
            rwlock_release_writelock(&table_lock);
            lock_releases++;
            return;
        }
        current = current->next;
    }
    
    // Create new record
    hashRecord *new_record = (hashRecord *)malloc(sizeof(hashRecord));
    if (new_record == NULL) {
        rwlock_release_writelock(&table_lock);
        lock_releases++;
        return;
    }
    
    new_record->hash = hash;
    strncpy(new_record->name, name, 49);
    new_record->name[49] = '\0';
    new_record->salary = salary;
    new_record->next = head;
    
    head = new_record;
    
    rwlock_release_writelock(&table_lock);
    lock_releases++;
}

void delete_record(const char *name) {
    uint32_t hash = jenkins_hash(name);
    fprintf(output_file, "%llu,DELETE,%s\n", get_timestamp(), name);
    
    rwlock_acquire_writelock(&table_lock);
    lock_acquisitions++;
    
    hashRecord *current = head;
    hashRecord *prev = NULL;
    
    while (current != NULL) {
        if (current->hash == hash) {
            if (prev == NULL) {
                head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            break;
        }
        prev = current;
        current = current->next;
    }
    
    rwlock_release_writelock(&table_lock);
    lock_releases++;
}

hashRecord* search_record(const char *name) {
    uint32_t hash = jenkins_hash(name);
    fprintf(output_file, "%llu,SEARCH,%s\n", get_timestamp(), name);
    
    rwlock_acquire_readlock(&table_lock);
    lock_acquisitions++;
    
    hashRecord *current = head;
    while (current != NULL) {
        if (current->hash == hash) {
            rwlock_release_readlock(&table_lock);
            lock_releases++;
            return current;
        }
        current = current->next;
    }
    
    rwlock_release_readlock(&table_lock);
    lock_releases++;
    fprintf(output_file, "No Record Found\n");
    return NULL;
}

void print_all_records(void) {
    rwlock_acquire_readlock(&table_lock);
    lock_acquisitions++;
    
    hashRecord *current = head;
    while (current != NULL) {
        fprintf(output_file, "%u,%s,%u\n", current->hash, current->name, current->salary);
        current = current->next;
    }
    
    rwlock_release_readlock(&table_lock);
    lock_releases++;
}

// Helper function for sorting
static int compare_records(const void *a, const void *b) {
    const hashRecord *record_a = *(const hashRecord **)a;
    const hashRecord *record_b = *(const hashRecord **)b;
    return (record_a->hash > record_b->hash) - (record_a->hash < record_b->hash);
}

void sort_and_print_records(void) {
    rwlock_acquire_readlock(&table_lock);
    lock_acquisitions++;
    
    // Count records
    int count = 0;
    hashRecord *current = head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    
    // Create array of pointers
    hashRecord **records = malloc(count * sizeof(hashRecord *));
    if (records == NULL) {
        rwlock_release_readlock(&table_lock);
        lock_releases++;
        return;
    }
    
    // Fill array
    current = head;
    for (int i = 0; i < count; i++) {
        records[i] = current;
        current = current->next;
    }
    
    // Sort array
    qsort(records, count, sizeof(hashRecord *), compare_records);
    
    // Print sorted records
    fprintf(output_file, "Number of lock acquisitions:  %d\n", lock_acquisitions);
    fprintf(output_file, "Number of lock releases:  %d\n", lock_releases + 1); // Add 1 for the upcoming release
    for (int i = 0; i < count; i++) {
        fprintf(output_file, "%u,%s,%u\n", records[i]->hash, records[i]->name, records[i]->salary);
    }
    
    free(records);
    rwlock_release_readlock(&table_lock);
    lock_releases++;
} 