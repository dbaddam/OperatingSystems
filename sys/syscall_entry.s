.text

.globl _syscall_entry
.align 8
#.external cur_task


# Hack - I'll be using r14, r15 to swap user/kernel stack here
# I have added them as clobber registers during syscall so, we
# can use them here and not restore them. 
_syscall_entry:
   # Swap the stack
   movq %rsp, %r14
   movq cur_task, %r15
   movq %rsp, 80(%r15)
   add  $0x1020, %r15  /* point to kstack[505]*/
   movq %r15, %rsp

   pushq %r14
   pushq %rcx
   pushq %r8
   pushq %r9
#   pushq %r11
   pushq %r10
   pushq %rdx
   pushq %rsi
   pushq %rdi
#   pushq %rbp
#   pushq %rbx
#   pushq %rax
/*Here we are basically messing up the 6th argument to pass syscall number */
   movq %rax, %r9
   callq syscall_handler
#   popq %rax
#   popq %rbx
#   popq %rbp
   popq %rdi
   popq %rsi
   popq %rdx
   popq %r10
#   popq %r11
   popq %r9
   popq %r8
   popq %rcx
   popq %r14
   movq %r14, %rsp
   sysretq
