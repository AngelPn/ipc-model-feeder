#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "../include/semaphores.h"

void sem_initialize(int semid, int val){
    union semun arg;
    arg.val = val;
    if (semctl(semid, 0, SETVAL, arg) < 0){
        printf("ERROR in sem_initialize(): %s\n\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void up(int semid){
    struct sembuf sops = {.sem_num = 0, .sem_op = 1, .sem_flg = 0};
    if((semop(semid, &sops, 1)) < 0){
        printf("ERROR in semop(), on V() signal: %s\n\n", strerror(errno));
        exit(getpid());
    }
}

void down(int semid){
    struct sembuf sops = {.sem_num = 0, .sem_op = -1, .sem_flg = 0};
    if((semop(semid, &sops, 1)) < 0){
        printf("ERROR in semop(), on P() signal: %s\n\n", strerror(errno));
        exit(getpid());
    }
}

void sem_remove(int semid){
    if((semctl(semid, 0, IPC_RMID, 0)) < 0)
        printf("ERROR in semctl(): %s\n\n", strerror(errno));
}