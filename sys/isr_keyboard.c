#include <sys/idt.h>
#include <sys/io.h>
#include <sys/pic.h>
#include <sys/kprintf.h>

#define MAX_BUFFER       4096
#define KEYBOARD_IN_PORT 0x60
int shift = 0;
int ctrl = 0;
int lines = 0;

char ui_input[MAX_BUFFER];
int index=0;

void printChar1(char c)
{
   char *temp2;
   temp2 = (char*)VIDEO_MEM_BASE+(80*45)-6;
       *temp2 = c;    
   
   if(ctrl == 1)
   {
       *(temp2-2) = '^';
   }else *(temp2-2) = ' ';  
   
}


void printChar(char c)
{
   // we are getting a '\b' character when a backspace is pressed along with its hex value 
   // I DO NOT KNOW WHY ???, so simply handling it here.
   if(c=='\b')
   {
      return;
   }
   if(ctrl == 1)
   {
      kprintf("%c",'^');
      ctrl = 0;
   }
   
   // 1. store in buffer
   if(c == '\n' || c == '\r')
   {
      lines++;
   }
   index = (index < MAX_BUFFER) ? index : 0;
   ui_input[index++] = c;

   // 2. send to kprintf as it will echo to screen
   kprintf("%c",c);
}

void isr_keyboard()
{
   //pic_irq_set_mask(KEYBOARD_IRQ);
   //pic_irq_set_mask(TIMER_IRQ | KEYBOARD_IRQ);
   unsigned int c = inb(KEYBOARD_IN_PORT);
  
  /* Reference:
   * http://www.cs.umd.edu/~hollings/cs412/s98/project/proj1/scancode 
   */

  char asccode[58][2] =       
			       
     {
       {   0,0   } ,
       { ' ',' ' } ,
       { '1','!' } ,
       { '2','@' } ,
       { '3','#' } ,
       { '4','$' } ,
       { '5','%' } ,
       { '6','^' } ,
       { '7','&' } ,
       { '8','*' } ,
       { '9','(' } ,
       { '0',')' } ,
       { '-','_' } ,
       { '=','+' } ,
       {   8,8   } ,
       {   9,9   } ,
       { 'q','Q' } ,
       { 'w','W' } ,
       { 'e','E' } ,
       { 'r','R' } ,
       { 't','T' } ,
       { 'y','Y' } ,
       { 'u','U' } ,
       { 'i','I' } ,
       { 'o','O' } ,
       { 'p','P' } ,
       { '[','{' } ,
       { ']','}' } ,
       {  13,13  } ,
       {   0,0   } ,
       { 'a','A' } ,
       { 's','S' } ,
       { 'd','D' } ,
       { 'f','F' } ,
       { 'g','G' } ,
       { 'h','H' } ,
       { 'j','J' } ,
       { 'k','K' } ,
       { 'l','L' } ,
       { ';',':' } ,
       {  39,34  } ,
       { '`','~' } ,
       {   0,0   } ,
       { '\\','|'} ,
       { 'z','Z' } ,
       { 'x','X' } ,
       { 'c','C' } ,
       { 'v','V' } ,
       { 'b','B' } ,
       { 'n','N' } ,
       { 'm','M' } ,
       { ',','<' } ,
       { '.','>' } ,
       { '/','?' } ,
       {   0,0   } ,
       {   0,0   } ,
       {   0,0   } ,
       { ' ',' ' } ,
   };
      // if backspace is pressed, we go ahead and send that to kprintf
      // kprintf handles that backspace
      if(c == 0x0E)
      {
         //kprintf("backsapce!");
         printChar(c);
      }

      //if control key is pressed, set ctrl variable
      if(c == 0x1D || c == 0xE0)
      {
         ctrl = 1;
         goto end;         
      } 

      //if control key is released, make ctrl 0
      if(c == 0xE0 || c == 0x9D)
      {
         ctrl = 0;
         goto end;         
      }

      //if left shift  or right shift is released
      if(c == 0xAA || c == 0xB6)
      {   
         shift = 0;
         goto end;         
      }
      
      //if left shift or right shift is pressed   
      if(c == 0x2A || c == 0x36)
      {   
         shift = 1;
         goto end;         
      }

   if(c < 58)
   {
      if(shift == 1 || ctrl == 1)
      {      
         printChar(asccode[c][1]);
      }   
      else 
      {      
         printChar(asccode[c][0]);
      }
   }

end:
   //pic_irq_clear_mask(TIMER_IRQ | KEYBOARD_IRQ);
   //pic_irq_clear_mask(KEYBOARD_IRQ);
   picack(KEYBOARD_IRQ); 
}
