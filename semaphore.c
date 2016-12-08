#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};

int createSemaphore(int key, int value) {
	int sd = semget(key, 1, IPC_CREAT | IPC_EXCL | 0644);

	if (sd < 0) {
		printf("Error creating semaphore\n");
	} else {
		union semun semdata;
		semdata.val = value;
		semctl(sd, 0, SETVAL, semdata);
	}
	
	return sd;
}

int getSemaphore(int key) {
	return semget(key, 1, 0);
}

int getSemaphoreValue(int sd) {
	return semctl(sd, 0, GETVAL);
}

void removeSemaphore(int sd) {
	semctl(sd, 0, IPC_RMID);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Incorrect number of arguments!\n");
		return -1;
	}

	int key = ftok("semaphore.c", 7);

	if (strcmp(argv[1], "create") == 0) {
		int value = 7;
		int sd = createSemaphore(key, value);
		printf("Created semaphore %d with value %d\n", sd, value);
	} else if (strcmp(argv[1], "get") == 0) {
		int sd = getSemaphore(key);
		printf("Semaphore %d has value %d\n", sd, getSemaphoreValue(sd));
	} else if (strcmp(argv[1], "remove") == 0) {
		int sd = getSemaphore(key);
		removeSemaphore(sd);
		printf("Removed semaphore %d\n", sd);
	}
	
	return 0;
}
