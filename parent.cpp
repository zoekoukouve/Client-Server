// #include "shared_memory.h"
#include "aid_functions.h"
#include <vector>
#include <string>
#include <cstring>
#include <limits.h>

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define INITIAL_VALUE 1
#define MAX_LINE_SIZE 65536 // 64KBytes

//void child(FILE *, char***, char**, int, int, int, int, sharedMemory, sem_t**, void*, void*, void*, void*, void*);
using namespace std;

void parent(vector<string>& filenames, int clients, int files, int requests){
    // Create and initialize semaphores
    sem_t* mutex_writer = sem_open("mutex_writer", O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    if(mutex_writer == SEM_FAILED){
        perror("sem_open(mutex_writer) failed on parent");
        exit(EXIT_FAILURE);
    }

    sem_t* mutex_finished = sem_open("mutex_finished", O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    if(mutex_finished == SEM_FAILED){
        semaph_close_unlink(mutex_writer, NULL, NULL, NULL); // Close and unlink already created semophores           
        perror("sem_open(mutex_finished) failed on parent");
        exit(EXIT_FAILURE);
    } 
    
    sem_t* mutex_diff = sem_open("mutex_diff", O_CREAT | O_EXCL, SEM_PERMS, 0);
    if(mutex_diff == SEM_FAILED){
        semaph_close_unlink(mutex_writer, mutex_finished, NULL, NULL); // Close and unlink already created semophores           
        perror("sem_open(mutex_diff) failed on parent");
        exit(EXIT_FAILURE);
    }

    sem_t* mutex_same = sem_open("mutex_same", O_CREAT | O_EXCL, SEM_PERMS, 0);
    if(mutex_same == SEM_FAILED){
        semaph_close_unlink(mutex_writer, mutex_finished, mutex_diff, NULL); // Close and unlink already created semophores           
        perror("sem_open(mutex_same) failed on parent");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char** argv){

    // // open a file
    // char* filename;
    // //filename = argv[1];
    // FILE* fp = fopen(filename, "r");
    // if (fp == NULL){
    //     printf("Could not open file %s", filename);
    //     return -1;
    // }
    // fclose(fp);


    int N = atoi(argv[1]);
    int M = atoi(argv[2]);
    int L = atoi(argv[3]);
    cout << N <<M<<L;

    vector<string> filenames;
    for (int i = 0; i < M; i++) {
        filenames.push_back(argv[i + 4]);
    }

    parent(filenames, N, M, L);

}