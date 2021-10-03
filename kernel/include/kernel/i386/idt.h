#ifndef FILE_I386_IDT_H
#define FILE_I386_IDT_H
#include <stdint.h>

extern uint64_t idt[256];

void set_idt_gate(unsigned num, void* handler, uint8_t type, uint8_t dpl);

#define set_intr_gate(num, handler) \
   set_idt_gate(num, handler, 14, 0)

#define set_trap_gate(num, handler) \
   set_idt_gate(num, handler, 15, 0)

#define set_syst_gate(num, handler) \
   set_idt_gate(num, handler, 15, 3)

#endif /* FILE_I386_IDT_H */
