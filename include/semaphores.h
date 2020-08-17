union semun{
	int val;
	struct semid_ds *buff;
	unsigned short *array;
};

void sem_initialize(int semid, int val);
void up(int semid);
void down(int semid);
void sem_remove(int semid);