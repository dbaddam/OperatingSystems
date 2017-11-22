.globl _isr_keyboard
.globl _isr_timer
.globl _isr_timer_init
.align 8

.macro pushRegs
   pushq %rax
   pushq %rbx
   pushq %rcx
   pushq %rdx
   pushq %rsi
   pushq %rdi
   pushq %rbp
   pushq %r8
   pushq %r9
   pushq %r10
   pushq %r11
   pushq %r12
   pushq %r13
   pushq %r14
   pushq %r15
.endm

.macro popRegs
   popq %r15
   popq %r14
   popq %r13
   popq %r12
   popq %r11
   popq %r10
   popq %r9
   popq %r8
   popq %rbp
   popq %rdi
   popq %rsi
   popq %rdx
   popq %rcx
   popq %rbx
   popq %rax
.endm

_isr_keyboard:
#   cld
   cli
   pushRegs

   call isr_keyboard

   popRegs
   sti
   iretq

_isr_timer_init:
#   cld
   cli
   pushRegs

   call isr_timer_init

   popRegs
   sti
   retq
  

_isr_timer:
#   cld
   cli
   pushRegs

   call isr_timer

   popRegs
   sti
   iretq
