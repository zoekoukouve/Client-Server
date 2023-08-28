#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <time.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/wait.h>


#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

#define MAX_LINE_SIZE 65536 // 64KBytes

// Function to count lines of a file
int file_lines(FILE*);

// Choose the num-th segment
char* ret_line(char**, int);

// Split the file to segments
char*** split_segments(FILE* , int , int);

// Close and unlink semophores
void semaph_close_unlink(int, char**, sem_t**, void* , void*, void*, void*, void*);

// Close sempohores
void semaph_close(int, sem_t**, void* , void*, void*, void*, void*);