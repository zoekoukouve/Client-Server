#include "aid_functions.h"

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