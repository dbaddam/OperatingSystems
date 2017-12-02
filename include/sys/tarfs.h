#ifndef _TARFS_H
#define _TARFS_H

extern char _binary_tarfs_start;
extern char _binary_tarfs_end;

#define STDIN 0
#define STDOUT 1
#define STDERR 2

struct posix_header_ustar {
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char checksum[8];
  char typeflag[1];
  char linkname[100];
  char magic[6];
  char version[2];
  char uname[32];
  char gname[32];
  char devmajor[8];
  char devminor[8];
  char prefix[155];
  char pad[12];
};

/*As of now defining the below struct here, IDK kishan!
struct dirent
{
   uint64_t d_ino;
   uint64_t d_off;
   uint64_t d_reclen;
   char d_type;
#define MAX_SIZE 256
   char d_name[MAX_SIZE];
};
typedef struct dirent dirent;
*/

//struct DIR


//shall define all the functions we mentioned in the tarfs.c
void init_tarfs();
int getFileFromTarfs(char *filename, char **file_start_address);
uint32_t open(char* path, int32_t mode);
uint64_t read(uint32_t fd, char *buf, uint64_t count);
int32_t readLine(char *buf);
int close(uint32_t fd);
int opendir_tarfs(char *path, uint64_t flags);
uint32_t readdir_tarfs(uint32_t fd, char *buf);
int closedir_tarfs(int fd);
int is_directory(char *dirname);
int sanitize_path(char *path, char *result);
#endif
