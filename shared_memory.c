#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#include "shared_memory.h"

void create_shared_memory(){
    if((SHMID = shmget(SHMKEY, sizeof(shared_data), PERMS | IPC_CREAT)) == -1){
        printf("ERROR in shmget(): %s\n\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if((shmem = (shared_data *)shmat(SHMID, 0, 0)) == (shared_data *)-1){
        printf("ERROR in shmat(): %s\n\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void remove_shared_memory(int shmid){
    struct shmid_ds dummy;
    if((shmctl(shmid, IPC_RMID, &dummy)) < 0)
        printf("ERROR in shmctl(): %s\n\n", strerror(errno));
}