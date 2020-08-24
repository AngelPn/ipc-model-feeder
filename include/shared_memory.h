#include <stdlib.h>
#include <string.h>

#define PERMS 0666
#define SHMKEY (key_t)1234
#define SEMKEY (key_t)5678

typedef struct shared_data{
	int value;
	long time_stamp;
} shared_data;

shared_data *shmem; //shmem is pointer to struct shared_data
int SHMID;

void create_shared_memory(); //Create shared memory of size shared_data and attach it to shmem pointer
void remove_shared_memory(int shmid); //Deallocate space allocated for shared memory
void detach_shared_memory(void *addr); //Detach shared memory from a process