#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/signal.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>


#define PERMS 0666
#define SHMKEY (key_t)1234
#define SEMKEY (key_t)5678

union semun{
	int val;
	struct semid_ds *buff;
	unsigned short *array;
};

typedef struct shared_data{
	int value;
	long time_stamp;
} shared_data;

shared_data *shmem = NULL; //shmem is pointer to struct shared_date

int SEMID=0;
int SHMID=0;

void create_shared_memory(){
    if((SHMID = shmget(SHMKEY, sizeof(shared_data), PERMS | IPC_CREAT)) == -1){
        printf("ERROR in shmget(): %s\n\n", strerror(errno));
        exit(1);
    }
    //int *shar_mem = NULL;
    if((shmem = (shared_data *)shmat(SHMID, 0, 0)) == (shared_data *)-1){
        printf("ERROR in shmat(): %s\n\n", strerror(errno));
        exit(1);
    }
}

void remove_shared_memory(int shmid){
    struct shmid_ds dummy;

    if((shmctl(shmid, IPC_RMID, &dummy)) < 0)
        printf("ERROR in shmctl(): %s\n\n", strerror(errno));
}

int main(int argc, const char *argv[]){
    if(argc < 3){
        printf("give <values> <consumers>\n");
        exit(1);
    }
    if(atoi(argv[1]) < 3000){
        printf("number of values must be > 3000\n");
        exit(1);
    }

    int i, j, m, status;
    double avg_time = 0;

    pid_t pid;

    srand(time(NULL));

    int num_values = atoi(argv[1]); //M
    int num_consumers = atoi(argv[2]); //n

    //Create and Attach Shared Memory
    create_shared_memory(); //create shared memory and attach it to a global variable

    //Detach and Remove Shared Memory
    if(shmdt(shmem) < 0)
        printf("ERROR in shmdt(): %s\n\n", strerror(errno));
    remove_shared_memory(SHMID);

    return 0;
}