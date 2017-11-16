.text

.globl _syscall_entry
.align 8
#.external cur_task

#TODOKISHAN - Should we save other registers as well?
# Also rax is supposed to have the return value. We shouldn't
# pop it at the end.
# The message in OSDev is 'All registers, except rcx and r11 
# (and the return value, rax), are preserved during the syscall.'

_syscall_entry:
   cli
#   movq 40(cur_task), %rsp
   pushq %rcx
   pushq %r8
   pushq %r9
#   pushq %r11
   pushq %r10
   pushq %rdx
   pushq %rsi
   pushq %rdi
#   pushq %rax
#   call sys_write
/* According to AMD64 ABI When 'call fn' is called, the function expects
   the arguments to be in registers rdi, rsi, rdx, rcx, r8 and r9. But
   for syscall, the first argument is*/
/*
   movq %r8, %r9
   movq %r10, %r8
   movq %rdx, %r10
   movq %rsi, %rdx
   movq %rdi, %rsi
   movq %rax, %rdi
*/
/*Here we are basically messing up the 6th argument to pass syscall number */
   movq %rax, %r9
   callq syscall_handler
#   popq %rax
   popq %rdi
   popq %rsi
   popq %rdx
   popq %r10
#   popq %r11
   popq %r9
   popq %r8
   popq %rcx
   sti
   sysretq
