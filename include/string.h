#include <sys/defs.h>

#ifndef _STRING_H
#define _STRING_H

size_t strlen(const char* str1);
size_t strcmp(const char* str1, const char* str2);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
char *strcat(char *dest, const char *src);
#endif
