#include "shared_memory.h"
#include "aid_functions.h"

#define INITIAL_VALUE 1
#define MAX_LINE_SIZE 65536 // 64KBytes

void child(FILE *, char***, char**, int, int, int, int, sharedMemory, sem_t**, void*, void*, void*, void*, void*);

int main(int argc, char** argv){

    // Amount of lines
    char* filename;
    filename = argv[1];
    FILE* fp = fopen(filename, "r");
    if (fp == NULL){
        printf("Could not open file %s", filename);
        return -1;
    }
    int lines_amount = file_lines(fp);
    fclose(fp);
    
    // Katatmhsh
    int katatmhsh = atoi(argv[2]);
    if (katatmhsh == 0){
        printf("error, katatmhsh");
        return -1;
    }

    // Amount of segments
    int segments_amount = lines_amount / katatmhsh;    
    if (lines_amount % katatmhsh != 0){
        segments_amount++;
    }

    // Split file to segments segments
    fp = fopen(filename, "r");
    char*** segm = split_segments(fp, segments_amount, katatmhsh);
    fclose(fp);


    // Create and initialize semaphores
    sem_t* mutex_count = sem_open("mutex_count", O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    if(mutex_count == SEM_FAILED){
        perror("sem_open(mutex_count) failed on parent, mutex_count");
        exit(EXIT_FAILURE);
    }

    sem_t* mutex_writer = sem_open("mutex_writer", O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    if(mutex_writer == SEM_FAILED){
        semaph_close_unlink(0, NULL, NULL, NULL, mutex_count, NULL, NULL, NULL); // Close and unlink already created semophores                
        perror("sem_open(mutex_writer) failed on parent");
        exit(EXIT_FAILURE);
    }

    sem_t* mutex_finished = sem_open("mutex_finished", O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    if(mutex_finished == SEM_FAILED){
        semaph_close_unlink(0, NULL, NULL, mutex_writer, mutex_count, NULL, NULL, NULL); // Close and unlink already created semophores           
        perror("sem_open(mutex_finished) failed on parent");
        exit(EXIT_FAILURE);
    } 
    
    sem_t* mutex_diff = sem_open("mutex_diff", O_CREAT | O_EXCL, SEM_PERMS, 0);
    if(mutex_diff == SEM_FAILED){
        semaph_close_unlink(0, NULL, NULL, mutex_writer, mutex_count, mutex_finished, NULL, NULL); // Close and unlink already created semophores           
        perror("sem_open(mutex_diff) failed on parent");
        exit(EXIT_FAILURE);
    }

    sem_t* mutex_same = sem_open("mutex_same", O_CREAT | O_EXCL, SEM_PERMS, 0);
    if(mutex_same == SEM_FAILED){
        semaph_close_unlink(0, NULL, NULL, mutex_writer, mutex_count, mutex_finished, mutex_diff, NULL); // Close and unlink already created semophores           
        perror("sem_open(mutex_same) failed on parent");
        exit(EXIT_FAILURE);
    }

    sem_t** semaph = malloc((segments_amount+1)* sizeof(sem_t*));
    
    char** sem_names = malloc((segments_amount+1)* sizeof(char*));
    for (int i = 1; i <=  segments_amount; i++){
        sem_names[i] = malloc(15* sizeof(char));    // Keeps the name of semophore
        sprintf(sem_names[i], "semaph%d", i);
        char s[10] = "semaph";
        char c[10];
        sprintf(c, "%d", i);
        strcat(s, c);
        
        semaph[i] = sem_open(s, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);    // Create semophore
        

        if(semaph[i] == SEM_FAILED){        // Close and unlink already created semophores and deallocate memory 
            free(sem_names[i]);
            semaph_close_unlink(i-1, sem_names, semaph, mutex_writer, mutex_count, mutex_finished, mutex_diff, mutex_same);
            perror("sem_open(semaph) failed");
            exit(EXIT_FAILURE);
        }   
    }


    int shmid;
    sharedMemory shared_memory;

    // Create memory segment
    if((shmid = shmget(IPC_PRIVATE, sizeof(sharedMemory), (S_IRUSR|S_IWUSR))) == -1){
        semaph_close_unlink(segments_amount, sem_names, semaph, mutex_writer, mutex_count, mutex_finished,mutex_diff, mutex_same);  // close and unlink already created semophores and deallocate memory 
        perror("Failed to create shared memory segment");
        return 1;
    }

    // Attach memory segment
    if((shared_memory = (sharedMemory)shmat(shmid, NULL, 0)) == (void*)-1){
        semaph_close_unlink(segments_amount, sem_names, semaph, mutex_writer, mutex_count, mutex_finished, mutex_diff, mutex_same);  // close and unlink already created semophores and deallocate memory 
        perror("Failed to attach memory segment");
        return 1;
    }

  
    // Initialize fields of shared memory
    if(sem_wait(mutex_count) < 0){            
        perror("sem_wait failed on parent, mutex_count");
        exit(EXIT_FAILURE);
    }
    shared_memory->count = 0;
    if(sem_post(mutex_count) < 0){
        perror("sem_post failed on parent, mutex_count");
        exit(EXIT_FAILURE);
    }
     

    if(sem_wait(mutex_writer) < 0){            
        perror("sem_wait failed on parent, mutex_count");
        exit(EXIT_FAILURE);
    }
    shared_memory->segment_num = 0;
    shared_memory->wanted_segment_num = 0;
    if(sem_post(mutex_writer) < 0){
        perror("sem_post failed on parent, mutex_writer");
        exit(EXIT_FAILURE);
    }
    

    if(sem_wait(mutex_finished) < 0){            
        perror("sem_wait failed on parent, mutex_finished");
        exit(EXIT_FAILURE);
    }
    shared_memory->finished = 0;
    if(sem_post(mutex_finished) < 0){
        perror("sem_post failed on parent, mutex_finished");
        exit(EXIT_FAILURE);
    }


    // Children array
    int K = atoi(argv[3]);  // Amount of kids
    int N = atoi(argv[4]);  // Amount of requests
    pid_t pids[K];

    // Initialize K kids

    // Does not allow to any child write to shared memory until all child are created
    if(sem_wait(mutex_writer) < 0){
        perror("sem_wait failed on child, mutex_readers");
        exit(EXIT_FAILURE);
    }
    
    FILE *writefile;
	char filenames[15];

    for(int i = 0; i < K; i++){
        if((pids[i] = fork()) < 0){ // Fork new process
            semaph_close_unlink(segments_amount, sem_names, semaph, mutex_writer, mutex_count, mutex_finished, mutex_diff, mutex_same);  // Close and unlink already created semophores and deallocate memory 
            perror("Failed to create process");
            return 1;
        }
        if(pids[i] == 0){          // If it is child process
            sprintf(filenames, "file_%d", i);       // Record file
	        writefile = fopen(filenames, "w");
            
            child(writefile, segm, sem_names, N, K, segments_amount, katatmhsh, shared_memory, semaph, mutex_writer, mutex_count, mutex_finished, mutex_diff, mutex_same);
            exit(0);
        }
    }

    // All child have been created
    if(sem_post(mutex_writer) < 0){
        perror("sem_post failed on parent");
        exit(EXIT_FAILURE);
    } 

    // Parent procedure main part
    
    // Services
    writefile = fopen("file_main", "w");        // Record file
    struct timeval t1, t2; 

    while (shared_memory->finished < K){
          
        if(sem_wait(mutex_diff) < 0){
            perror("sem_wait failed on child, mutex_diff");
            exit(EXIT_FAILURE);
        }          
        
        if(shared_memory->segment_num != 0){
            gettimeofday(&t2, NULL);
            fprintf(writefile,"segment %d entered at %ld sec %ld usec and left at %ld sec %ld usec \n", shared_memory->segment_num, t1.tv_sec, t1.tv_usec, t2.tv_sec, t2.tv_usec); 

        }
                
        shared_memory->segment = segm[shared_memory->wanted_segment_num];
        gettimeofday(&t1, NULL); 
        shared_memory->segment_num = shared_memory->wanted_segment_num;
           
        if(sem_post(mutex_same) < 0){
            perror("sem_post failed on parent");
            exit(EXIT_FAILURE);
        }
    }

        
    fclose(writefile);
    int status;

    // Collect children that have finished
    for(int i = 0; i < K; i++){
        wait(&status);
    }


    // Detach shared memory
    if(shmdt((void*)shared_memory) == -1){
       perror("Failed to destroy shared memory segment");
       return 1;
    }
    
    // Free segments
    for (int i = 1; i <= segments_amount; i++){
        for(int j = 1; j <= katatmhsh; j++)
            free(segm[i][j]);
        free(segm[i]); 
    }
    free(segm);   

    // Close and unlink semaphores
    semaph_close_unlink(segments_amount, sem_names, semaph, mutex_writer, mutex_count, mutex_finished, mutex_diff, mutex_same);
    return 0;
}