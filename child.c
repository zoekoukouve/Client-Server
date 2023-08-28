#include "shared_memory.h"
#include "aid_functions.h"


void child(FILE* writefile, char*** segm, char** sem_names, int N, int K, int segments_amount, int katatmhsh, sharedMemory shared_memory, sem_t** semaph, void* mutex_writer, void* mutex_count, void* mutex_finished, void* mutex_diff, void* mutex_same){
    
    struct timeval request;
    struct timeval reply;
    
    // first request
    srand(time(NULL) + getpid()); // Randomize seed

    int wanted_seg = rand()%segments_amount + 1; // Random segment
    int nline = rand()%(katatmhsh) + 1; // Random Line
    gettimeofday(&request, NULL);

    // FIFO
    if(sem_wait(semaph[wanted_seg]) < 0){
        perror("sem_wait failed on child, semaph[wanted_seg]");
        exit(EXIT_FAILURE);
    }
    
    // If the requested segment is not in shared memory, child asks for that
    if(wanted_seg != shared_memory->segment_num){
        if(sem_wait(mutex_writer) < 0){
            perror("sem_wait failed on child, mutex_writer");
            exit(EXIT_FAILURE);
        }
        shared_memory->wanted_segment_num = wanted_seg;
                
        if(sem_post(mutex_diff) < 0){   // Communication parent - child
            perror("sem_post failed on child, semaph[wanted_seg]");
            exit(EXIT_FAILURE);
        }
        
        //sem_wait(mutex_same);
        if(sem_wait(mutex_same) < 0){
            perror("sem_wait failed on child, mutex_same");
            exit(EXIT_FAILURE);
        }
    
    }
    
    // This child as a reader of the current memory
    if(sem_wait(mutex_count) < 0){
            perror("sem_wait failed on child, mutex_count");
            exit(EXIT_FAILURE);
    }
    shared_memory->count++;
    if(sem_post(semaph[wanted_seg]) < 0){   // Multiple readers
        perror("sem_post failed on child, semaph[wanted_seg]");
        exit(EXIT_FAILURE);
    }
    if(sem_post(mutex_count) < 0){
        perror("sem_post failed on child, mutex_count");
        exit(EXIT_FAILURE);
    }

    // First request
    char* line = ret_line(shared_memory->segment, nline);     // Requested line
    gettimeofday(&reply, NULL);
    fprintf(writefile,"request time:%ld sec %ld usec  reply time:%ld sec %ld usec  <%d,%d>  %s\n",request.tv_sec, request.tv_usec, reply.tv_sec, reply.tv_usec, wanted_seg, nline,line);     // Record
    usleep(20000);             

    // Other requests Requests     
    for(int i = 1; i < N; i++){
        if (rand()%10 < 7){         // probability 0.7 to ask for the same segment
            nline = rand()%(katatmhsh) + 1; 
            gettimeofday(&request, NULL);
            
        }
        else{                       // probability 0.3 to ask for a new segment
            int nwanted_seg = rand()%segments_amount + 1; // New segment
            if (nwanted_seg == wanted_seg){     // if the new segment happens to be the same with the previous
                nline = rand()%(katatmhsh) + 1; 
                gettimeofday(&request, NULL);
            }
            else{
                wanted_seg = nwanted_seg;
                nline = rand()%(katatmhsh) + 1;
                gettimeofday(&request, NULL);

                if(sem_wait(mutex_count) < 0){              // Remove this child as a reader of the previous segment
                    perror("sem_wait failed on child, mutex_count");
                    exit(EXIT_FAILURE);
                }
                shared_memory->count--;
                if (shared_memory->count == 0){                   // FIFO
                    if(sem_post(mutex_writer) < 0){
                        perror("sem_post failed on child, semaph[wanted_seg]");
                        exit(EXIT_FAILURE);
                    }
                }
                if(sem_post(mutex_count) < 0)
                    perror("sem_post failed on child, mutex_count");


                if(sem_wait(semaph[wanted_seg]) < 0){
                    perror("sem_wait failed on child, semaph[wanted_seg]");
                    exit(EXIT_FAILURE);
                }

                if(sem_wait(mutex_writer) < 0){
                    perror("sem_wait failed on child, mutex_writer");
                    exit(EXIT_FAILURE);
                }       
                shared_memory->wanted_segment_num = wanted_seg;
                          
                if(sem_post(mutex_diff) < 0){   // Communucation parent - child
                    perror("sem_post failed on child, semaph[wanted_seg]");
                    exit(EXIT_FAILURE);
                }
                
                if(sem_wait(mutex_same) < 0){
                    perror("sem_wait failed on child, mutex_same");
                    exit(EXIT_FAILURE);
                }
                

                 // This child as a reader of the current memory
                if(sem_wait(mutex_count) < 0){
                    perror("sem_wait failed on child, mutex_count");
                    exit(EXIT_FAILURE);
                }
                shared_memory->count++;
                if(sem_post(semaph[wanted_seg]) < 0){   // Multiple readers
                    perror("sem_post failed on child, semaph[wanted_seg]");
                    exit(EXIT_FAILURE);
                }
                if(sem_post(mutex_count) < 0){
                    perror("sem_post failed on child, mutex_count");
                    exit(EXIT_FAILURE);
                }
                
            } 
        }
        
        // Reply
        line = ret_line(shared_memory->segment, nline);
        gettimeofday(&reply, NULL);
        fprintf(writefile,"request time:%ld sec %ld usec  reply time:%ld sec %ld usec  <%d,%d>  %s\n",request.tv_sec, request.tv_usec, reply.tv_sec, reply.tv_usec, wanted_seg, nline,line);     // Record
        usleep(20000);      // Wait 20 ms   /////////////////////////////////////////////////////////////////////////////////////////
    }


    // The child does not need access to the memory anymore - reduse readers
    if(sem_wait(mutex_count) < 0){
        perror("sem_wait failed on child, mutex_count");
        exit(EXIT_FAILURE);
    }
    shared_memory->count--;
    if (shared_memory->count == 0){   
        if(sem_post(mutex_writer) < 0){
            perror("sem_post failed on child, semaph[wanted_seg]");
            exit(EXIT_FAILURE);
        }
    }
    if(sem_post(mutex_count) < 0)
        perror("sem_post failed on child, mutex_count");
          
    

    // The child has been served
    if(sem_wait(mutex_finished) < 0){
        perror("sem_wait failed on child, mutex_finished");
        exit(EXIT_FAILURE);
    }
    shared_memory->finished++;

    if(sem_post(mutex_finished) < 0)
        perror("sem_post failed on child, mutex_finished");

    if (shared_memory->finished == K){        // Parent procedure should be unblocked
        sem_post(mutex_diff);
    }

    // Close semaphores used by this child 
    semaph_close(segments_amount, semaph, mutex_writer, mutex_count, mutex_finished, mutex_diff, mutex_same);

    for (int i = 1; i <= segments_amount; i++){     // Deallocate memory 
        for(int j = 1; j <= katatmhsh; j++)
            free(segm[i][j]);
        free(segm[i]); 
        free(sem_names[i]);
    }
    free(segm);
    free(sem_names);

    fclose(writefile);

    return;
}