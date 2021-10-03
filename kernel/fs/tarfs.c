#include <stdint.h>
#include "kernel/errors.h"
#include "fs/tarfs.h"
#include "string.h"

static size_t parse_octal(const char* str) {
   size_t n = 0;
   for (size_t i = 0; str[i]; ++i) {
      n = n * 8 + (str[i] - '0');
   }
   return n;
}

int tar_lookup(const void* archive, const char* filename, const void** out, size_t* size) {
   const struct tar_header* ptr = archive;
   while (memcmp(ptr->ustar, "ustar", 5) == 0) {
      const size_t filesz = parse_octal(ptr->size);
      if (!strcmp(filename, ptr->filename)) {
         *out = (void*)((size_t)ptr + 512);
         *size = filesz;
         return 0;
      }
      ptr = (struct tar_header*)((size_t)ptr + (size_t)((((filesz + 511) / 512) + 1) * 512));
   }
   return -ENOENT;
}
