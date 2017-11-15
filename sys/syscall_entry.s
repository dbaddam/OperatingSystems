.text

.globl _syscall_entry
.align 8
#.external cur_task

_syscall_entry:
   cli
#   movq 40(cur_task), %rsp
   pushq %rax
   pushq %rdi
   pushq %rsi
   pushq %rcx
   pushq %rdx
   pushq %r8
   pushq %r9
   pushq %r10
   pushq %r11
   call sys_write
   popq %r11
   popq %r10
   popq %r9
   popq %r8
   popq %rdx
   popq %rcx
   popq %rsi
   popq %rdi
   popq %rax
   sti
   sysretq
