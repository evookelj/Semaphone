#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

char* getLastLine(char* filename, int shmkey) {
  int f = open(filename, O_RDONLY, 0);
  int shmid = shmget(shmkey, 0, 0);
  int size;
  shmat(shmid, &size, SHM_RDONLY);
  char* buf = calloc(size+1,1);
  lseek(f, -1*size, SEEK_END);
  int r = read(f, buf, size);
  printf("last line: %s\n", buf);
  return buf;
}

void getUserLine(char* filename, int shmkey, int semkey) {
  char* line;
  scanf("What line would you like to add?\n%s", line);
  int shmid = shmget(shmkey,0,0);
  int semid = semget(semkey,0,0);

  struct sembuf sb;
  sb.sem_num = 0;
  sb.sem_flg = SEM_UNDO;
  sb.sem_op = -1;

  semop(semid, &sb, 1);

  int newSize;
  void* att = shmat(shmid, &newSize, 0);
  printf("result shmat: %d\n", att);
  newSize = strlen(line);
  shmdt(&newSize);
  printf("after shmdt\n");

  int fd = open(filename, O_WRONLY | O_APPEND, 0);
  write(fd, line, newSize);
  
  sb.sem_op = 1;
  semop(semid, &sb, 1);

  close(fd);
  
}

int main() {
  char *filename = "story.txt";
  int shm_key = ftok("resources.c", 7);
  int sem_key = ftok("resources.c", 8);
  char* ll = getLastLine(filename, shm_key);
  getUserLine(filename, shm_key, sem_key);
  
  return 0;
}
