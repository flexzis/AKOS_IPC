#include <stdio.h>
#include <stdlib.h>

int ntypes = 0;
int* dirty_dishes;
int* wash_durations;
int* wipe_durations;

void error(char* msg)
{
    perror(msg);
    exit(-1);
}

void print_arr(int* arr, int sz)
{
  for(int i = 0; i < sz; ++i)
    printf("%d ", arr[i]);
  printf("\n");
}

void read_data(char* fwash, char* fwipe, char* fdirty)
{
  FILE* fd = fopen(fwash, "r");
  if (!fd) error("(init) can't open file");

  int type, num;
  char c;
  for (;fscanf(fd, "%d %c %d", &type, &c, &num) != EOF; ++ntypes);

  wash_durations = (int*)malloc(ntypes * sizeof(int));
  wipe_durations = (int*)malloc(ntypes * sizeof(int));
  dirty_dishes   = (int*)malloc(ntypes * sizeof(int));

  fseek(fd, 0, SEEK_SET);
  for (;fscanf(fd, "%d %c %d", &type, &c, &num) != EOF;)
    wash_durations[type] = num;

  fclose(fd);
  fd = fopen(fwipe, "r");
  if (!fd) error("(init) can't open file");

  for (;fscanf(fd, "%d %c %d", &type, &c, &num) != EOF;)
    wipe_durations[type] = num;

  fclose(fd);
  fd = fopen(fdirty, "r");
  if (!fd) error("(init) can't open file");

  for (int i = 0; i < ntypes; ++i)
    dirty_dishes[i] = 0;

  for (;fscanf(fd, "%d %c %d", &type, &c, &num) != EOF;)
    dirty_dishes[type] += num;

  fclose(fd);
}

void free_data()
{
  free(wash_durations);
  free(wipe_durations);
  free(dirty_dishes);
}
