#include <unistd.h>
#include <stdio.h>

int fputs(const char *s, FILE * fp)
{
    stdin->fd = 0;
    stdout->fd = 1;
    stderr->fd = 2;
    const char *c = s;
    int len = 0;

    while (*c)
    {
       len++;
       c++;
    }

    return write(fp->fd, s, len);
}
