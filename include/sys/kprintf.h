#ifndef __KPRINTF_H
#define __KPRINTF_H

#include <sys/mem.h>
#include <sys/task.h>
#define VIDEO_MEM_BASE (KERNEL_BASE + 0xB8000)

void kprintf(const char *fmt, ...);
void longTOhexa( unsigned long number, char *p, int base);
int stringlen(const char *a);
void strrev(char *a);
void intTOstring(int number, char *p, int base);
void unsignedLongTOstring(unsigned long number, char *p, int base);


/* TODOKISHAN - We should exit() instead of a while loop here */
#define ERROR(...) \
do{ \
kprintf("ERROR : " __VA_ARGS__); \
exit(-1); \
}while(0);
#endif
