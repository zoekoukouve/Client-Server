// #include "shared_memory.h"
#include "aid_functions.h"
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
    void* mutex_r;
    void* mutex_w;
    pid_t key;
    tempSharedMemory shared_mem;
};

vector<string> filenames;

void child(FILE *,int, int, int, sharedMemory, void*, void*, void*, void*, void*, int, double);

void* threadFunction(void* arg) {
    
    // Extract the data from the argument
    CallData* lineData = (CallData*)arg;
    int first_line = lineData->first_line;
    int last_line = lineData->last_line;
    int wanted_file =  lineData->wanted_file;
    pid_t key =  lineData->key;
    tempSharedMemory shared_mem = lineData->shared_mem;

    string sfilename = filenames[wanted_file];
    const char* filename = sfilename.c_str(); 

    FILE* fp = fopen(filename, "r");
    if (fp == NULL){
        printf("Could not open file %s", filename);
        pthread_exit(NULL);
    }
    
    return_segment(fp, first_line, last_line, (int)key, shared_mem, lineData->mutex_r, lineData->mutex_w);
    fclose(fp);

    pthread_exit(NULL);
}


void parent(int clients, int files, int requests, double lamda){
    
    // Create and initialize semaphores
    sem_t* mutex_writer = sem_open("mutex_writer", O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    if(mutex_writer == SEM_FAILED){
        perror("sem_open(mutex_writer) failed on parent");
        exit(EXIT_FAILURE);
    }

    sem_t* mutex_finished = sem_open("mutex_finished", O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    if(mutex_finished == SEM_FAILED){
        semaph_close_unlink(mutex_writer, NULL, NULL, clients, NULL, NULL, NULL, NULL); // Close and unlink already created semophores           
        perror("sem_open(mutex_finished) failed on parent");
        exit(EXIT_FAILURE);
    } 
    
    sem_t* mutex_diff = sem_open("mutex_diff", O_CREAT | O_EXCL, SEM_PERMS, 0);
    if(mutex_diff == SEM_FAILED){
        semaph_close_unlink(mutex_writer, mutex_finished, NULL, clients, NULL, NULL, NULL, NULL); // Close and unlink already created semophores           
        perror("sem_open(mutex_diff) failed on parent");
        exit(EXIT_FAILURE);
    }


    // Array of semaphores, one for each client
    sem_t** semaph_r = new sem_t*[clients+1]; 
    sem_t** semaph_w = new sem_t*[clients+1];   
    char** sem_names_r = new char*[clients + 1];
    char** sem_names_w = new char*[clients + 1];
    for (int i = 1; i <=  clients; i++){
        sem_names_r[i] = new char[15];    // Keeps the name of semophore
        sprintf(sem_names_r[i], "semaph_r%d", i);
        semaph_r[i] = sem_open(sem_names_r[i], O_CREAT | O_EXCL, SEM_PERMS, 0);    // Create semophore

        if(semaph_r[i] == SEM_FAILED){        // Close and unlink already created semophores and deallocate memory 
            delete[] sem_names_r[i];
            semaph_close_unlink(mutex_writer, mutex_finished, mutex_diff, i-1, sem_names_r, semaph_r, sem_names_w, semaph_w);
            perror("sem_open(semaph) failed");
            exit(EXIT_FAILURE);
        }   

        sem_names_w[i] = new char[15];    // Keeps the name of semophore
        sprintf(sem_names_w[i], "semaph_w%d", i);
        semaph_w[i] = sem_open(sem_names_w[i], O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);    // Create semophore

        if(semaph_w[i] == SEM_FAILED){        // Close and unlink already created semophores and deallocate memory 
            delete[] sem_names_w[i];
            semaph_close_unlink(mutex_writer, mutex_finished, mutex_diff, i-1, sem_names_r, semaph_r, sem_names_w, semaph_w);
            perror("sem_open(semaph) failed");
            exit(EXIT_FAILURE);
        }   
    }


    int shmid;
    sharedMemory shared_memory;

    // Create memory segment
    if((shmid = shmget(IPC_PRIVATE, sizeof(shared_memory), (S_IRUSR|S_IWUSR))) == -1){
        semaph_close_unlink(mutex_writer, mutex_finished, mutex_diff, clients, sem_names_r, semaph_r, sem_names_w, semaph_w);
        perror("Failed to create shared main memory segment");
        return;
    }

    // Attach memory segment
    if((shared_memory = (sharedMemory)shmat(shmid, NULL, 0)) == (void*)-1){
        semaph_close_unlink(mutex_writer, mutex_finished, mutex_diff, clients, sem_names_r, semaph_r, sem_names_w, semaph_w);
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
    FILE *writefile;
	char filenames[17];

    for(int i = 0; i < clients; i++){
        if((pids[i] = fork()) < 0){ // Fork new process
            semaph_close_unlink(mutex_writer, mutex_finished, mutex_diff, clients, sem_names_r, semaph_r, sem_names_w, semaph_w);
            perror("Failed to create process");
            return;
        }
        if(pids[i] == 0){          // If it is child process
            sprintf(filenames, "file_%d", i);       // Record file
	        writefile = fopen(filenames, "w");
            child(writefile, clients, requests, files, shared_memory, mutex_writer, mutex_finished, mutex_diff, semaph_r[i+1], semaph_w[i+1], i+1, lamda);
            semaph_close_client(mutex_writer, mutex_finished, mutex_diff, clients, sem_names_r, semaph_r, sem_names_w, semaph_w);
            exit(0);
        }
    }



    // Services
    pthread_t subThreads[clients*requests];
    int i = 0;
    vector<CallData*> data;  // Temporary vector to store data of each line
    CallData* callData;    

    // All child have been created
    if(sem_post(mutex_writer) < 0){
        perror("sem_post failed on parent");
        exit(EXIT_FAILURE);
    }

    // Check if chlidren are finished
    if(sem_wait(mutex_finished) < 0){
        perror("sem_post failed on parent");
        exit(EXIT_FAILURE);
    }
    
    while (shared_memory->finished < clients){

        if(sem_post(mutex_finished) < 0){
            perror("sem_post failed on parent");
            exit(EXIT_FAILURE);
        } 
        
          
        if(sem_wait(mutex_diff) < 0){
            perror("sem_wait failed on child, mutex_diff");
            exit(EXIT_FAILURE);
        }          
            
        if (shared_memory->temp_mem_used == 1){
           
            callData = new CallData();  // Thread's argument
            
            callData->last_line = shared_memory->end_line;
            callData->first_line = shared_memory->start_line;
            callData->wanted_file = shared_memory->file_num;
            callData->key = shared_memory->temp_shared_mem_key;
            callData->mutex_r = semaph_r[shared_memory->sem_id];   // Semaphores of shared memory
            callData->mutex_w = semaph_w[shared_memory->sem_id];
            // cout << "File " << callData->wanted_file << "lines: " << callData->first_line << callData->last_line << endl;
            data.push_back(callData);

            int shmid_s = shmget(shared_memory->temp_shared_mem_key, sizeof(temp_shared_memory), 0666 | IPC_CREAT );
            if (shmid_s == -1) {
                perror("Failed to get shared memory segment");
                return ;
            }

            // Attach the shared memory segment to the process's address space
            tempSharedMemory shared_mem = (tempSharedMemory)shmat(shmid_s, NULL, 0);
            if (shared_mem == reinterpret_cast<temp_shared_memory*>(-1)) {
                perror("Failed to attach shared memory in server");
                return;
            }

            callData->shared_mem = shared_mem;
            
            // Create the thread
            if (pthread_create(&subThreads[i], NULL, threadFunction, (void*)data[i]) != 0) {
                cerr << "Error creating sub thread " << i << endl;
                return;
            }

            i++;

           shared_memory->temp_mem_used = 0;

           
           if(sem_post((sem_t*)mutex_writer) < 0){
                perror("sem_post failed on child, semaph[wanted_seg]");
                exit(EXIT_FAILURE);
            }

        }  

        // Check if chlidren are finished
        if(sem_wait(mutex_finished) < 0){
            perror("sem_post failed on parent");
            exit(EXIT_FAILURE);
        }      

    }
    

    // Wait for sub threads to finish
    for (int i = 0; i < clients*requests; ++i) {
        pthread_join(subThreads[i], NULL);
    }

    // Delete arguments of the threads
    for (int i = 0; i < clients*requests; ++i) {
        delete data[i];
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

    // Delete the shared memory 
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        semaph_close_unlink(mutex_writer, mutex_finished, mutex_diff, clients, sem_names_r, semaph_r, sem_names_w, semaph_w);
        perror("Failed to delete shared memory segment");
        return;
    }

    // Close and unlink semaphores
    semaph_close_unlink(mutex_writer, mutex_finished, mutex_diff, clients, sem_names_r, semaph_r, sem_names_w, semaph_w);
}

int main(int argc, char** argv){

    int N = atoi(argv[1]);
    int M = atoi(argv[2]);
    int L = atoi(argv[3]);
    double lamda = stod(argv[4]);
    
    for (int i = 0; i < M; i++) {
        filenames.push_back(argv[i + 5]);
    }

    parent(N, M, L, lamda);

}