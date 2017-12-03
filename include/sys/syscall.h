#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <sys/defs.h>

#define __NR_read                               0ul
#define __NR_write                              1ul
#define __NR_open                               2ul
#define __NR_close                              3ul
#define __NR_lseek                              8ul
#define __NR_access                             21ul
#define __NR_pipe                               22ul
#define __NR_sched_yield			24ul
#define __NR_dup2                               33ul
#define __NR_getpid                             39ul
#define __NR_fork                               57ul
#define __NR_execve                             59ul
#define __NR_wait4                              61ul
#define __NR_getdents                           78ul
#define __NR_getcwd                             79ul
#define __NR_exit                               60ul
#define __NR_chdir                              80ul
#define __NR_getppid                            110ul
#define __NR_exit_group                         231ul
#define __NR_readdirt                           232ul
#define __NR_writedirt                          233ul
#define __NR_opendirt                           234ul
#define __NR_closedirt                          235ul
#define __NR_sleep				400ul
#define __NR_sbrk				401ul
#define __NR_ps  				402ul
#define __NR_kill  				403ul

size_t sys_write(int, char*, int);
void init_syscall();
void _syscall_entry();
#endif
