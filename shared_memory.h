
struct shared_memory{
    int count;
    int segment_num;
    int wanted_segment_num;
    char** segment;   // Pointer to desired segment
    int finished;
};

typedef struct shared_memory* sharedMemory;
