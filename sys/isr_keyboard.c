#include <sys/idt.h>
#include <sys/io.h>
#include <sys/pic.h>
#include <sys/kprintf.h>


#define KEYBOARD_IN_PORT 0x60

void isr_keyboard()
{
   unsigned char c = inb(KEYBOARD_IN_PORT);

   /* Dinesh babu,
      1. Here you may have to use an array for scancodes
         which map 0x02-> 1 , 0x03->2, 0x82->'1 released'
         and so on. 
      2. You should probably maintain a 'shift' and 'control'
         variables specially which would if 'shift' or 'control'
         keys are pressed BUT NOT released.
      3. You have to ignore printing all the 'released' keys
      4. The spec says just print the last pressed character
         next to our timer. Don't print all the characters to the console.
         Ofcourse, you can test initially using printf like I did.*/
    
   if (c == 0x02) /*1 pressed*/
   {
      kprintf("Character - 1\n"); 
   }
   else if (c == 0x82) /*1 released*/
   {
      kprintf("Character - 1 released\n"); 
   }
   else if (c == 0x03) /*2 pressed*/
   {
      kprintf("Character - 2\n"); 
   }
   else if (c == 0x83) /*2 pressed*/
   {
      kprintf("Character - 2 released\n"); 
   }
   else
   {
      kprintf("Character - UNKNOWN\n"); 
   }

   picack(KEYBOARD_IRQ); 
}
