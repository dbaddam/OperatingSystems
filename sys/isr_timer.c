#include <sys/os.h>
#include <sys/pic.h>
#include <sys/io.h>


uint64_t secs = 0;
uint64_t subsec = 0;

void isr_timer()
{

   //pic_irq_set_mask(TIMER_IRQ | KEYBOARD_IRQ);
   subsec++;
   if (subsec == 18)
   {
      char str[25] ;
      int  len;
      secs++;
      subsec = 0;
      
      // convert the secs in int to string and add 's' to it, Ex: "23s".
      unsignedLongTOstring(secs, str, 10);
      len = stringlen(str);
      str[len] = 's';
      str[len+1] = '\0';

      char *temp2, *temp1;
      temp2 = (char*)(VIDEO_MEM_BASE)+80*45;
      for(temp1 = "Time since boot: " ; *temp1; temp1 += 1,temp2 += 2)
      {   
        *temp2 = *temp1;    
      }  
      for(temp1 = str; *temp1; temp1 += 1,temp2 += 2)
      {
        *temp2 = *temp1;    
      }

   }
   //pic_irq_clear_mask(TIMER_IRQ | KEYBOARD_IRQ);
   //pic_irq_clear_mask(TIMER_IRQ);
   picack(TIMER_IRQ);

   if (subsec == 0)
      decrement_sleep();
   if (subsec%9 == 0)
      schedule();
   
}

void isr_timer_init()
{
   outb(0x43, 0b00010100);
   outb(0x40, 119);
}
