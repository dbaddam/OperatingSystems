#include <unistd.h>
#include <stdio.h>

int fgets(char *s, int size, FILE * fp)
{
    return read(fp->fd, s, size);
}
