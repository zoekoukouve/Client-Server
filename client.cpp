#include "aid_functions.h"
#include <cstdlib>   // Include this header for srand and rand functions
#include <ctime>     // Include this header for time function
#include <sys/time.h>
#include <cmath>
#include <sys/ipc.h> //shared memory
#include <sys/shm.h>

#define MAX_LINE_SIZE 65536 // 64KBytes

using namespace std;

void child(FILE* writefile, int clients, int requests, int files_amount, sharedMemory shared_memory, void* mutex_writer, void* mutex_finished, void* mutex_diff, void* mutex_reader, void* mutex_writer_s, int id, double lamda){

    // Requests time
    // struct timeval request;
    // struct timeval reply;
    double waiting = 0.0;

    int returned_lines = 0;                 // Counts the returned lines
    int *files = new int[files_amount];     // Usefull to find used files
    for (int i = 0; i < files_amount; i++)
        files[i] = 0;

   
    
    // first request
    srand(time(NULL) + getpid()); // Randomize seed

    // Other requests Requests     
    for(int i = 0; i < requests; i++){
        // Wanted lines
        int file = rand()%files_amount;
        files[file]++;
        int first_line = rand()%LINES;
        int last_line = rand()%(LINES-first_line+1) + first_line;

        // New segment
        int segment_lines = last_line - first_line +1;
        returned_lines += segment_lines;

        key_t shm_key = (key_t)getpid(); // Create a unique memory for each client
       
        // Create or open the shared memory segment 
        int shmid = shmget(shm_key, sizeof(temp_shared_memory), 0666 | IPC_CREAT );
        if (shmid == -1) {
            perror("Failed to create/open shared memory segment");
            return;
        }

        // Attach the shared memory segment to the process's address space
        tempSharedMemory segment = (tempSharedMemory)shmat(shmid, NULL, 0);
        if (segment == reinterpret_cast<temp_shared_memory*>(-1)) {
            perror("Failed to attach shared memory in client");
            return;
        }

        if(sem_wait((sem_t*)mutex_writer) < 0){
            perror("sem_wait failed on child, mutex_writer");
            exit(EXIT_FAILURE);
        }       

        // // Allocate memory for the segment->segment array using new
        // segment->segment = new char*[segment_lines + 1]; // +1 to account for 0-based indexing
        // for (int i = 1; i <= segment_lines; i++) {
        //     segment->segment[i] = new char[MAX_LINE_SIZE];
        // }

        //gettimeofday(&request, NULL);

        shared_memory->file_num = file;
        shared_memory->start_line = first_line;
        shared_memory->end_line = last_line;
        shared_memory->mutex_s = (sem_t*)mutex_reader;
        shared_memory->temp_shared_mem_key = shm_key;
        shared_memory->temp_mem_used = 1;
        shared_memory->sem_id = id;
                    
        if(sem_post((sem_t*)mutex_diff) < 0){   // Communucation parent - child
            perror("sem_post failed on child, semaph[wanted_seg]");
            exit(EXIT_FAILURE);
        }
        
        
        fprintf(writefile,"request  <%d,%d:%d> \n", file, first_line, last_line);     // Record
        fprintf(writefile,"\n");     // Record
       
        
        for (int i = 0; i < segment_lines; i++){ 
            if(sem_wait((sem_t*)mutex_reader) < 0){
                perror("sem_wait failed on child, mutex_reader");
                exit(EXIT_FAILURE);
            }

            //cout << i << "  " << segment->segment << endl;//;
            fprintf(writefile,"%s", segment->segment);     // Record

            //fflush(stdout);
            if(sem_post((sem_t*)mutex_writer_s) < 0){   // Communucation parent - child
                perror("mutex_writer_s failed on child");
                exit(EXIT_FAILURE);
            }
        }  

        fprintf(writefile,"\n\n");
            
        // shared_memory->temp_mem_used = 0;

        
        if (shmdt(segment) == -1) {
            perror("Failed to detach shared memory");
            return;
        }

        // Delete the shared memory segment
        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
            perror("Failed to delete shared memory segment");
            return;
        }


        // if(sem_post((sem_t*)mutex_writer) < 0){
        //     semaph_close_client(mutex_writer, mutex_finished, mutex_diff, mutex_reader);
        //     perror("sem_post failed on child, semaph[wanted_seg]");
        //     exit(EXIT_FAILURE);
        // }

        double w = 1.0 - exp(-lamda*i);
        w = sqrt(w*w);                  // making sure that it is a positive number
        waiting += w;
        usleep(w);
        //usleep(20000);      // Wait 20 ms   /////////////////////////////////////////////////////////////////////////////////////////
    }

          
    

    // The child has been served
    if(sem_wait((sem_t*)mutex_finished) < 0){
        perror("sem_wait failed on child, mutex_finished");
        exit(EXIT_FAILURE);
    }
    shared_memory->finished++;

    if(sem_post((sem_t*)mutex_finished) < 0)
        perror("sem_post failed on child, mutex_finished");

    if (shared_memory->finished == clients){        // Parent procedure should be unblocked
        sem_post((sem_t*)mutex_diff);
    }

    // Close semaphores used by this child 
    //semaph_close_client(mutex_writer, mutex_finished, mutex_diff, mutex_reader);

   // cout <<endl <<" oxi allo paidiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"<<endl;
    fflush(stdout);

    int used_files = 0;
    for (int i = 0; i < files_amount; i++){
        if (files[i] > 0)
            used_files++;
    }
    delete[] files;
    //cout << "From that client used files " << used_files << " returned lines " << returned_lines << endl;
    fprintf(writefile,"Used files %d returned_lines %d average waiting time %lf \n", used_files, returned_lines, waiting/requests); 

    // Detach shared memory
    if(shmdt((void*)shared_memory) == -1){
       perror("Failed to destroy shared memory segment");
       return;
    }

    fclose(writefile);

    return;
}