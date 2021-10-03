#include "string.h"

void* memcpy(void* restrict dest, const void* restrict src, size_t num) {
   char* d = dest;
   const char* s = src;
   for (size_t i = 0; i < num; ++i) {
      d[i] = s[i];
   }
   return dest;
}

void* memmove(void* dest, const void* src, size_t num) {
   char* d = dest;
   const char* s = src;
   if (d < s) {
      for (size_t i = 0; i < num; ++i) {
         d[i] = s[i];
      }
   } else if (d > s) {
      for (size_t i = num; i != 0; --i) {
         d[i - 1] = s[i - 1];
      }
   }
   return dest;
}
void* memset(void* dest, int val, size_t num) {
   char* d = dest;
   for (size_t i = 0; i < num; ++i) {
      d[i] = val;
   }
   return dest;
}

char* strcpy(char* restrict dest, const char* restrict src) {
   char ch;
   do {
      ch = *src++;
      *dest++ = ch;
   } while (ch);
   return dest;
}

char* strncpy(char* restrict dest, const char* restrict src, size_t num) {
   char ch = '\0';
   for (size_t i = 0; i < (num - 1); ++i) {
      ch = *src++;
      *dest++ = ch;
      if (!ch)
         break;
   }
   if (ch) {
      *dest++ = '\0';
   }
   return dest;
}


size_t strlen(const char* str) {
   size_t len = 0;
   while (*str)
      ++len;
   return len;
}
