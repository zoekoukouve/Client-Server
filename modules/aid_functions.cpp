#include "aid_functions.h"
#include <cstring>
#include "shared_memory.h"

#include <sys/ipc.h> //shared memory
#include <sys/shm.h>

using namespace std;  

// Close and unlink semophores
void semaph_close_unlink(void* mutex_writer, void* mutex_finished, void* mutex_diff, void* mutex_same){
     
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

    if (mutex_same != NULL){
        if(sem_close((sem_t*)mutex_same) < 0){
            perror("sem_close(mutex_same) failed!");
            exit(EXIT_FAILURE);
        }
        if(sem_unlink("mutex_same") < 0){
            perror("sem_unlink(mutex_same) failed");
            exit(EXIT_FAILURE);
        }
    }

   

    return;
}

// Close sempohores
void semaph_close(void* mutex_writer, void* mutex_finished, void* mutex_diff, void* mutex_same){
    
   
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

    if (mutex_same != NULL){
        if(sem_close((sem_t*)mutex_same) < 0){
            perror("sem_close(mutex_same) failed!");
            exit(EXIT_FAILURE);
        }
    }

    return;
}


void return_segment(FILE* fp, int first_line, int last_line, char** temp_memory,int key){
   
    char line[MAX_LINE_SIZE]; 
    int linecounter = 0;
    char* lii;

    // Create memory segment
    int shmid;
    tempSharedMemory shared_mem;
    if((shmid = shmget(key, sizeof(tempSharedMemory), (IPC_CREAT | 0666))) == -1){  // getpid() is used to create different mem segments
        perror("Failed to create shared memory segment in parent");
        return;
    }

    // Attach memory segment
    if((shared_mem = (tempSharedMemory)shmat(shmid, NULL, 0)) == (void*)-1){
        perror("Failed to attach memory segment");
        return;
    }

    //char*** segm = malloc((segments_amount + 1)* sizeof(char**)); 
    shared_mem->segment = (char**)malloc((12)* sizeof(char*));
    while ((lii=fgets(line, MAX_LINE_SIZE, fp)) != NULL) {
		if (linecounter == last_line + 1) {
			return;
		} else if (linecounter < first_line){
            // do nothing
        } else{
            // cout << linecounter-first_line;
            // cout << lii;

            fflush(stdout);
            

            shared_mem->segment[linecounter - first_line +1] = (char*)malloc(MAX_LINE_SIZE*sizeof(char));
            strcpy(shared_mem->segment[linecounter - first_line +1],lii);
            //strcpy(segment->segment[linecounter - first_line +1], lii);
            // temp_memory[linecounter - first_line][MAX_LINE_SIZE - 1] = '\0';
            // return;
            
        }
        
		linecounter++;
	}

    // Detach shared memory
    if(shmdt((void*)shared_mem) == -1){
        perror("Failed to destroy shared memory segment");
        return;
    }

}
