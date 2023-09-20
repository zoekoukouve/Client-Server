#include "aid_functions.h"
#include <cstring>

#include <sys/ipc.h> //shared memory
#include <sys/shm.h>

using namespace std;  

// Close and unlink semophores
void semaph_close_unlink(void* mutex_writer, void* mutex_finished, void* mutex_diff, int segments_amount, char** sem_names_r, sem_t** semaph_r, char** sem_names_w, sem_t** semaph_w){
     
    if (mutex_writer != NULL){
        if(sem_close((sem_t*)mutex_writer) < 0){
            perror("sem_close(mutex_writer) failed!");
            exit(EXIT_FAILURE);
        }
        if(sem_unlink("mutex_writer") < 0){
            perror("sem_unlink(mutex_writer) failed");
            exit(EXIT_FAILURE);
        } 
    }

    if (mutex_finished != NULL){
        if(sem_close((sem_t*)mutex_finished) < 0){
            perror("sem_close(mutex_finished) failed on child");
            exit(EXIT_FAILURE);
        }
        if(sem_unlink("mutex_finished") < 0){
            perror("sem_unlink(mutex_finished) failed");
            exit(EXIT_FAILURE);
        }
    }

    if (mutex_diff != NULL){
        if(sem_close((sem_t*)mutex_diff) < 0){
            perror("sem_close(mutex_diff) failed!");
            exit(EXIT_FAILURE);
        }
        if(sem_unlink("mutex_diff") < 0){
            perror("sem_unlink(mutex_diff) failed");
            exit(EXIT_FAILURE);
        } 
    }

    if (semaph_r != NULL){
        for (int i = 1; i <= segments_amount; i++){
            if(sem_close(semaph_r[i]) < 0){
                perror("sem_close() failed");
                exit(EXIT_FAILURE);
            }
            if(sem_unlink(sem_names_r[i]) < 0){
                perror("sem_unlink() failed");
                exit(EXIT_FAILURE);
            }
            delete [] sem_names_r[i];
        }
        delete[] sem_names_r;
        delete[] semaph_r;   
    }

    if (semaph_w != NULL){
        for (int i = 1; i <= segments_amount; i++){
            if(sem_close(semaph_w[i]) < 0){
                perror("sem_close() failed");
                exit(EXIT_FAILURE);
            }
            if(sem_unlink(sem_names_w[i]) < 0){
                perror("sem_unlink() failed");
                exit(EXIT_FAILURE);
            }
            delete [] sem_names_w[i];
        }
        delete[] sem_names_w;
        delete[] semaph_w;   
    }

    return;
}

// Close semophores in client
void semaph_close_client(void* mutex_writer, void* mutex_finished, void* mutex_diff, int segments_amount, char** sem_names_r, sem_t** semaph_r, char** sem_names_w, sem_t** semaph_w){
    
   
    if (mutex_writer != NULL){
        if(sem_close((sem_t*)mutex_writer) < 0){
            perror("sem_close(mutex_writer) failed!");
            exit(EXIT_FAILURE);
        }
    }

    if (mutex_finished != NULL){
        if(sem_close((sem_t*)mutex_finished) < 0){
            perror("sem_close(mutex_finished) failed on child");
            exit(EXIT_FAILURE);
        }
    }

    if (mutex_diff != NULL){
        if(sem_close((sem_t*)mutex_diff) < 0){
            perror("sem_close(mutex_diff) failed!");
            exit(EXIT_FAILURE);
        }
    }


    if (semaph_r != NULL){
        for (int i = 1; i <= segments_amount; i++){
            if(sem_close(semaph_r[i]) < 0){
                perror("sem_close() failed");
                exit(EXIT_FAILURE);
            }
            delete [] sem_names_r[i];
        }
        delete[] sem_names_r;
        delete[] semaph_r;   
    }

    if (semaph_w != NULL){
        for (int i = 1; i <= segments_amount; i++){
            if(sem_close(semaph_w[i]) < 0){
                perror("sem_close() failed");
                exit(EXIT_FAILURE);
            }
            delete [] sem_names_w[i];
        }
        delete[] sem_names_w;
        delete[] semaph_w;   
    }

    return;
}

void return_segment(FILE* fp, int first_line, int last_line,int shm_key, tempSharedMemory shared_mem, void* mutex_reader, void* mutex_writer_s){
   
    char line[MAX_LINE_SIZE + 2]; 
    int linecounter = 0;
    char* lii;

    while ((lii=fgets(line, MAX_LINE_SIZE, fp)) != NULL) {
		if (linecounter > last_line) {
            // Detach shared memory
            // if(shmdt((void*)shared_mem) == -1){                         /////////////////////////////////////////////
            //     perror("Failed to destroy shared memory segment");
            //     return;
            // }
            
            return;
		} else if (linecounter < first_line){
            // do nothing
        } else{

            if(sem_wait((sem_t*)mutex_writer_s) < 0){   // Communucation server - client
                perror("sem_wait failed on child, mutex_writer_s");
                exit(EXIT_FAILURE);
            }

            strcpy(shared_mem->segment,lii);

            if (linecounter == last_line){              // safe detach from server
                if(shmdt((void*)shared_mem) == -1){
                    perror("Failed to destroy shared memory segment");
                    return;
                } 
            }

            if(sem_post((sem_t*)mutex_reader) < 0){   // Communucation server - client
                perror("mutex_writer_s failed on child, mutex_reader");
                exit(EXIT_FAILURE);
            }
              
        }
        
		linecounter++;
	}
       
    
    //Detach shared memory
    if(shmdt((void*)shared_mem) == -1){
        perror("Failed to destroy shared memory segment");
        return;
    }

}
