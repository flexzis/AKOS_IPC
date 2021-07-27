#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include "../initialization.h"


void fprint_beg(FILE* fd, int* a)
{
  fseek(fd, 0, SEEK_SET);
  fprintf(fd, "%d", *a);
}

void fscan_beg(FILE* fd, int* a)
{
  fseek(fd, 0, SEEK_SET);
  fscanf(fd, "%d", a);
}

void change_weight(int semid, struct sembuf* sbuf, FILE* fd)
{
  //wait until other process will stop writing to file
  sbuf->sem_op = 0;
  semop(semid, sbuf, 1);
  //mark that currently file is busy
  sbuf->sem_op = 1;
  semop(semid, sbuf, 1);
  int cur_weight;
  fscan_beg(fd, &cur_weight);
//  fprintf(stderr, "%s\n", );
  --cur_weight;
  fprint_beg(fd, &cur_weight);
  //mark end of writing
  sbuf->sem_op = -1;
  semop(semid, sbuf, 1);
}

void wipe()
{
  key_t key = ftok("communicator", 0);
  if (key < 0) error("(wash) can't generate key");

  struct sembuf sbuf;
  int semid = semget(key, 1, 0666|IPC_CREAT);
  if (semid < 0) error("(wash) can't get semid");

  sbuf.sem_num = 0;
  sbuf.sem_flg = 0;
  FILE* fd = fopen("communicator", "w+");
  if (!fd) error("(wash) can't open communicator file");

  int cur_weight = 0;
  fprint_beg(fd, &cur_weight);

  for (int type = 0; type < ntypes; ++type)
  {
    for(int dishes = dirty_dishes[type]; dishes >= 0; --dishes)
    {
      fscan_beg(fd, &cur_weight);
      while(cur_weight <= 0)
        fscan_beg(fd, &cur_weight);
      sleep(wipe_durations[type]);
      change_weight(semid, &sbuf, fd);
      printf("Dish wiped. %d more of type %d left.\n", dishes, type);
    }
  }

}

int main()
{
  read_data("../wash_durations", "../wipe_durations", "../dirty_dishes");
  wipe();
  free_data();
}
