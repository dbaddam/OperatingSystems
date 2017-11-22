#.globl _isr_timer
#.globl _isr_timer_init
.align 8

_isr_timer_init:
   pushq %rax
#   pushq %rbx
   pushq %rcx
   pushq %rdx
   pushq %r8
   pushq %r9
   pushq %r10
   pushq %r11
   cld
   cli
   call isr_timer_init
   popq %r11
   popq %r10
   popq %r9
   popq %r8
   popq %rdx
   popq %rcx
#   popq %rbx
   popq %rax
   retq
  

_isr_timer:
   pushq %rax
#   pushq %rbx
   pushq %rcx
   pushq %rdx
   pushq %r8
   pushq %r9
   pushq %r10
   pushq %r11
   cld
   cli
   call isr_timer
   popq %r11
   popq %r10
   popq %r9
   popq %r8
   popq %rdx
   popq %rcx
#   popq %rbx
   popq %rax
   iretq
