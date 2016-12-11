#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

char *getLastLine(char *filename, int shmkey) {
  int f = open(filename, O_RDONLY, 0);

	if (f < 0) {
		printf("Unable to access file %s: %s\n", filename, strerror(errno));
		return NULL;
	}
	
  int shmid = shmget(shmkey, 0, 0);

	if (shmid < 0) {
		printf("Unable to access shared memory: %s\n", strerror(errno));
	}
	
  int *size_ptr = shmat(shmid, 0, 0);
  int size = *size_ptr;
	char *buf = calloc(size + 1,1);

	if (size) {
		lseek(f, -1 * size, SEEK_END);
		read(f, buf, size);
		buf[strcspn(buf, "\n")] = 0;
	}

	return buf;
}

void getUserLine(char *filename, int shmkey, int semkey) {
  //get sem and shm
  int shmid = shmget(shmkey, 1, 0);
  int semid = semget(semkey, 1, 0);

	if (shmid < 0) {
		printf("Unable to access shared memory: %s\n", strerror(errno));
		return;
	}

	if (semid < 0) {
		printf("Unable to access semaphore: %s\n", strerror(errno));
		return;
	}
	
  //take semaphore
  struct sembuf sb;
  sb.sem_num = 0;
  sb.sem_flg = SEM_UNDO;
  sb.sem_op = -1;
  semop(semid, &sb, 1);
  
	char *lastLine = getLastLine(filename, shmkey);

	if (lastLine && *lastLine) {
		printf("Last line: %s\n", lastLine);
		free(lastLine);
		printf("Your turn: ");
	} else {
		printf("You start: ");
	}

  char newLine[10000];
  fgets(newLine, sizeof(newLine), stdin);

  //write new length to shm
  int *size_ptr = (int *) shmat(shmid, 0, 0);
  int new_size = *size_ptr;
  new_size = strlen(newLine);
  *size_ptr = strlen(newLine);
  shmdt(size_ptr);

  //write to file
  int fd = open(filename, O_WRONLY | O_APPEND, 0);

	if (fd < 0) {
		printf("Unable to access file %s: %s\n", filename, strerror(errno));
	} else {
		write(fd, newLine, new_size);
		close(fd);
	}

  //give back sem
  sb.sem_op = 1;
  semop(semid, &sb, 1);
}

int main() {
  char *filename = "story.txt";
  int shmkey = ftok("resources.c", 7);
  int semkey = ftok("resources.c", 8);
  getUserLine(filename, shmkey, semkey);
  return 0;
}
