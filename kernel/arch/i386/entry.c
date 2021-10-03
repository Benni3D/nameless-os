#include <stddef.h>
#include <stdint.h>
#include "asm/int.h"

extern void init_traps(void);

extern void kernel_main(void);

void kernel_entry(uint32_t magic, const void* mbs) {
   (void)magic;
   (void)mbs;
   
   init_traps();

   //sti();

   __asm("int $0x03");

   kernel_main();

   while (1);
}
