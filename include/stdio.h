#ifndef _STDIO_H
#define _STDIO_H

static const int EOF = -1;

struct FILE
{
   int fd;
   int flags;
#define VALID_FD_FILE 1
};

#define MAX_FILES 1000
typedef struct FILE FILE;
extern FILE files[MAX_FILES];

#define stdin   (files+0)
#define stdout  (files+1)
#define stderr  (files+2)

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR   2

int putchar(int c);
int puts(const char *s);
int printf(const char *format, ...);

char *gets(char *s);

FILE *fopen(const char *path, const char *mode);
int fputs(const char *s, FILE* fp);
int fgets(char *s, int size, FILE* fp);
int fclose(FILE *fp);

#endif
