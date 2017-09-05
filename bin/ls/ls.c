#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int main()
{
  int fd =  open("/home/dbaddam/workdir/bin/ls/dinesh",O_RDONLY);

  char buf[1024];
  int x = read(fd, buf, 1024);

  printf("%d\n",x);
  puts(buf);
   close(fd);
   return 0;
}


