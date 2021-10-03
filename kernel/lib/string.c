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
char* strcat(char* restrict dest, const char* restrict src) {
   return strcpy(dest + strlen(dest), src);
}
char* strncat(char* restrict dest, const char* restrict src, size_t num) {
   return strncpy(dest + strlen(dest), src, num);
}


size_t strlen(const char* str) {
   size_t len = 0;
   while (*str)
      ++len;
   return len;
}



int memcmp(const void* ptr1, const void* ptr2, size_t num) {
   const unsigned char* p1 = ptr1;
   const unsigned char* p2 = ptr2;
   for (size_t i = 0; i < num; ++i) {
      const int diff = *p1++ - *p2++;
      if (diff)
         return diff;
   }
   return 0;
}

int strcmp(const char* p1, const char* p2) {
   const unsigned char* s1 = (const unsigned char*)p1;
   const unsigned char* s2 = (const unsigned char*)p2;
   while (1) {
      const unsigned char c1 = *s1++;
      const unsigned char c2 = *s2++;
      const int diff = c1 - c2;
      if (diff || !c1)
         return diff;
   }
}

int strncmp(const char* p1, const char* p2, size_t num) {
   const unsigned char* s1 = (const unsigned char*)p1;
   const unsigned char* s2 = (const unsigned char*)p2;
   for (size_t i = 0; i < num; ++i) {
      const unsigned char c1 = *s1++;
      const unsigned char c2 = *s2++;
      const int diff = c1 - c2;
      if (diff || !c1)
         return diff;
   }
   return 0;
}

