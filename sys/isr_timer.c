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
      char str[25] ;
      secs++;
      subsec = 0;
      
      // convert the secs in int to string and add 's' to it, Ex: "23s".
      // intTOstring(secs, str, 10);
      unsignedLongTOstring(secs, str, 10);
      str[stringlen(str)] = 's';
      str[stringlen(str)+1] = '\0';

      char *temp2, *temp1;
      temp2 = (char*)0xb8000+80*45;
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
}

void isr_timer_init()
{
   outb(0x43, 0b00010100);
   outb(0x40, 119);
}
