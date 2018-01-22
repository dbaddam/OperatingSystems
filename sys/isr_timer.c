#include <sys/os.h>
#include <sys/pic.h>
#include <sys/io.h>


uint64_t secs = 0;
uint64_t subsec = 0;

void isr_timer()
{

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
      temp2 = (char*)(VIDEO_MEM_BASE)+160*24+80;;
      for(temp1 = "Time since boot: " ; *temp1; temp1 += 1,temp2 += 2)
      {   
        *temp2 = *temp1;    
      }  
      for(temp1 = str; *temp1; temp1 += 1,temp2 += 2)
      {
        *temp2 = *temp1;    
      }

   }
   picack(TIMER_IRQ);

   if (subsec == 0)
      decrement_sleep();

   /* Pre-emptive scheduling - every 38 ms */
   if (subsec%2 == 0)
      schedule();
   
}

void isr_timer_init()
{
   outb(0x43, 0b00010100);
   outb(0x40, 119);
}
