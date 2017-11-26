#ifndef _DIRENT_H
#define _DIRENT_H

#define NAME_MAX 255

typedef unsigned long off_t;

struct dirent {
// unsigned long d_ino;
// unsigned long d_off;
// unsigned short d_reclen;
 char d_name[NAME_MAX+1];
};
typedef struct dirent dirent;

struct linux_dirent {
           long           d_ino;
           off_t          d_off;
           unsigned short d_reclen;
           char           d_name[];
       };

typedef struct DIR DIR;

DIR *opendir(const char *name);
dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
int getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count);
#endif
