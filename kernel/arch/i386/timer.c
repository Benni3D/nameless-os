#include <stdint.h>
#include "kernel/printk.h"
#include "asm/port.h"

// TODO: implement scheduling
void handle_timer(uint32_t esp) {
   //printk("Timer\n");
   send_EOI(0x20);
}
