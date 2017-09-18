#include <sys/idt.h>
#include <sys/io.h>
#include <sys/pic.h>
#include <sys/kprintf.h>


#define KEYBOARD_IN_PORT 0x60
int shift = 0;
int ctrl = 0;

void printChar(char c)
{
   char *temp2;
   temp2 = (char*)0xb8000+(160*20)-6;
//    temp2 = (char *)0xBF050;  
 //for(temp1 = str; *temp1; temp1 += 1,temp2 += 2)
   {   
     //*temp2 = *temp1;
//       kprintf("%c\n",c);
       *temp2 = c;    
   }
   
   if(ctrl == 1)
   {
       *(temp2-2) = '^';
   }else *(temp2-2) = ' ';  
   
}

void isr_keyboard()
{
   unsigned int c = inb(KEYBOARD_IN_PORT);

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

//   char shifted[200];
//   char unshifted[200];
//   shifted[2] = '!';
//   unshifted[2] = '1';
  // int shift=0;
      

      //if control key is pressed, set ctrl variable
      if(c == 0x1D || c == 0xE0)
      {
         ctrl = 1;         
      } 

      //if control key is released, make ctrl 0
      if(c == 0xE0 || c == 0x9D)
      {
         ctrl = 0;
      }

      //if left shift  or right shift is released
      if(c == 0xAA || c == 0xB6)
      {   
         shift = 0;
      }
      
      //if left shift or right shift is pressed   
      if(c == 0x2A || c == 0x36)
      {   
         shift = 1;
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

   
   
//   printChar(c);    
   /*
   if (c == 0x02) 
   {
      printChar('1'); 
   }
   else if (c == 0x82) 
   {
      kprintf("Character - 1 released\n"); 
   }
   else if (c == 0x03) 
   {
      kprintf("Character - 2\n"); 
   }
   else if (c == 0x83) 
   {
      kprintf("Character - 2 released\n"); 
   }
   else
   {
      kprintf("Character - UNKNOWN\n"); 
   }
*/
   picack(KEYBOARD_IRQ); 
}
