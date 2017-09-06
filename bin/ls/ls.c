       #include <dirent.h>
       #include <stdio.h>
       #include <unistd.h>
       #include <stdlib.h>

       #define MAX_BUFFER_SIZE 1024

       
       int main(int argc, char *argv[], char *envp[])
       {
           int fd, n;
           char buffer[MAX_BUFFER_SIZE];
           struct linux_dirent* d;
           int pos;

           fd = open(argc > 1 ? argv[1] : ".", O_RDONLY /*| O_DIRECTORY*/ );
           if (fd == -1)
               fputs("wrong with fd!", stdout);

           for ( ; ; ) {
               n = getdents(fd, (struct linux_dirent *)buffer, MAX_BUFFER_SIZE);
               if (n == -1)
                    fputs("Something wrong!", stdout);

               if (n == 0)
                   break;

               for (pos = 0; pos < n;) {
                   d = (struct linux_dirent *) (buffer + pos);
                   fputs(d->d_name, stdout);
                   fputs("  ", stdout);
                   pos += d->d_reclen;
               }
               fputs("\n", stdout);
           }

           close(fd);
           return 0;
       }
