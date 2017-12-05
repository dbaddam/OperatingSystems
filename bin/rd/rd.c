#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

//#include <sbuutil.h>

int main(int argc, char* argv[], char* envp[])
{
           DIR *dirp;
           dirp = opendir("usr");
           if (dirp == NULL)
           {
              puts("error - path is not correct");
              //write(1, "something went wrong in getting dirp pointer!", 256);
              return -1;
           }else{
              printf("1. succesfully opened path = usr, fd = %d\n", dirp->fd);
           }
           puts("2. The directories in path = usr are: ");
           dirent *d;
           while((d = readdir(dirp)) != NULL)
           {
              write(1, d->d_name, strlen(d->d_name));
              write(1, "  ", 2);
           }
           puts("");
         /*  closedir(dirp);
           puts("3. succesfully closed path = usr.");
    */

           DIR *dirp1;
           dirp1 = opendir("usr");
           if (dirp1 == NULL)
           {
              puts("error - path is not correct");
              //write(1, "something went wrong in getting dirp pointer!", 256);
              return -1;
           }else{
              printf("1. succesfully opened path = usr, fd = %d\n", dirp1->fd);
           }
           puts("2. The directories in path = usr are: ");
           dirent *d1;
           while((d1 = readdir(dirp1)) != NULL)
           {
              write(1, d1->d_name, strlen(d1->d_name));
              write(1, "  ", 2);
           }
           puts("");
         /*  closedir(dirp1);
           puts("3. succesfully closed path = usr.");
       */
      
           DIR *dirp2;
           dirp2 = opendir("usr/dinesh");
           if (dirp2 == NULL)
           {
              puts("error - path is not correct");
              //write(1, "something went wrong in getting dirp pointer!", 256);
              return -1;
           }else{
              printf("1. succesfully opened path = usr/dinesh, fd = %d\n", dirp2->fd);
           }
           puts("2. The directories in path = usr/dinesh are: ");
           dirent *d2;
           while((d2 = readdir(dirp2)) != NULL)
           {
              write(1, d2->d_name, strlen(d2->d_name));
              write(1, "  ", 2);
           }
           puts("");
       /*    closedir(dirp2);
           puts("3. succesfully closed path = usr/dinesh.");
       */
           return 0;
          
}
