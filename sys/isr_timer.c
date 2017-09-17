#include <sys/idt.h>
#include <sys/io.h>
#include <sys/pic.h>
#include <sys/kprintf.h>


uint64_t secs = 0;
uint64_t subsec = 0;

void isr_timer()
{

   subsec++;
   if (subsec == 10000)
   {
      char *p;
      char *temp2;
      char* s = "Time since boot:";
      char str[20] ;
      secs++;
      subsec = 0;
      longTOhexa(secs, str, 10);

      kprintf("Elasped sec - %d\n", secs);

      for (temp2 = (char*)0xb8000+150*25, p = s; *p && temp2 < (char*)0xb8000+25; temp2 += 2, p++)
        *temp2 = *p;
      for (p = str; *p && temp2 < (char*)0xb8000+25; temp2 += 2, p++)
        *temp2 = *p;

      
   }
   //kprintf("Inside ISR_TIMER BORING\n");
   picack(TIMER_IRQ); 
}

void isr_timer_init()
{
   outb(0x43, 0b00010100);
   outb(0x40, 119);
}
