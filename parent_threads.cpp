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
    void* mutex_s;
    pid_t key;
    tempSharedMemory shared_mem;
};

vector<string> filenames;

void child(FILE *,int, int, int, sharedMemory, void*, void*, void*, void*);

void* threadFunction(void* arg) {
    //cout << "dhiiiiiiiiiiiiiiiiiiiiiiiiiiii"<<endl;
    
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
    
    return_segment(fp, first_line, last_line, (int)key, shared_mem);
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
        semaph_close_unlink(mutex_writer, NULL, NULL, clients, NULL, NULL); // Close and unlink already created semophores           
        perror("sem_open(mutex_finished) failed on parent");
        exit(EXIT_FAILURE);
    } 
    
    sem_t* mutex_diff = sem_open("mutex_diff", O_CREAT | O_EXCL, SEM_PERMS, 0);
    if(mutex_diff == SEM_FAILED){
        semaph_close_unlink(mutex_writer, mutex_finished, NULL, clients, NULL, NULL); // Close and unlink already created semophores           
        perror("sem_open(mutex_diff) failed on parent");
        exit(EXIT_FAILURE);
    }


    // Array of semaphores, one for each client
    sem_t** semaph = new sem_t*[clients+1];    
    char** sem_names = new char*[clients + 1];
    for (int i = 1; i <=  clients; i++){
        sem_names[i] = new char[15];    // Keeps the name of semophore
        sprintf(sem_names[i], "semaph%d", i);
        semaph[i] = sem_open(sem_names[i], O_CREAT | O_EXCL, SEM_PERMS, 0);    // Create semophore

        if(semaph[i] == SEM_FAILED){        // Close and unlink already created semophores and deallocate memory 
            free(sem_names[i]);
            semaph_close_unlink(mutex_writer, mutex_finished, mutex_diff, i-1, sem_names, semaph);
            perror("sem_open(semaph) failed");
            exit(EXIT_FAILURE);
        }   
    }


    int shmid;
    sharedMemory shared_memory;

    // Create memory segment
    if((shmid = shmget(IPC_PRIVATE, sizeof(shared_memory), (S_IRUSR|S_IWUSR))) == -1){
        semaph_close_unlink(mutex_writer, mutex_finished, mutex_diff, clients, sem_names, semaph);
        perror("Failed to create shared main memory segment");
        return;
    }

    // Attach memory segment
    if((shared_memory = (sharedMemory)shmat(shmid, NULL, 0)) == (void*)-1){
        semaph_close_unlink(mutex_writer, mutex_finished, mutex_diff, clients, sem_names, semaph);
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
            semaph_close_unlink(mutex_writer, mutex_finished, mutex_diff, clients, sem_names, semaph);
            perror("Failed to create process");
            return;
        }
        if(pids[i] == 0){          // If it is child process
            sprintf(filenames, "file_%d", i);       // Record file
	        writefile = fopen(filenames, "w");
            child(writefile, clients, requests, files, shared_memory, mutex_writer, mutex_finished, mutex_diff, semaph[i+1]);
            exit(0);
        }
    }



    // Services

    vector<pthread_t> subThreads(clients*requests);  // Vector to store worker threads
    int i = 0;
    vector<CallData*> data;  // Temporary vector to store data of each line
    CallData* callData;    

    // All child have been created
    if(sem_post(mutex_writer) < 0){
        perror("sem_post failed on parent");
        exit(EXIT_FAILURE);
    }
    
    while (shared_memory->finished < clients){

        cout<<"filarakia m "<<endl;
          
        if(sem_wait(mutex_diff) < 0){
            perror("sem_wait failed on child, mutex_diff");
            exit(EXIT_FAILURE);
        }          
            
        //if (shared_memory->temp_mem_used == 1){
           
            callData = new CallData();  // Allocate memory 
            
            callData->last_line = shared_memory->end_line;
            callData->first_line = shared_memory->start_line;
            callData->wanted_file = shared_memory->file_num;
            callData->key = shared_memory->temp_shared_mem_key;
            callData->mutex_s = shared_memory->mutex_s;
            cout << "File " << callData->wanted_file << "lines: " << callData->first_line << callData->last_line << endl;
            data.push_back(callData);

///////////////////////////////////////////////////thread//////////////////////////////////////////////////////////////////////
            int shmid = shmget(shared_memory->temp_shared_mem_key, sizeof(temp_shared_memory), 0666 | IPC_CREAT );
            if (shmid == -1) {
                perror("Failed to get shared memory segment");
                return ;
            }

    // Attach the shared memory segment to the process's address space
            tempSharedMemory shared_mem = (tempSharedMemory)shmat(shmid, NULL, 0);
            if (shared_mem == reinterpret_cast<temp_shared_memory*>(-1)) {
                perror("Failed to attach shared memory in server");
                return;
            }
            cout << endl << shared_memory->temp_shared_mem_key << " "<< shared_mem ->k << endl;

            callData->shared_mem = shared_mem;
            
            if (pthread_create(&subThreads[i], NULL, threadFunction, (void*)data[i]) != 0) {
                cerr << "Error creating sub thread " << i << endl;
                return;
            }

            i++;
          
           // string sfilename = filenames[wanted_file];
           cout << "vghkaaaaaaaaaaaaaaa"<<endl;


          // shared_memory->temp_mem_used = 0;


           if(sem_post((sem_t*)mutex_writer) < 0){
           // semaph_close_client(mutex_writer, mutex_finished, mutex_diff, mutex_same);
                perror("sem_post failed on child, semaph[wanted_seg]");
                exit(EXIT_FAILURE);
            }

        //}        

    }

  //  cout << endl <<" alelouiaaaaaaaaaaaaa tsirp"<< endl;

    // Wait for sub threads to finish
    for (int i = 0; i < clients*requests; ++i) {
        pthread_join(subThreads[i], NULL);
    }
    
    int status;

  //  cout << endl <<" alelouiaaaaaaaaaaaaa"<< endl;
   //  fflush(stdout);

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
    semaph_close_unlink(mutex_writer, mutex_finished, mutex_diff, clients, sem_names, semaph);
}

int main(int argc, char** argv){

    int N = atoi(argv[1]);
    int M = atoi(argv[2]);
    int L = atoi(argv[3]);
    cout << N <<M<<L << endl;

    
    for (int i = 0; i < M; i++) {
        filenames.push_back(argv[i + 4]);
    }

    if(sem_unlink("semaph1") < 0){
            perror("sem_unlink(1) failed");
          //  exit(EXIT_FAILURE);
    }
    if(sem_unlink("semaph2") < 0){
            perror("sem_unlink(2) failed");
           // exit(EXIT_FAILURE);
    }
    if(sem_unlink("semaph3") < 0){
            perror("sem_unlink(3) failed");
           // exit(EXIT_FAILURE);
    }
    if(sem_unlink("semaph4") < 0){
            perror("sem_unlink(3) failed");
           // exit(EXIT_FAILURE);
    }
    if(sem_unlink("semaph5") < 0){
            perror("sem_unlink(3) failed");
           // exit(EXIT_FAILURE);
    }
sem_unlink("semaph6");
sem_unlink("semaph7");
    sem_unlink("mutex_writer");
    sem_unlink("mutex_finished");
    sem_unlink("mutex_diff");
    sem_unlink("mutex_same");    
    parent(N, M, L);

}