#include "kernel/errors.h"
#include "sys/utsname.h"
#include "string.h"

int uname(struct utsname* utsbuf) {
   if (!utsbuf) {
      return -EFAULT;
   }

   strcpy(utsbuf->sysname,    "Unnamed");
   strcpy(utsbuf->nodename,   "localhost");
   strcpy(utsbuf->release,    "0.0.0");
   strcpy(utsbuf->version,    "0.0.0");
   strcpy(utsbuf->machine,    "i386");
   return 0;
}
