.globl switch_task
.align 8

/* rdi - old thread
   rsi - new thread*/
switch_task:
   movq %rdi, %r8 
   movq (%rsp), %rax
 
   movq %r15, 0(%r8) 
   movq %r14, 8(%r8) 
   movq %r13,16(%r8) 
   movq %r12,24(%r8) 
   movq %rbp,32(%r8) 
   movq %rsp,40(%r8) 
   movq %rbx,48(%r8)
   #movq %cr3,64(%r8)
   # move return address into rip 
   movq %rax,56(%r8) 

  /* Get the new thread contents*/ 
   movq  %rsi, %r8
   movq  0(%r8), %r15  
   movq  8(%r8), %r14
   movq 16(%r8), %r13
   movq 24(%r8), %r12
   movq 32(%r8), %rbp
   movq 40(%r8), %rsp
   movq 48(%r8), %rbx
   #movq %cr3,64(%r8)
   movq 56(%r8), %rax
   movq %rax, (%rsp) 
   retq
  
