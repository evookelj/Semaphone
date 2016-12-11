#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <errno.h>

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

char *readFile(char *filename) {
	struct stat fs;

	if (stat(filename, &fs) < 0) {
		printf("Stat error\n");
		return NULL;
	}

	int file = open(filename, O_RDONLY);
	int size = fs.st_size;
	char *s = (char *) calloc(sizeof(char), size + 1);
	read(file, s, size);
	close(file);
	return s;
}	

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Incorrect number of arguments!\n");
		return -1;
	}

	char *filename = "story.txt";
	int shm_key = ftok("resources.c", 7);
	int sem_key = ftok("resources.c", 8);
	int sem_value = 1;

	if (strcmp(argv[1], "-c") == 0) {
		umask(0000);
		int file = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);

		if (file < 0) {
			printf("Unable to create file %s: %s\n", filename, strerror(errno));
			return 1;
		} else {
			printf("Successfully created %s\n", filename);		
			close(file);
		}

		int shm = shmget(shm_key, sizeof(int), IPC_CREAT | IPC_EXCL | 0644);

		if (shm < 0) {
			printf("Unable to allocate shared memory: %s\n", strerror(errno));
			return 1;
		} else {
			printf("Successfully created shared memory\n");
		}

		int sem = createSemaphore(sem_key, sem_value);

		if (sem < 0) {
			printf("Unable to create semaphore: %s\n", strerror(errno));
			return 1;
		} else {
			printf("Successfully created semaphore\n");
		}
	} else if (strcmp(argv[1], "-v") == 0) {
		char *s = readFile(filename);
		printf("%s", s);
		free(s);
	} else if (strcmp(argv[1], "-r") == 0) {
		int shm = shmget(shm_key, 0, 0);

		if (shm < 0) {
			printf("Unable to access shared memory: %s\n", strerror(errno));
		} else {
			shmctl(shm, IPC_RMID, 0);
			printf("Removed shared memory\n");
		}

		int sem = getSemaphore(sem_key);

		if (sem < 0) {
			printf("Unable to access semaphore: %s\n", strerror(errno));
		} else {
			removeSemaphore(sem);
			printf("Removed semaphore\n");
		}

		char *s = readFile(filename);
		printf("%s", s);
		free(s);
	}
	
	return 0;
}
