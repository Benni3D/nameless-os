#include <stdint.h>
#include "kernel/tty.h"
#include "kernel/printk.h"
#include "kernel/atkbd.h"
#include "asm/int.h"

void kernel_main(void) {
   tty_init();

   tty_printk = 1;
   printk("Hello World!\n");
   init_atkbd();

   printk("Hello back\n");
   sti();

   int tty = 2;
   while (1) {
      char ch;
      int rv = tty_read(tty, &ch, 1);
      tty_write(tty, &ch, 1);
   }
}
