#ifndef FILE_KERNEL_PRINTK_H
#define FILE_KERNEL_PRINTK_H
#include <stdnoreturn.h>
#include <stdarg.h>

extern int tty_printk;

noreturn void panic(const char*, ...);
void printk(const char*, ...);
void vprintk(const char*, va_list);

#endif /* FILE_KERNEL_PRINTK_H */
