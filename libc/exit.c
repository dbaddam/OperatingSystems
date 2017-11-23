#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

void exit(int status)
{
/*
   __asm__
   (
    "movq $60, %rax\n\t"
    "movq $0, %rdi\n\t"
    "syscall"
   );
*/
/*   __asm__ __volatile__
   (
    "movq %1, %%rax\n\t"
    "movq %2, %%rdi\n\t"
    "syscall"
    : "=r"(status) 
    : "r"((long long)60), "r" ((long long)status)
    : "rax", "rdi", "memory" 
   );
*/

   syscall1(exit, status);
}
