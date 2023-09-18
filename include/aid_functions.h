#include <iostream>
#include <unistd.h>
#include <limits.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "shared_memory.h"

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

#define MAX_LINE_SIZE 65536 // 64KBytes


// Close and unlink semophores
void semaph_close_unlink(void*, void*, void*, int , char** , sem_t**, char** , sem_t**);

// Close semophores on clients
void semaph_close_client(void*, void*, void*, int , char** , sem_t**, char** , sem_t**);

// Returns the requested segment in the selected temporary memory
void return_segment(FILE*, int, int, int, tempSharedMemory, void*, void*);
