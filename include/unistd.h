#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/defs.h>

int open(const char *pathname, int flags);
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int unlink(const char *pathname);

#define R_OK    4               /* Test for read permission.  */
#define W_OK    2               /* Test for write permission.  */
#define X_OK    1               /* Test for execute permission.  */
#define F_OK    0               /* Test for existence.  */
int access(const char *pathname, int mode);

int chdir(const char *path);
char *getcwd(char *buf, size_t size);

pid_t fork();
int execve(const char *file, char *const argv[], char *const envp[]);
int execvpe(const char *file, char *const argv[], char *const envp[]);
int execvp(const char *file, char *const argv[]);

pid_t wait(int *status);
int waitpid(int pid, int *status);

unsigned int sleep(unsigned int seconds);

pid_t getpid(void);
pid_t getppid(void);

// OPTIONAL: implement for ``on-disk r/w file system (+10 pts)''
#define SEEK_SET 0
#define SEEK_CUR 1
off_t lseek(int fd, off_t offset, int whence);
//int mkdir(const char *pathname, mode_t mode);

// OPTIONAL: implement for ``signals and pipes (+10 pts)''
int pipe(int pipefd[2]);

int dup2(int oldfd, int newfd);

typedef unsigned long long ull;

#define syscall0(name) \
ull sysret; \
__asm__ __volatile__ ( \
    "movq %1, %%rax\n\t" \
    "syscall\n\t" \
    "movq %%rax, %0\n\t" \
    : "=r"(sysret) \
    : "r"(__NR_##name) \
    : "rax", "memory" \
                     ); 

#define syscall1(name, a1) \
ull sysret; \
__asm__ __volatile__ ( \
    "movq %1, %%rax\n\t" \
    "movq %2, %%rdi\n\t" \
    "syscall\n\t" \
    "movq %%rax, %0\n\t" \
    : "=r"(sysret) \
    : "r"(__NR_##name), "r" ((ull)a1) \
    : "rax", "rdi", "memory" \
                     ); 

#define syscall2(name, a1, a2) \
ull sysret; \
__asm__ __volatile__ ( \
    "movq %1, %%rax\n\t" \
    "movq %2, %%rdi\n\t" \
    "movq %3, %%rsi\n\t" \
    "syscall\n\t" \
    "movq %%rax, %0\n\t" \
    : "=r"(sysret) \
    : "r"(__NR_##name), "r" ((ull)a1), "r" ((ull)a2) \
    : "rax", "rdi", "rsi", "memory" \
                     ); 

#define syscall3(name, a1, a2, a3) \
ull sysret; \
__asm__ __volatile__ ( \
    "movq %1, %%rax\n\t" \
    "movq %2, %%rdi\n\t" \
    "movq %3, %%rsi\n\t" \
    "movq %4, %%rdx\n\t" \
    "syscall\n\t" \
    "movq %%rax, %0\n\t" \
    : "=r"(sysret) \
    : "r"(__NR_##name), "r" ((ull)a1), "r" ((ull)a2), "r" ((ull)a3) \
    : "rax", "rdi", "rsi", "rdx", "memory" \
                     ); 

#define syscall4(name, a1, a2, a3, a4) \
ull sysret; \
__asm__ __volatile__ ( \
    "movq %1, %%rax\n\t" \
    "movq %2, %%rdi\n\t" \
    "movq %3, %%rsi\n\t" \
    "movq %4, %%rdx\n\t" \
    "movq %4, %%r10\n\t" \
    "syscall\n\t" \
    "movq %%rax, %0\n\t" \
    : "=r"(sysret) \
    : "r"(__NR_##name), "r" ((ull)a1), "r" ((ull)a2), "r" ((ull)a3) \
    : "rax", "rdi", "rsi", "rdx", "r10", "memory" \
                     ); 
#endif
