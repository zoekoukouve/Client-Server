#include "aid_functions.h"
#include <cstring>

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


void return_segment(FILE* fp, int first_line, int last_line, char** temp_memory){
   
    char line[MAX_LINE_SIZE]; 
    int linecounter = 0;
    char* lii;
    
    while ((lii=fgets(line, MAX_LINE_SIZE, fp)) != NULL) {
		if (linecounter == last_line + 1) {
			return;
		} else if (linecounter < first_line){
            // do nothing
        } else{
            cout << linecounter-first_line;
            cout << lii;

            fflush(stdout);
            
           
            strncpy(temp_memory[linecounter - first_line], lii, MAX_LINE_SIZE - 1);
            temp_memory[linecounter - first_line][MAX_LINE_SIZE - 1] = '\0';
            return;
            
        }
        
		linecounter++;
	}
}
