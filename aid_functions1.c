//#include "aid_functions.h"
// Function to count lines of a file
int file_lines(FILE* fp){
    if (fp == NULL){
        printf("error, NULL file");
        return -1;
    }
    
    int flag = 0; 
    int lines = 0; 
    for (char c = getc(fp); c != EOF; c = getc(fp)){
        flag = 1;
        if (c == '\n'){ // Increment count if this character is newline
            lines++;
            flag = 0;
        }
    }

    if (flag == 1) // If last line doesn't end with '\n'
        lines++;
    return lines;
}


// Choose the num-th segment
char* ret_line(char** segm, int num){
    return segm[num];
}

// Split the file to segments
char*** split_segments(FILE* fp, int segments_amount, int katatmhsh){
    char line[MAX_LINE_SIZE]; 

    // allocate memory
    char*** segm = malloc((segments_amount + 1)* sizeof(char**)); 
    for (int i = 1; i <= segments_amount; i++){
        segm[i] = malloc((katatmhsh + 1)* sizeof(char*));
    }

	int filecounter = 1;
    int linecounter = 1;

    // split segments
    char* lii;
    while ((lii=fgets(line, MAX_LINE_SIZE, fp)) != NULL) {
		if (linecounter == katatmhsh + 1) {
			linecounter = 1;
			filecounter++;
		}
        segm[filecounter][linecounter] = malloc(MAX_LINE_SIZE*sizeof(char));
        strcpy(segm[filecounter][linecounter],lii);
		linecounter++;
	}

    // if last segment is not fully completed add line 
    while (linecounter != katatmhsh + 1){
        segm[filecounter][linecounter] = malloc(20*sizeof(char));
        strcpy(segm[filecounter][linecounter],"placeholer  \n");
		linecounter++;
    }

    return segm;
}

// Close and unlink semophores
void semaph_close_unlink(int segments_amount, char** sem_names, sem_t** semaph, void* mutex_writer, void* mutex_count, void* mutex_finished, void* mutex_diff, void* mutex_same){
    
    if (mutex_count != NULL){
        if(sem_close(mutex_count) < 0){
            perror("sem_close(mutex_count) failed on parent");
            exit(EXIT_FAILURE);
        }
        if(sem_unlink("mutex_count") < 0){
            perror("sem_unlink(mutex_count) failed");
            exit(EXIT_FAILURE);
        }
    }    

    if (mutex_writer != NULL){
        if(sem_close(mutex_writer) < 0){
            perror("sem_close(mutex_writer) failed!");
            exit(EXIT_FAILURE);
        }
        if(sem_unlink("mutex_writer") < 0){
            perror("sem_unlink(mutex_writer) failed");
            exit(EXIT_FAILURE);
        } 
    }

    if (mutex_finished != NULL){
        if(sem_close(mutex_finished) < 0){
            perror("sem_close(mutex_finished) failed on child");
            exit(EXIT_FAILURE);
        }
        if(sem_unlink("mutex_finished") < 0){
            perror("sem_unlink(mutex_finished) failed");
            exit(EXIT_FAILURE);
        }
    }

    if (mutex_diff != NULL){
        if(sem_close(mutex_diff) < 0){
            perror("sem_close(mutex_diff) failed!");
            exit(EXIT_FAILURE);
        }
        if(sem_unlink("mutex_diff") < 0){
            perror("sem_unlink(mutex_diff) failed");
            exit(EXIT_FAILURE);
        } 
    }

    if (mutex_same != NULL){
        if(sem_close(mutex_same) < 0){
            perror("sem_close(mutex_same) failed!");
            exit(EXIT_FAILURE);
        }
        if(sem_unlink("mutex_same") < 0){
            perror("sem_unlink(mutex_same) failed");
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
            free(sem_names[i]);
        }
    free(sem_names);
    free(semaph);   
    }

    return;
}

// Close sempohores
void semaph_close(int segments_amount, sem_t** semaph, void* mutex_writer, void* mutex_count, void* mutex_finished, void* mutex_diff, void* mutex_same){
    
    if (mutex_count != NULL){
        if(sem_close(mutex_count) < 0){
            perror("sem_close(mutex_count) failed on parent");
            exit(EXIT_FAILURE);
        }
    }    

    if (mutex_writer != NULL){
        if(sem_close(mutex_writer) < 0){
            perror("sem_close(mutex_writer) failed!");
            exit(EXIT_FAILURE);
        }
    }

    if (mutex_finished != NULL){
        if(sem_close(mutex_finished) < 0){
            perror("sem_close(mutex_finished) failed on child");
            exit(EXIT_FAILURE);
        }
    }

    if (mutex_diff != NULL){
        if(sem_close(mutex_diff) < 0){
            perror("sem_close(mutex_diff) failed!");
            exit(EXIT_FAILURE);
        }
    }

    if (mutex_same != NULL){
        if(sem_close(mutex_same) < 0){
            perror("sem_close(mutex_same) failed!");
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
    free(semaph);   
    }

    return;
}