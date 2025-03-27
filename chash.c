#include "hash_table.h"
#include "shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define MAX_LINE_LENGTH 256
#define MAX_THREADS 100

// Define the global output file pointer
FILE* output_file = NULL;

// Get current timestamp in milliseconds
uint64_t get_timestamp(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

// Thread argument structure
typedef struct {
    char command[50];
    char name[50];
    uint32_t salary;
} thread_arg_t;

// Thread function
void* process_command(void* arg) {
    thread_arg_t* thread_arg = (thread_arg_t*)arg;
    
    if (strcmp(thread_arg->command, "insert") == 0) {
        insert_record(thread_arg->name, thread_arg->salary);
    } else if (strcmp(thread_arg->command, "delete") == 0) {
        delete_record(thread_arg->name);
    } else if (strcmp(thread_arg->command, "search") == 0) {
        hashRecord* record = search_record(thread_arg->name);
        if (record != NULL) {
            fprintf(output_file, "%u,%s,%u\n", record->hash, record->name, record->salary);
        }
    } else if (strcmp(thread_arg->command, "print") == 0) {
        print_all_records();
    }
    
    free(thread_arg);
    return NULL;
}

int main() {
    // Open input and output files
    FILE* input_file = fopen("commands.txt", "r");
    if (input_file == NULL) {
        fprintf(stderr, "Error opening commands.txt\n");
        return 1;
    }
    
    output_file = fopen("output.txt", "w");
    if (output_file == NULL) {
        fprintf(stderr, "Error opening output.txt\n");
        fclose(input_file);
        return 1;
    }
    
    // Initialize hash table
    init_hash_table();
    
    // Read number of threads
    char line[MAX_LINE_LENGTH];
    if (fgets(line, sizeof(line), input_file) == NULL) {
        fprintf(stderr, "Error reading number of threads\n");
        fclose(input_file);
        fclose(output_file);
        return 1;
    }
    
    int num_threads;
    if (sscanf(line, "threads,%d,0", &num_threads) != 1) {
        fprintf(stderr, "Invalid thread count format\n");
        fclose(input_file);
        fclose(output_file);
        return 1;
    }
    
    if (num_threads <= 0 || num_threads > MAX_THREADS) {
        fprintf(stderr, "Invalid number of threads\n");
        fclose(input_file);
        fclose(output_file);
        return 1;
    }
    
    fprintf(output_file, "Running %d threads\n", num_threads);
    
    // Create thread array
    pthread_t threads[MAX_THREADS];
    int thread_count = 0;
    
    // Process commands
    while (fgets(line, sizeof(line), input_file) != NULL) {
        char command[50], name[50];
        uint32_t salary;
        
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        if (sscanf(line, "%[^,],%[^,],%u", command, name, &salary) != 3) {
            fprintf(stderr, "Invalid command format\n");
            continue;
        }
        
        // Create thread argument
        thread_arg_t* arg = malloc(sizeof(thread_arg_t));
        if (arg == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            continue;
        }
        
        strcpy(arg->command, command);
        strcpy(arg->name, name);
        arg->salary = salary;
        
        // Create thread
        if (pthread_create(&threads[thread_count], NULL, process_command, arg) != 0) {
            fprintf(stderr, "Thread creation failed\n");
            free(arg);
            continue;
        }
        
        thread_count++;
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    fprintf(output_file, "Finished all threads.\n");
    
    // Print final sorted records
    sort_and_print_records();
    
    // Cleanup
    cleanup_hash_table();
    fclose(input_file);
    fclose(output_file);
    
    return 0;
} 