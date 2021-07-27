#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <zlib.h>

#define BUFLEN 256

void error(char* errmsg)
{
  perror(errmsg);
  exit(-1);
}

void cp_compress(char* source, char* dest)
{
  FILE* src = fopen(source, "r");
  if(!src)
    error("(cp_compress) can't open source file");

  gzFile dst = gzopen(dest, "w");
  if(!dst)
    error("(cp_compress) can't open destination file");

  int n;
  char buf[BUFLEN];
  for(;;)
  {
    n = fread(buf, 1, sizeof(buf), src);
    if(ferror(src))
      error("(cp_compress) fread");

    if (n == 0)
      break;
    if(gzwrite(dst, buf, (unsigned)n) != n)
      error("(cp_compress) gzwrite");
  }

  fclose(src);
  if(gzclose(dst) != Z_OK)
   error("(cp_compress) can't close gz");
}

char* pathcat(char* dest, char* source)
{
    strcat(dest, "/");
    strcat(dest, source);
    return dest;
}

//searches in given path file "fname"
int contains (char* source, char* fname)
{
  DIR* src = opendir(source);
  if(!src)
    error("(contains) can't open source dir");

  errno = 0;
  struct dirent* kid;
  while(kid = readdir(src))
  {
    if(strcmp(kid->d_name, fname) == 0)
      return 1;
  }
  if (errno != 0)
    error("(contains)");

  return 0;
}

void backup(char* source, char* dest)
{
  printf("%s --> %s\n", source, dest);
  DIR* src = opendir(source);
  if(!src)
    error("(backup) can't open source directory");

  DIR* dst = opendir(dest);
  if (!dst)
    error("(backup) can't open destination directory");

  errno = 0;
  struct dirent* kid;
  while(kid = readdir(src))
  {
    if (strcmp(kid->d_name, ".") == 0 || strcmp(kid->d_name, "..") == 0)
      continue;

    char src_path[BUFLEN];
    strcpy(src_path, source);
    pathcat(src_path, kid->d_name);

    char dst_path[BUFLEN];
    strcpy(dst_path, dest);
    pathcat(dst_path, kid->d_name);

    struct stat statbuf;
    if (stat(src_path, &statbuf) < 0)
      error("(backup) stat1");

    if(contains(dest, src_path))
    {
      struct stat dst_statbuf;

      errno = 0;
      if (stat(dst_path, &dst_statbuf) < 0)
      if (errno )
        error("(backup) stat2");

      //latest version of file was already backuped
      if (statbuf.st_mtime <= dst_statbuf.st_mtime)
        continue;
    }

    if(S_ISDIR(statbuf.st_mode))
    {
      //create new dir in dst with same permissions
      mkdir(dst_path, 00777 & statbuf.st_mode);
      backup(src_path, dst_path);
    }

    if(S_ISREG(statbuf.st_mode))
      cp_compress(src_path, dst_path);
  }
  if (errno != 0) //error occured
    error("(backup)");
}

int main(int argc, char* argv[])
{
    backup(argv[1], argv[2]);
    return 0;
}
