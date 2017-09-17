#ifndef _PIC_H
#define _PIC_H

#include <sys/defs.h>

void picremap(int moffset, int soffset);
void picack(unsigned char irq);

#endif
