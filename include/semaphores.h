union semun{
	int val;
	struct semid_ds *buff;
	unsigned short *array;
};

void sem_initialize(int semid, int val); //Initialize semaphore with value val
void up(int semid); //Increase by 1 the semaphore with id semid
void down(int semid); //Decrease by 1 the semaphore with id semid
void sem_remove(int semid); //Delete semaphore with id semid