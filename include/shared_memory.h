
struct shared_memory{
    int file_num;
    int start_line;
    int end_line;
    int temp_mem_used;
    //int wanted_segment_num;
    char** temp_mem;   // Pointer to desired temporary shared memory
    int finished;
};

typedef struct shared_memory* sharedMemory;
