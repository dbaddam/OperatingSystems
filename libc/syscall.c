#include <stdlib.h>
#include <syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define MAX_FILE_SIZE 1024

void ps()
{
   syscall0(ps);
}

void kill(int pid)
{
   syscall1(kill, pid);
}

int access(const char *pathname, int mode)
{
   syscall2(access, pathname, mode);

   return (int)sysret;
}

int chdir(const char* path)
{
   syscall1(chdir, path);
   return (int)sysret;
}

void exit(int status)
{
   syscall1(exit, status);
}

pid_t fork()
{
   syscall0(fork);

   return (pid_t)sysret;
}

char *getcwd(char *buf, size_t size)
{
   syscall2(getcwd, buf, size);

   return (char*)sysret;
}

int close(int fd)
{
   syscall1(close, fd);

   return (int)sysret;
}

int dup2(int oldfd, int newfd)
{
   syscall2(dup2, oldfd, newfd);

   return (int)sysret;   
}


int execvpe(const char *filename, char *const argv[], char *const envp[])
{
   char  file[MAX_FILE_SIZE];
   char *path = getenv("PATH");
   int   i;

   /* If it's a relative path we don't search in PATH */
   for (i = 0; filename[i];i++)
   {
       if (filename[i] == '/')
          return execve(filename, argv, envp);
   }

   /* If the file is in this directory, execute now */
   if (!access(filename, F_OK))
      return execve(filename, argv, envp);

   if (!path)
      return -1;

   while (*path)
   {
      char* keystart = path;
      char* keyend = keystart;

      while (*keyend && *keyend != ':')
         keyend++;

      strncpy(file, path, (keyend-keystart));

      file[keyend-keystart] = '/';
      file[keyend-keystart+1] = '\0';
 
      strcat(file, filename);

      if (!access(file, F_OK))
         break;

      if (!*keyend)
         return 1;

      path += (keyend-keystart)+1;
   }

   return execve(file, argv, envp);
}

int execve(const char *filename, char *const argv[], char *const envp[])
{
   syscall3(execve, filename, argv, envp);

   return (int)sysret;
}

int execvp(const char *filename, char *const argv[])
{
   return execvpe(filename, argv, _sbush_env);
}

pid_t getpid()
{
   syscall0(getpid);

   return (pid_t)sysret;
}

pid_t getppid()
{
   syscall0(getppid);

   return (pid_t)sysret;
}

int pipe(int pipefd[2])
{
   syscall1(pipe, pipefd);

   return (int)sysret;
}

int open(const char *pathname, int flags)
{
   syscall2(open, pathname, flags);

   return (int)sysret;
}

ssize_t read(int fd, void *buf, size_t count)
{
  syscall3(read, fd, buf, count); 

  return (ssize_t) sysret;
}

off_t lseek(int fd, off_t offset, int whence)
{
   syscall3(lseek, fd, offset, whence);
   return (off_t)sysret;
}

unsigned int sleep(unsigned int secs)
{
   syscall1(sleep, secs);

   return (unsigned int)sysret;
}

pid_t wait(int *status)
{
   return wait4(-1, status, 0, NULL);
}

int waitpid(int pid, int *status)
{
   return wait4(pid, status, 0, NULL);
}

pid_t wait4(pid_t pid, int *status, int options, rusage* usage)
{
   syscall4(wait4, pid, status, options, usage);

   return (pid_t)sysret;
}

ssize_t write(int fd, const void *buf, size_t count)
{
  syscall3(write, fd, buf, count); 

  return (ssize_t) sysret;
}

void yield()
{
   syscall0(sched_yield);
}
