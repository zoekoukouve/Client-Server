#include "aid_functions.h"
#include <cstring>

#include <sys/ipc.h> //shared memory
#include <sys/shm.h>

using namespace std;  

// Close and unlink semophores
void semaph_close_unlink(void* mutex_writer, void* mutex_finished, void* mutex_diff, int segments_amount, char** sem_names, sem_t** semaph){
     
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

   if (semaph != NULL){
        for (int i = 1; i <= segments_amount; i++){
            if(sem_close(semaph[i]) < 0){
                perror("sem_close() failed");
                exit(EXIT_FAILURE);
            }
            if(sem_unlink(sem_names[i]) < 0){
                perror("sem_unlink() failed");
                exit(EXIT_FAILURE);
            }
            delete [] sem_names[i];
        }
        delete[] sem_names;
        delete[] semaph;   
    }

    return;
}

// Close semophores
void semaph_close(void* mutex_writer, void* mutex_finished, void* mutex_diff, int segments_amount, char** sem_names, sem_t** semaph){
    
   
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

    if (semaph != NULL){
        for (int i = 1; i <= segments_amount; i++){
            if(sem_close(semaph[i]) < 0){
                perror("sem_close() failed");
                exit(EXIT_FAILURE);
            }
        }
        delete semaph;   
    }

    return;
}

// Close semophores in client
void semaph_close_client(void* mutex_writer, void* mutex_finished, void* mutex_diff, int segments_amount, char** sem_names, sem_t** semaph){
    
   
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

    // if (mutex_same != NULL){
    //     if(sem_close((sem_t*)mutex_same) < 0){
    //         perror("sem_close(mutex_same) failed!");
    //         exit(EXIT_FAILURE);
    //     }
    // }

    if (semaph != NULL){
        for (int i = 1; i <= segments_amount; i++){
            if(sem_close(semaph[i]) < 0){
                perror("sem_close() failed");
                exit(EXIT_FAILURE);
            }
            delete [] sem_names[i];
        }
        delete[] sem_names;
        delete[] semaph;   
    }


    return;
}

void return_segment(FILE* fp, int first_line, int last_line,int shm_key, tempSharedMemory shared_mem){
   
    char line[MAX_LINE_SIZE]; 
    int linecounter = 0;
    char* lii;
    
    // Allocate memory for the segment->segment array using new
    //  int segment_lines = last_line - first_line +1;
    // shared_mem->segment = new char*[segment_lines + 1]; // +1 to account for 0-based indexing
    // for (int i = 1; i <= segment_lines; i++) {
    //     shared_mem->segment[i] = new char[MAX_LINE_SIZE];
    // }

    while ((lii=fgets(line, MAX_LINE_SIZE, fp)) != NULL) {
        //cout<<"aaaaaaaaaaaaaaaaaa"<<endl;
		if (linecounter > last_line) {
            // Detach shared memory
            if(shmdt((void*)shared_mem) == -1){
                perror("Failed to destroy shared memory segment");
                return;
            }
            cout<<"efygaaaaaaaaaaaaaaaaaaaaa"<<endl;
            fflush(stdout);
            return;
		} else if (linecounter < first_line){
            // do nothing
        } else{
            cout << linecounter - first_line +1;
            //cout << "re mlka";

            fflush(stdout);

            strcpy(shared_mem->segment[linecounter - first_line +1],lii);
            cout << shared_mem->segment[linecounter - first_line +1];
            fflush(stdout);
  
        }
        
		linecounter++;
       // cout << linecounter <<"zoeeee" << last_line << endl;
        fflush(stdout);
	}
       
    
    //Detach shared memory
    if(shmdt((void*)shared_mem) == -1){
        perror("Failed to destroy shared memory segment");
        return;
    }

}
