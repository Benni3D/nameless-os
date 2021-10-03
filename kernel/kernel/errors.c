#include "kernel/errors.h"
#include "kernel/util.h"

static char* error_strs[] = {
   [0]         = "Success",
   [ENOTTY]    = "Not a typewriter",
   [ENODEV]    = "No such device",
   [EOVERFLOW] = "Overflow",
   [EIO]       = "I/O error",
   [ENOENT]    = "No such file or directory",
   [EISDIR]    = "Is a directory",
   [ENOTDIR]   = "Not a directory",
   [EINVAL]    = "Invalid argument",
};

char* strerror(int err) {
   if (err < arraylen(error_strs)) {
      return error_strs[err];
   } else {
      return "invalid error";
   }
}
