#ifndef FILE_CTYPE_H
#define FILE_CTYPE_H

#define isinrange(ch, lo, hi) (((ch) >= (lo)) && ((ch) <= (hi)))

inline static int iscntrl(int ch) {
   return isinrange(ch, 0x00, 0x1F) || (ch == 0x7F);
}

inline static int isblank(int ch) {
   return (ch == 0x09) || (ch == 0x20);
}

inline static int isspace(int ch) {
   return isinrange(ch, 0x09, 0x0D) || (ch == 0x20);
}

inline static int isupper(int ch) {
   return isinrange(ch, 0x41, 0x5A);
}

inline static int islower(int ch) {
   return isinrange(ch, 0x61, 0x7A);
}

inline static int isalpha(int ch) {
   return isupper(ch) || islower(ch);
}

inline static int isdigit(int ch) {
   return isinrange(ch, 0x30, 0x39);
}

inline static int isxdigit(int ch) {
   return isdigit(ch) || isinrange(ch, 0x41, 0x46) || isinrange(ch, 0x61, 0x66);
}

inline static int isalnum(int ch) {
   return isalpha(ch) || isdigit(ch);
}

inline static int ispunct(int ch) {
   return isinrange(ch, 0x21, 0x2F)
      ||  isinrange(ch, 0x3A, 0x40)
      ||  isinrange(ch, 0x5B, 0x60)
      ||  isinrange(ch, 0x7B, 0x7E);
}

inline static int isgraph(int ch) {
   return isinrange(ch, 0x21, 0x7E);
}

inline static int isprint(int ch) {
   return isinrange(ch, 0x20, 0x7E);
}

inline static int isascii(int ch) {
   return isinrange(ch, 0x00, 0x7F);
}

#endif /* FILE_CTYPE_H */
