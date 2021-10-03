#ifndef FILE_SYS_STAT_H
#define FILE_SYS_STAT_H
#include <sys/types.h>

struct stat {
   dev_t       st_dev;        // ID of device containing file
   ino_t       st_ino;        // Inode number
   mode_t      st_mode;       // File type and mode
   nlink_t     st_nlink;      // Number of hard links
   uid_t       st_uid;        // User ID of owner
   gid_t       st_gid;        // Group ID of owner
   dev_t       st_rdev;       // Device ID (if special file)
   off_t       st_size;       // Total size, in bytes

   time_t      st_atime;      // Time of last access
   time_t      st_mtime;      // Time of last modification
   time_t      st_ctime;      // Time of last status change
};

#define S_IFMT    00170000
#define S_IFREG    0100000
#define S_IFBLK    0060000
#define S_IFDIR    0040000
#define S_IFCHR    0020000
#define S_IFIFO    0010000
#define S_ISUID    0004000
#define S_ISGID    0002000
#define S_ISVTX    0001000

#define S_ISREG(m)   (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)   (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)   (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)   (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)  (((m) & S_IFMT) == S_IFIFO)

#define S_IRWXU   00700
#define S_IRUSR   00400
#define S_IWUSR   00200
#define S_IXUSR   00100

#define S_IRWXG   00070
#define S_IRGRP   00040
#define S_IWGRP   00020
#define S_IXGRP   00010

#define S_IRWXO   00007
#define S_IROTH   00004
#define S_IWOTH   00002
#define S_IXOTH   00001

int chmod(const char* path, mode_t mode);
int mkdir(const char* path, mode_t mode);
int mkfifo(const char* path, mode_t mode);
int fstat(int fd, struct stat* stbuf);
int stat(const char* path, struct stat* stbuf);
mode_t umask(mode_t mode);

#endif /* FILE_SYS_STAT_H */
