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

shared_data *shmem = NULL; //shmem is pointer to struct shared_data

int SEMID=0;
int SHMID=0;

void down(int semid){
    struct sembuf sops = {.sem_num = 0, .sem_op = -1, .sem_flg = 0};
    if((semop(semid, &sops, 1)) < 0){
        printf("ERROR in semop(), on P() signal: %s\n\n", strerror(errno));
        exit(getpid());
    }
}

void up(int semid){
    struct sembuf sops = {.sem_num = 0, .sem_op = 1, .sem_flg = 0};
    if((semop(semid, &sops, 1)) < 0){
        printf("ERROR in semop(), on V() signal: %s\n\n", strerror(errno));
        exit(getpid());
    }
}

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

long get_current_time_with_ms (void){
    long   ms;
    time_t s;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds

    return ms;
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

    int i, status;
    double avg_time = 0;

    pid_t pid;

    srand(time(NULL));

    int num_values = atoi(argv[1]); //M
    int num_consumers = atoi(argv[2]); //n

    //Create and Attach Shared Memory
    create_shared_memory(); //create shared memory and attach it to a global variable

    int *values = calloc(num_values, sizeof(int)); //array of values

    int *semaphores = calloc(num_consumers + 3, sizeof(int)); //array of semaphores

    for(int j = 0; j < num_consumers + 3; j++){
        if ((semaphores[i] = semget(SEMKEY + j, 1, IPC_CREAT | PERMS)) < 0){
            printf("ERROR in semget(): %s\n\n", strerror(errno));
            exit(1);
        }
    }

    sem_init(semaphores[0], num_consumers); //semaphore used for counting: start with n
    sem_init(semaphores[1], 1); //empty
    sem_init(semaphores[2], 0); //full
    for(int j = 3; j < num_consumers + 3; j++) //initialize consumers
        sem_init(semaphores[j], 0);

    for(int j = 0; j < num_consumers; j++){
        printf("fork...\n");
        if((pid = fork())< 0){
            printf("ERROR in fork(): %s\n\n", strerror(errno));
            exit(1);
        }
        else if(!pid) break;
    }

    if(pid){ //parent - feeder
        printf("Feeder!\n");
        for(int k = 0; k < num_values; k++){
            down(semaphores[1]); //feeder will start writing

            values[k] = rand() % 100; //fill the feeder's array with values < 100

            shmem-> value = values[k];
            shmem -> time_stamp = get_current_time_with_ms();  //get current time in ms

            for(int j = 3; j < num_consumers + 3; j++) up(semaphores[j]);//wake up all consumers

            up(semaphores[2]); //memory is full
        }
    }
    else{ //child
        int semValue;
        FILE* fp = fopen("output", "w");

        int *read_values= calloc(num_values, sizeof(int));

        printf("I'm the process child %d sem_pos = %d\n", getpid(), i);

        for(int j = 0; j < num_values; j++){
            down(semaphores[i+3]);

            down(semaphores[2]); //other consumers need to wait

            read_values[j] = shmem -> value; //consumer keeps the value

            avg_time += get_current_time_with_ms() - shmem -> time_stamp;
            avg_time /= 2;

            down(semaphores[0]); //semaphore counter -1


            union semun arg;
            semValue = semctl(semaphores[0], 0, GETVAL, arg);

            if(!semValue) { //if all consumers have done their work
                sem_init(semaphores[0], num_consumers); //reset counter
                up(semaphores[1]); //unblock feeder
            }
            else up(semaphores[2]); //unblock next consumer
        }

        if(!semValue){ //final int has been written
            printf("Average delay time:\n");
            printf("%.15lf ms\n", avg_time);

            fprintf(fp, "Consumer with PID %d has finished running\n", getpid());
            fprintf(fp, "Output:\n");
            for(int m = 0; m < num_values; m++)
                fprintf(fp, "%d\n", read_values[m]);
            fprintf(fp,"Average delay time:\n");
            fprintf(fp,"%.15lf ms\n", avg_time);
        }

        fclose(fp);
        free(read_values);

        exit(0);
    }

    //Detach and Remove Shared Memory
    if(shmdt(shmem) < 0)
        printf("ERROR in shmdt(): %s\n\n", strerror(errno));
    remove_shared_memory(SHMID);

    return 0;
}