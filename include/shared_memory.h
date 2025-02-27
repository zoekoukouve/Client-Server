#define MAX_LINE_SIZE 65536 // 64KBytes
#define LINES 10

struct temp_shared_memory{
    char segment[MAX_LINE_SIZE];
};

typedef struct temp_shared_memory* tempSharedMemory;

struct shared_memory{
    int file_num;
    int start_line;
    int end_line;
    int temp_mem_used;
    int temp_shared_mem_key;
    int finished;
    int sem_id;
};

typedef struct shared_memory* sharedMemory;



