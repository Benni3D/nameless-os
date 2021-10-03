#include "kernel/printk.h"
#include "kernel/tty.h"
#include "snprintf.h"

extern noreturn void halt();

int tty_printk;

noreturn void panic(const char* fmt, ...) {
   va_list ap;
   va_start(ap, fmt);

   printk("Kernel panic: ");
   vprintk(fmt, ap);
   va_end(ap);

   halt();
}

void printk(const char* fmt, ...) {
   va_list ap;
   va_start(ap, fmt);

   vprintk(fmt, ap);

   va_end(ap);
}

void vprintk(const char* fmt, va_list ap) {
   static char buf[512];
   const int num = vsnprintf(buf, sizeof(buf), fmt, ap);
   if (num > 0)
      tty_write(tty_printk, buf, num);
}
