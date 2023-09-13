#define MAX_LINE_SIZE 65536 // 64KBytes
#define LINES 10

struct temp_shared_memory{
    char segment[LINES+1][MAX_LINE_SIZE+4];
    int k;
    int sample[100];
};

typedef struct temp_shared_memory* tempSharedMemory;

struct shared_memory{
    int file_num;
    int start_line;
    int end_line;
    int temp_mem_used;
    void* mutex_s;
    int temp_shared_mem_key;
    int finished;
};

typedef struct shared_memory* sharedMemory;



