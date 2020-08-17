#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/signal.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "semaphores.h"
#include "shared_memory.h"

long get_current_time(){
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return spec.tv_nsec;
}

int main(int argc, const char *argv[]){
    //Arguments Handling
    if(argc < 3){
        printf("Give <values> <consumers>\n");
        exit(EXIT_FAILURE);
    }
    if(atoi(argv[1]) < 3000){
        printf("Number of values must be > 3000\n");
        exit(EXIT_FAILURE);
    }

    int num_values = atoi(argv[1]); //get M
    int num_consumers = atoi(argv[2]); //get n
    int i, j, state;
    double avg_time = 0;
    pid_t pid;

    srand(time(NULL));

    //Create and Attach Shared Memory
    create_shared_memory();

    //Counting semaphores
    int *semaphores = calloc(num_consumers + 3, sizeof(int));

    for(i = 0; i < num_consumers + 3; i++){
        if ((semaphores[i] = semget(SEMKEY + i, 1, IPC_CREAT | PERMS)) < 0){
            printf("ERROR in semget(): %s\n\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    //Initialize semaphores
    sem_initialize(semaphores[0], num_consumers); //semaphore for counting: start with n
    sem_initialize(semaphores[1], 1); //empty: shared memory is empty
    sem_initialize(semaphores[2], 0); //full: shared memory is not full
    for(j = 3; j < num_consumers + 3; j++) //initialize consumers
        sem_initialize(semaphores[j], 0);

    for(i = 0; i < num_consumers; i++){
        printf("fork\n");
        if((pid = fork())< 0){
            printf("ERROR in fork(): %s\n\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        else if(!pid) break;
    }

    int *values = calloc(num_values, sizeof(int)); //array of values

    if(pid){ //parent - feeder
        printf("Feeder!\n");
        for(int k = 0; k < num_values; k++){
            down(semaphores[1]); //the value of “empty” is reduced by 1 because one slot will be filled now

            values[k] = rand() % 100; //fill the feeder's array
            
            shmem -> value = values[k]; //value in shared memory
            shmem -> time_stamp = get_current_time();  //time stamp is current time
            //wake up all consumers
            for(j = 3; j < num_consumers + 3; j++)
                up(semaphores[j]);

            up(semaphores[2]); //feeder has placed the item and thus the value of “full” is increased by 1
        }
    }
    else{ //child
        int sem_value;
        FILE* fp = fopen("output", "w");

        int *read_values= calloc(num_values, sizeof(int));

        printf("Process child %d with semaphore position = %d\n", getpid(), i);

        for(j = 0; j < num_values; j++){
            down(semaphores[i+3]);
            down(semaphores[2]); //As the consumer is removing an item from buffer, the value of “full” is reduced by 1

            read_values[j] = shmem -> value; //consumer keeps the value

            avg_time += get_current_time() - (shmem -> time_stamp);
            avg_time /= 2;

            down(semaphores[0]); //semaphore counter -1

            union semun arg;
            sem_value = semctl(semaphores[0], 0, GETVAL, arg);

            if(sem_value == 0) { //if all consumers have finished
                sem_initialize(semaphores[0], num_consumers); //reset counter
                up(semaphores[1]); //all consumers has consumed the item, thus increasing the value of “empty” by 1
            }
            else up(semaphores[2]); //unblock next consumer
        }

        if(sem_value == 0){ //final integer has been written
            printf("Average delay time: %.15lf ns\n", avg_time);

            fprintf(fp, "Consumer with PID %d has finished\n", getpid());
            for(int m = 0; m < num_values; m++)
                fprintf(fp, "%d\n", read_values[m]);
            fprintf(fp,"Average delay time: %.15lf ns\n", avg_time);
        }
        fclose(fp);
        free(read_values);
        exit(EXIT_SUCCESS);
    }

    while((wait(&state)) > 0);

    free(values);
    for(j = 0; j < num_consumers + 3; j++)
        sem_remove(semaphores[j]);

    //Detach and Remove Shared Memory
    if(shmdt(shmem) < 0)
        printf("ERROR in shmdt(): %s\n\n", strerror(errno));
    remove_shared_memory(SHMID);

    return 0;
}