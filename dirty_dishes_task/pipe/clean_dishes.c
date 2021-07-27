#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include "../initialization.h"

void wiper(int write_fd, int read_fd)
{
  for(;;)
  {
    int type;
    read(read_fd, &type, sizeof(int));
    if (type == -1)
      break;
    sleep(wipe_durations[type]);
    write(write_fd, &type, sizeof(int));
    printf("Wiped dish of type %d.\n", type);
  }
}

void washer(int write_fd, int read_fd, int TL)
{
  int cur_weight = 0;
  for (int type = 0; type < ntypes; ++type)
  {
    for(int dishes = dirty_dishes[type]; dishes > 0; --dishes)
    {
      if(cur_weight < TL)
      {
        write(write_fd, &type, sizeof(int));
        ++cur_weight;
        sleep(wash_durations[type]);
        printf("Dish washed. %d more of type %d left.\n", dishes, type);
      }
      else
      {
        int buf[TL];
        cur_weight -= read(read_fd, buf, TL * sizeof(int)) / sizeof(int);
        ++dishes;
      }
    }
  }
  //wait until wiper will wipe all current dishes
  int end_type = -1;
  write(write_fd, &end_type, sizeof(int));
  while(cur_weight > 0)
  {
    int buf[TL];
    cur_weight -= read(read_fd, buf, TL * sizeof(int)) / sizeof(int);
  }
  //say to wiper that all dishes are
  printf("All dishes cleaned!\n");
}

int main()
{
  int TL = atoi(getenv("TABLE_LIMIT"));
  read_data("../wash_durations", "../wipe_durations", "../dirty_dishes");
  //pfd[0] -- read   pfd[1] -- write
  int washer_pfd[2];
  if (pipe(washer_pfd) < 0) error("pipe");
  int wiper_pfd[2];
  if (pipe(wiper_pfd) < 0) error("pipe");

  pid_t pid = fork();
  if (pid < 0)
    error("fork");
  if (pid == 0)
  {
    close(wiper_pfd[0]);
    close(washer_pfd[1]);
    wiper(wiper_pfd[1], washer_pfd[0]);
    close(washer_pfd[0]);
    close(wiper_pfd[1]);
  }
  if (pid > 0)
  {
    close(washer_pfd[0]);
    close(wiper_pfd[1]);
    washer(washer_pfd[1], wiper_pfd[0], TL);
    close(wiper_pfd[0]);
    close(washer_pfd[1]);
  }
  free_data();
}
