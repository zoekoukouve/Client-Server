#include "shared_memory.h"
#include "aid_functions.h"
#include <cstdlib>   // Include this header for srand and rand functions
#include <ctime>     // Include this header for time function

#define LINES 4
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
        cout << i << "size of seg " << segment_lines << endl;
        char** segment = (char**)malloc(segment_lines * sizeof(char*));
        for (int i = 0 ; i < segment_lines; i++){
            segment[i] = (char*)malloc(MAX_LINE_SIZE * sizeof(char));
        }


        if(sem_wait((sem_t*)mutex_writer) < 0){
            perror("sem_wait failed on child, mutex_writer");
            exit(EXIT_FAILURE);
        }       
        shared_memory->file_num = file;
        shared_memory->start_line = first_line;
        shared_memory->end_line = last_line;
        shared_memory->temp_mem = segment;
        shared_memory->temp_mem_used = 1;
                    
        if(sem_post((sem_t*)mutex_diff) < 0){   // Communucation parent - child
            perror("sem_post failed on child, semaph[wanted_seg]");
            exit(EXIT_FAILURE);
        }
        
        if(sem_wait((sem_t*)mutex_same) < 0){
            perror("sem_wait failed on child, mutex_same");
            exit(EXIT_FAILURE);
        }
            
        //execute//////////////////////////////////////////

        for (int i = 0 ; i < segment_lines; i++){
            cout << i ;//<< " " << segment[i] << endl;
        }  
            
        shared_memory->temp_mem_used = 0;

        // free temp segment
        for (int i = 0 ; i < segment_lines; i++){
            free(segment[i]);
        }
        free(segment);

        if(sem_post((sem_t*)mutex_writer) < 0){
            perror("sem_post failed on child, semaph[wanted_seg]");
            exit(EXIT_FAILURE);
        }

        
        // Reply
        // line = ret_line(shared_memory->segment, nline);
        // gettimeofday(&reply, NULL);
        // fprintf(writefile,"request time:%ld sec %ld usec  reply time:%ld sec %ld usec  <%d,%d>  %s\n",request.tv_sec, request.tv_usec, reply.tv_sec, reply.tv_usec, wanted_seg, nline,line);     // Record
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


    return;
}