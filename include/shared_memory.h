#define MAX_LINE_SIZE 65536 // 64KBytes
#define LINES 10

struct temp_shared_memory{
    char segment[LINES+1][MAX_LINE_SIZE+1];
    int k;
    int sample[100];
};

typedef struct temp_shared_memory* tempSharedMemory;

struct shared_memory{
    int file_num;
    int start_line;
    int end_line;
    int temp_mem_used;
    //int wanted_segment_num;
    char** temp_mem;   // Pointer to desired temporary shared memory
    int temp_shared_mem_key;
    int finished;
};

typedef struct shared_memory* sharedMemory;



