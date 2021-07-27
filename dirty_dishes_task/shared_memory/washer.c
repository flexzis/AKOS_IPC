#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include "../initialization.h"

void wash(int* dishes_on_table, int TL)
{
  for(int type = 0; type < ntypes; ++type)
  {
    for (int dishes = dirty_dishes[type]; dishes >= 0;)
    {
      while(dishes_on_table[0] >= TL);
      sleep(wash_durations[type]);
      printf("Washed dish of type %d, %d of this type left\n", type, );
      ++dishes_on_table[0];
      --dishes;
    }
  }
}

int main()
{
  int TL = atoi(getenv("TABLE_LIMIT"));
  read_data("../wash_durations", "../wipe_durations", "../dirty_dishes");
  int shmid;
  if(shmid = shmget(IPC_PRIVATE, TL, IPC_CREAT | 0666) < 0)
    error("can't create shared memory");

  pid_t pid = fork();
  if (pid < 0)
    error("(washer) can't fork");
  if (pid == 0)
  {
    char wiper_fname[] = "wiper.exe";
    int str_sz = 10;
    char str_shmid[str_sz];
    char str_TL[str_sz];
    snprintf(str_shmid, str_sz, "%d", shmid);
    snprintf(str_TL, str_sz, "%d", TL);
    if (execl(wiper_fname, wiper_fname, str_shmid, str_TL, NULL) < 0)
      error("can't exec");
  }
  else
  {
    int* dishes_on_table;
    if (dishes_on_table = (int*)shmat(shmid, NULL, 0) < 0)
      error("(washer) can't find shared memory");
    dishes_on_table[0] = 0;
    wash(dishes_on_table, TL);
    shmdt(dishes_on_table);
  }
  return 0;
}
