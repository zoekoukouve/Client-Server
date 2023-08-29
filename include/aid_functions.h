#include <iostream>
#include <unistd.h>
#include <limits.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

#define MAX_LINE_SIZE 65536 // 64KBytes


// Close and unlink semophores
void semaph_close_unlink(void*, void*, void*, void*);

// Close sempohores
void semaph_close(void*, void*, void*, void*);