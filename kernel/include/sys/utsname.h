#ifndef FILE_SYS_UTSNAME_H
#define FILE_SYS_UTSNAME_H
#include <sys/types.h>

struct utsname {
   char sysname[33];    // Operating system name
   char nodename[33];   // Name within network/Hostname
   char release[33];    // Operating system release
   char version[33];    // Operating system version
   char machine[33];    // Hardware identifier
};

int uname(struct utsname* utsbuf);

#endif /* FILE_SYS_UTSNAME_H */
