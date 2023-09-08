#include "aid_functions.h"
#include <cstdlib>   // Include this header for srand and rand functions
#include <ctime>     // Include this header for time function

#include <sys/ipc.h> //shared memory
#include <sys/shm.h>

#define MAX_LINE_SIZE 65536 // 64KBytes

using namespace std;

void child(int clients, int requests, int files_amount, sharedMemory shared_memory, void* mutex_writer, void* mutex_finished, void* mutex_diff, void* mutex_same){

    // struct timeval request;
    // struct timeval reply;
    
    // first request
    srand(time(NULL) + getpid()); // Randomize seed

    // Other requests Requests     
    for(int i = 0; i < requests; i++){
        // Wanted lines
        int file = rand()%files_amount;
        int first_line = rand()%LINES;
        int last_line = rand()%(LINES-first_line+1) + first_line;

        // New segment
        int segment_lines = last_line - first_line +1;
        // cout << i << "size of seg " << segment_lines << endl;
        // char** segment = (char**)malloc((segment_lines+1) * sizeof(char*));
        // for (int i = 1 ; i <= segment_lines; i++){
        //     segment[i] = (char*)malloc(MAX_LINE_SIZE * sizeof(char));
        // }

        // int shmid;
        // tempSharedMemory segment;
        key_t shm_key = (key_t)getpid(); // Replace 12345 with your chosen integer

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

        // // Create memory segment
        // if ((shmid = shmget((key_t)getpid(), sizeof(tempSharedMemory), (IPC_CREAT | 0666))) == -1) {
        //     semaph_close(mutex_writer, mutex_finished, mutex_diff, mutex_same);
        //     perror("Failed to create shared memory segment in client");
        //     return ;
        // }

        // // Attach memory segment
        // if((segment = (tempSharedMemory)shmat(shmid, NULL, 0)) == (void*)-1){
        //     semaph_close(mutex_writer, mutex_finished, mutex_diff, mutex_same);
        //     perror("Failed to attach memory segment in client");
        //     return;
        // }

        // segment->segment = (char**)malloc((1 + segment_lines)* sizeof(char*));
        // for (int i = 1; i <= segment_lines; i++){
        //     segment->segment[i] = (char*)malloc(MAX_LINE_SIZE*sizeof(char));;
        // }
         // Allocate memory for segment->segment
        // segment->segment = new char*[segment_lines + 1]; // +1 to account for 0-based indexing

        // for (int i = 1; i <= segment_lines; i++) {
        //     // Allocate memory for each line
        //     segment->segment[i] = new char[MAX_LINE_SIZE];
        // }

        if(sem_wait((sem_t*)mutex_writer) < 0){
            perror("sem_wait failed on child, mutex_writer");
            exit(EXIT_FAILURE);
        }       

        // // Allocate memory for the segment->segment array using new
        // segment->segment = new char*[segment_lines + 1]; // +1 to account for 0-based indexing
        // for (int i = 1; i <= segment_lines; i++) {
        //     segment->segment[i] = new char[MAX_LINE_SIZE];
        // }


        shared_memory->file_num = file;
        shared_memory->start_line = first_line;
        shared_memory->end_line = last_line;
        shared_memory->temp_mem = NULL;
        shared_memory->temp_shared_mem_key = shm_key;
        shared_memory->temp_mem_used = 1;
        segment->k = shmid;
                    
        if(sem_post((sem_t*)mutex_diff) < 0){   // Communucation parent - child
            perror("sem_post failed on child, semaph[wanted_seg]");
            exit(EXIT_FAILURE);
        }
        
        if(sem_wait((sem_t*)mutex_same) < 0){
            perror("sem_wait failed on child, mutex_same");
            exit(EXIT_FAILURE);
        }
            
        //execute//////////////////////////////////////////

        for (int i = 1; i <= segment_lines; i++){ 
            cout << i << "  " << segment->sample[i] << segment->segment[i]<<endl;//;
            fflush(stdout);
        }  
            
        shared_memory->temp_mem_used = 0;

        cout << "hiiii";
        fflush(stdout);
        // // free temp segment
        // for (int i = 1; i <= segment_lines; i++){
        //     free(segment->segment[i]);
        //     cout << i << "  " <<endl;//;<< segment->segment[i]<<endl;//;
        //     fflush(stdout);
        // }
        // cout << "hiiii";
        // free(segment->segment);
        // cout << "sin boy";

        fflush(stdout);

            // Clean up memory allocated with new
        // for (int i = 1; i <= segment_lines; i++) {
        //     delete[] segment->segment[i];
        // }
        // delete[] segment->segment;
        
        if (shmdt(segment) == -1) {
            semaph_close(mutex_writer, mutex_finished, mutex_diff, mutex_same);
            perror("Failed to detach shared memory");
            return;
        }

        // Delete the shared memory segment
        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
            semaph_close(mutex_writer, mutex_finished, mutex_diff, mutex_same);
            perror("Failed to delete shared memory segment");
            return;
        }


        if(sem_post((sem_t*)mutex_writer) < 0){
            semaph_close(mutex_writer, mutex_finished, mutex_diff, mutex_same);
            perror("sem_post failed on child, semaph[wanted_seg]");
            exit(EXIT_FAILURE);
        }

        
       usleep(20000);      // Wait 20 ms   /////////////////////////////////////////////////////////////////////////////////////////
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
    semaph_close(mutex_writer, mutex_finished, mutex_diff, mutex_same);

    cout <<endl <<" oxi allo paidiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"<<endl;
    fflush(stdout);

    return;
}