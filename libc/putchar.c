#include <stdio.h>
#include <syscall.h>
#include <unistd.h>


int putchar(int c)
{
  //char* p = &((char)c);
  syscall3(write, 1, &c, 1);   
  return (sysret == 1) ? c:0;
}
