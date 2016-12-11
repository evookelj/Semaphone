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
  int shmid = shmget(shmkey, 0, 0);
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
  
  //take semaphore
  struct sembuf sb;
  sb.sem_num = 0;
  sb.sem_flg = SEM_UNDO;
  sb.sem_op = -1;
  semop(semid, &sb, 1);
  
  char newLine[10000];
  printf("Your turn: ");
  fgets(newLine, sizeof(newLine), stdin);

  //write new length to shm
  int *size_ptr = (int *) shmat(shmid, 0, 0);
  int new_size = *size_ptr;
  new_size = strlen(newLine);
  *size_ptr = strlen(newLine);
  shmdt(size_ptr);

  //write to file
  int fd = open(filename, O_WRONLY | O_APPEND, 0);
  write(fd, newLine, new_size);

  //give back sem
  sb.sem_op = 1;
  semop(semid, &sb, 1);

  close(fd);
}

int main() {
  char *filename = "story.txt";
  int shm_key = ftok("resources.c", 7);
  int sem_key = ftok("resources.c", 8);
	char *ll = getLastLine(filename, shm_key);
	printf("last line: %s\n", ll);
	free(ll);
  getUserLine(filename, shm_key, sem_key);
  
  return 0;
}
