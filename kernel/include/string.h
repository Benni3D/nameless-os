#ifndef FILE_STRING_H
#define FILE_STRING_H
#include <stddef.h>

void* memcpy(void* restrict, const void* restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);

char* strcpy(char* restrict, const char* restrict);
char* strncpy(char* restrict, const char* restrict, size_t);

char* strcat(char* restrict, const char* restrict);
char* strncat(char* restrict, const char* restrict, size_t);

int memcmp(const void*, const void*, size_t);
int strcmp(const char*, const char*);
int strncmp(const char*, const char*, size_t);

size_t strlen(const char*);

char* strerror(int);

#endif /* FILE_STRING_H */
