#ifndef FILE_SYS_TYPES_H
#define FILE_SYS_TYPES_H
#include <stddef.h>
#include <stdint.h>

typedef long time_t;

typedef int pid_t;
typedef uint16_t uid_t;
typedef uint16_t gid_t;
typedef uint16_t dev_t;
typedef uint16_t ino_t;
typedef uint16_t mode_t;
typedef uint16_t umode_t;
typedef uint8_t  nlink_t;
typedef int32_t  daddr_t;
typedef int32_t  off_t;

typedef struct { int qout, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;

struct ustat {
   daddr_t  f_tfree;    // Total free blocks
   ino_t    f_tinode;   // Number of free inodes
   char     f_fname[6]; // Filesystem name
   char     f_fpack[6]; // Filesystem pack name
};

#endif /* FILE_SYS_TYPES_H */
