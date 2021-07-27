#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include "../initialization.h"

void wipe(int* dishes_on_table, int TL)
{
  for(int type = 0; type < ntypes; ++type)
  {
    for (int dishes = dirty_dishes[type]; dishes >= 0;)
    {
      while(dishes_on_table[0] == 0);
      sleep(wipe_durations[type]);
      printf("Wiped dish of type %d\n", type);
      --dishes_on_table[0];
      --dishes;
    }
  }
  printf("Washing completed.\n");
}

int main(int argc, char* argv[])
{
  read_data("../wash_durations", "../wipe_durations", "../dirty_dishes");

  int shmid = atoi(argv[1]);
  int TL = atoi(argv[2]);

  int* dishes_on_table = (int*)shmat(shmid, NULL, 0);
  if (dishes_on_table < 0)
    error("(wiper) can't find shared memory");

  dishes_on_table[0] = 0;

  wipe(dishes_on_table, TL);
  shmdt(dishes_on_table);
  return 0;
}
