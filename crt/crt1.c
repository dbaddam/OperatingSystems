#include <stdlib.h>
#include <stdio.h>



void _start(void) {

   int    argc = 1;
   char **argv = NULL;
   char **envp = NULL;
   void  *rsp;

   __asm__ __volatile__(
        "movq %%rsp, %0"
        :"=r"(rsp)
        :
        : "rsp"
   );

   argc = *((int*)(rsp + 24));
   argv = rsp + 32;
   envp = rsp + 24 + ((argc + 2)*8);
   setupenv(envp); 
   exit(main(argc, argv, envp));
}
