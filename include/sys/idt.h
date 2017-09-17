#ifndef _IDT_H
#define _IDT_H

#include <sys/defs.h>

void init_idt();
void register_irq(unsigned char num, uint64_t fp);
void register_all_irqs();

#define TIMER_IRQ     0
#define KEYBOARD_IRQ  1

void _isr_timer();
void isr_timer_init();

void _isr_keyboard();
#endif
