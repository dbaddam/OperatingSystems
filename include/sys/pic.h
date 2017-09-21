#ifndef _PIC_H
#define _PIC_H

#include <sys/defs.h>

#define MASTER_PIC_OFFSET 0x20
#define SLAVE_PIC_OFFSET  0x28

void picremap(int moffset, int soffset);
void picack(unsigned char irq);
void pic_irq_set_mask(unsigned char irq_line);
void pic_irq_clear_mask(unsigned char irq_line);

#endif
