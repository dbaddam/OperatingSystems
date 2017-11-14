#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <sys/defs.h>

void sys_write();
void init_syscall();
void _syscall_entry();
#endif
