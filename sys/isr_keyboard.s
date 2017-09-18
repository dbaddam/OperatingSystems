.globl _isr_keyboard
.align 8

_isr_keyboard:
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
   call isr_keyboard
   popq %r11
   popq %r10
   popq %r9
   popq %r8
   popq %rdx
   popq %rcx
#   popq %rbx
   popq %rax
   iretq
