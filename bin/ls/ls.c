#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#define MAX_BUFFER_SIZE 1024

/*
typedef unsigned long ull;
#define __NR_getdents 78ul
#define syscall3(name, a1, a2, a3) \
ull sysret; \
__asm__ __volatile__ ( \
    "movq %1, %%rax\n\t" \
    "movq %2, %%rdi\n\t" \
    "movq %3, %%rsi\n\t" \
    "movq %4, %%rdx\n\t" \
    "syscall\n\t" \
    "movq %%rax, %0\n\t" \
    : "=r"(sysret) \
    : "r"(__NR_##name), "r" ((ull)a1), "r" ((ull)a2), "r" ((ull)a3) \
    : "rax", "rdi", "rsi", "rdx", "memory" \
                     ); 

int getdents(int fd, struct dirent* dirp, int size)
{
    syscall3(getdents, fd, dirp, size);

    return (int)sysret;
}
*/
int main(int argc, char *argv[], char *envp[])
{
   char buffer[MAX_BUFFER_SIZE] = ".";
   int fd = open((argc > 1) ? argv[1]:buffer, O_RDONLY); 
   struct dirent dirp;
   if(fd < 0)
      puts("Something went wrong!");
   else
   {
      while(getdents(fd, &dirp, MAX_BUFFER_SIZE) > 0)
      {
            fputs(dirp.d_name-1, stdout);
            fputs("  ",stdout);

            //printf("offset = %ld\n", dirp.d_off);
            //printf("offset = %ld\n", dirp.d_off - 1);
            lseek(fd, dirp.d_off, SEEK_SET);
      }
      fputs("\n",stdout);
   }
   close(fd);   
   return 0;
}


