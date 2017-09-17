#ifndef __KPRINTF_H
#define __KPRINTF_H

void kprintf(const char *fmt, ...);
void longTOhexa( unsigned long number, char *p, int base);
int stringlen(const char *a);
void strrev(char *a);
void intTOstring(int number, char *p, int base);

#endif
