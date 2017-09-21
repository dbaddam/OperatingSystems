#include <sys/pic.h>
#include <sys/io.h>

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)
#define PIC_ACK	        0x20
#define PIC1_MIN_IRQ    0
#define PIC1_MAX_IRQ    7
#define PIC2_MIN_IRQ    8
#define PIC2_MAX_IRQ    15

#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */
 
#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */


void picremap(int moffset, int soffset)
{
   unsigned char m1, m2;  /* masks */

   m1 = inb(PIC1_DATA);
   m2 = inb(PIC2_DATA);

   /* TODO - Should I add I/O waits */
   outb(PIC1_COMMAND, ICW1_INIT + ICW1_ICW4);  /* Telling PIC we want to init */
   outb(PIC2_COMMAND, ICW1_INIT + ICW1_ICW4);
   outb(PIC1_DATA, moffset);             /* Tell the PIC where the offset is */
   outb(PIC2_DATA, soffset);
   outb(PIC1_DATA, 4);        /* Tell master there is a slave */
   outb(PIC2_DATA, 2);        /* Tell slave that it is a slave */

   outb(PIC1_DATA, ICW4_8086);
   outb(PIC2_DATA, ICW4_8086);

   outb(PIC1_DATA, m1);
   outb(PIC2_DATA, m2);
}

void picack(unsigned char irq)
{

   if (irq > PIC2_MAX_IRQ)
      return;
   else if (irq >=  PIC2_MIN_IRQ)
   {
      /* This is slave, send the ACK to both PICs */
      outb(PIC2_COMMAND, PIC_ACK);
      outb(PIC1_COMMAND, PIC_ACK);
   }
   else
   {
      outb(PIC1_COMMAND, PIC_ACK);
   } 
}

void pic_irq_set_mask(unsigned char irq_line)
{
   uint16_t port;
   uint8_t value;
 
   if (irq_line < 8) {
      port = PIC1_DATA;
   } else {
      port = PIC2_DATA;
      irq_line -= 8;
   }
   value = inb(port) | (1 << irq_line);
   outb(port, value);        
}
 
void pic_irq_clear_mask(unsigned char irq_line)
{
   uint16_t port;
   uint8_t value;

   if(irq_line < 8) {
      port = PIC1_DATA;
   } else {
      port = PIC2_DATA;
      irq_line -= 8;
   }
   value = inb(port) & ~(1 << irq_line);
   outb(port, value);        
}
