// #include "shared_memory.h"
#include "aid_functions.h"
#include "shared_memory.h"
#include <vector>
#include <string>
#include <cstring>
#include <limits.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h> //threads

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define INITIAL_VALUE 1
#define MAX_LINE_SIZE 65536 // 64KBytes
#define LINES 10

using namespace std;

struct CallData {    // Struct that stores data of each line
    int last_line;
    int first_line;
    int wanted_file;
    char** temp_memory;
    void* mutex_s;
};

vector<string> filenames;

void child(int, int, int, sharedMemory, void*, void*, void*, void*);

void* threadFunction(void* arg) {
    cout << "dhiiiiiiiiiiiiiiiiiiiiiiiiiiii"<<endl;
    
    // Extract the data from the argument
    CallData* lineData = (CallData*)arg;
    int first_line = lineData->first_line;
    int last_line = lineData->last_line;
    int wanted_file =  lineData->wanted_file;
    char** temp_memory=  lineData->temp_memory;

    string sfilename = filenames[wanted_file];
    const char* filename = sfilename.c_str(); 

    FILE* fp = fopen(filename, "r");
    if (fp == NULL){
        printf("Could not open file %s", filename);
        pthread_exit(NULL);
    }
    
    return_segment(fp, first_line, last_line, temp_memory);
    fclose(fp);

    if(sem_post((sem_t*)lineData->mutex_s) < 0){
        perror("sem_post failed on parent");
        exit(EXIT_FAILURE);
    }
                cout<<"dystuxws m "<<endl;
    pthread_exit(NULL);
}


void parent(int clients, int files, int requests){
    
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


    int shmid;
    sharedMemory shared_memory;

    // Create memory segment
    if((shmid = shmget(IPC_PRIVATE, sizeof(sharedMemory), (S_IRUSR|S_IWUSR))) == -1){
        semaph_close_unlink(mutex_writer, mutex_finished, mutex_diff, mutex_same);
        perror("Failed to create shared memory segment");
        return;
    }

    // Attach memory segment
    if((shared_memory = (sharedMemory)shmat(shmid, NULL, 0)) == (void*)-1){
        semaph_close_unlink(mutex_writer, mutex_finished, mutex_diff, mutex_same);
        perror("Failed to attach memory segment");
        return;
    }

  
    // Initialize fields of shared memory
    if(sem_wait(mutex_writer) < 0){            
        perror("sem_wait failed on parent, mutex_count");
        exit(EXIT_FAILURE);
    }
    shared_memory->file_num = -1;
    shared_memory->start_line = -1;
    shared_memory->end_line = -1;
    shared_memory->temp_mem_used = -1;
    shared_memory->temp_mem = NULL;

    if(sem_post(mutex_writer) < 0){
        perror("sem_post failed on parent, mutex_writer");
        exit(EXIT_FAILURE);
    }
    

    if(sem_wait(mutex_finished) < 0){            
        perror("sem_wait failed on parent, mutex_finished");
        exit(EXIT_FAILURE);
    }
    shared_memory->finished = 0;
    if(sem_post(mutex_finished) < 0){
        perror("sem_post failed on parent, mutex_finished");
        exit(EXIT_FAILURE);
    }



    // Initialize K kids

    // Does not allow to any child write to shared memory until all child are created
    if(sem_wait(mutex_writer) < 0){
        perror("sem_wait failed on child, mutex_readers");
        exit(EXIT_FAILURE);
    }
    
    pid_t pids[clients];

    for(int i = 0; i < clients; i++){
        if((pids[i] = fork()) < 0){ // Fork new process
            semaph_close_unlink(mutex_writer, mutex_finished, mutex_diff, mutex_same);
            perror("Failed to create process");
            return;
        }
        if(pids[i] == 0){          // If it is child process
            child(clients, requests, files, shared_memory, mutex_writer, mutex_finished, mutex_diff, mutex_same);
            exit(0);
        }
    }

    // All child have been created
    if(sem_post(mutex_writer) < 0){
        perror("sem_post failed on parent");
        exit(EXIT_FAILURE);
    }


    // Services

    vector<pthread_t> subThreads(clients*requests);  // Vector to store worker threads
    int i = 0;
    vector<CallData*> data;  // Temporary vector to store data of each line
    CallData* callData;    
    
    while (shared_memory->finished < clients){

        cout<<"filarakia m "<<endl;
          
        if(sem_wait(mutex_diff) < 0){
            perror("sem_wait failed on child, mutex_diff");
            exit(EXIT_FAILURE);
        }          

            
        if (shared_memory->temp_mem_used == 1){
            // int last_line = shared_memory->end_line;
            // int first_line = shared_memory->start_line;
            // int wanted_file = shared_memory->file_num;
            // char** temp_memory = shared_memory->temp_mem;
            // cout << "File " << wanted_file << "lines: " << first_line << last_line << endl;

            callData = new CallData();  // Allocate memory 
            
            callData->last_line = shared_memory->end_line;
            callData->first_line = shared_memory->start_line;
            callData->wanted_file = shared_memory->file_num;
            callData->temp_memory = shared_memory->temp_mem;
            callData->mutex_s = mutex_same;
            cout << "File " << callData->wanted_file << "lines: " << callData->first_line << callData->last_line << endl;
            data.push_back(callData);

///////////////////////////////////////////////////thread//////////////////////////////////////////////////////////////////////
            
            
            if (pthread_create(&subThreads[i], NULL, threadFunction, (void*)data[i]) != 0) {
                cerr << "Error creating sub thread " << i << endl;
                return;
            }

            i++;
          
           // string sfilename = filenames[wanted_file];
           cout << "vghkaaaaaaaaaaaaaaa"<<endl;

         } 
        
        // if(sem_post(mutex_same) < 0){
        //     perror("sem_post failed on parent");
        //     exit(EXIT_FAILURE);
        // }
                //cout<<"dystuxws m "<<endl;

    }


    // Wait for sub threads to finish
    for (int i = 0; i < clients*requests; ++i) {
        pthread_join(subThreads[i], NULL);
    }
    
    int status;

    // Collect children that have finished
    for(int i = 0; i < clients; i++){
        wait(&status);
    }

    // Detach shared memory
    if(shmdt((void*)shared_memory) == -1){
       perror("Failed to destroy shared memory segment");
       return;
    }

    // Close and unlink semaphores
    semaph_close_unlink(mutex_writer, mutex_finished, mutex_diff, mutex_same);
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
    cout << N <<M<<L << endl;

    
    for (int i = 0; i < M; i++) {
        filenames.push_back(argv[i + 4]);
    }

    // sem_t* mutex_writer, *mutex_finished, *mutex_diff, *mutex_same;
    // semaph_close_unlink(mutex_writer, mutex_finished, mutex_diff, mutex_same);
    sem_unlink("mutex_writer");
    sem_unlink("mutex_finished");
    sem_unlink("mutex_diff");
    sem_unlink("mutex_same");    
    parent(N, M, L);

}