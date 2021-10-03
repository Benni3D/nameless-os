#include <stdint.h>
#include "kernel/i386/idt.h"
#include "kernel/printk.h"

extern void halt();

static void handle_trap(const char* str, uint32_t esp, uint32_t nr) {
   printk("Exception: %s\nesp = %p\nnr = %u\n", str, esp, nr);
}

extern void excep_divide_error();
void handle_divide_error(uint32_t esp, uint32_t nr) {
   handle_trap("divide error", esp, nr);
}

extern void excep_int3();
extern void excep_debug();
void handle_int3(uint32_t esp, uint32_t nr) {
   handle_trap("debug", esp, nr);
}

extern void excep_nmi();
void handle_nmi(uint32_t esp, uint32_t nr) {
   handle_trap("nmi", esp, nr);
}

extern void excep_overflow();
void handle_overflow(uint32_t esp, uint32_t nr) {
   handle_trap("overflow", esp, nr);
}

extern void excep_bounds();
void handle_bounds(uint32_t esp, uint32_t nr) {
   handle_trap("bounds", esp, nr);
}

extern void excep_invalid_op();
void handle_invalid_op(uint32_t esp, uint32_t nr) {
   handle_trap("invalid operand", esp, nr);
}

extern void excep_no_coprocessor();
void handle_no_coprocessor(uint32_t esp, uint32_t nr) {
   handle_trap("no coprocessor", esp, nr);
}

extern void excep_coprocessor_segment_overrun();
void handle_coprocessor_segment_overrun(uint32_t esp, uint32_t nr) {
   handle_trap("coprocessor segment overrun", esp, nr);
}

extern void excep_reserved();
void handle_reserved(uint32_t esp, uint32_t nr) {
   handle_trap("reserved trap", esp, nr);
}

extern void excep_coprocessor();
void handle_coprocessor(uint32_t esp, uint32_t nr) {
   handle_trap("coprocessor error", esp, nr);
}

extern void excep_double_fault();
void handle_double_fault(uint32_t esp, uint32_t nr) {
   handle_trap("double fault", esp, nr);
}

extern void excep_invalid_TSS();
void handle_invalid_TSS(uint32_t esp, uint32_t nr) {
   handle_trap("invalid TSS", esp, nr);
}

extern void excep_segment_not_present();
void handle_segment_not_present(uint32_t esp, uint32_t nr) {
   handle_trap("segment not present", esp, nr);
}

extern void excep_stack_segment();
void handle_stack_segment(uint32_t esp, uint32_t nr) {
   handle_trap("stack segment", esp, nr);
}

extern void excep_general_protection();
void handle_general_protection(uint32_t esp, uint32_t nr) {
   handle_trap("general protection fault", esp, nr);
}

extern void intr_unhandled();
void handle_unhandled(void) {
   handle_trap("unhandled interrupt", 0, 0);
   halt();
}

void set_idt_gate(unsigned num, void* handler_ptr, uint8_t type, uint8_t dpl) {
   const uint32_t handler = (uint32_t)handler_ptr;
   uint64_t desc = 0;
   desc |= handler & 0xffff;
   desc |= 0x08ull                     << 16;
   desc |= (uint64_t)(type & 15)       << 40;
   desc |= (uint64_t)(dpl & 3)         << 45;
   desc |= 1ull                        << 47;
   desc |= (uint64_t)(handler >> 16)   << 48;
   idt[num] = desc;
}

extern void intr_timer();
extern void intr_keyboard();
extern void intr_mouse();

void init_traps(void) {
   set_trap_gate(0x00, &excep_divide_error);
   set_trap_gate(0x01, &excep_debug);
   set_trap_gate(0x02, &excep_nmi);
   set_syst_gate(0x03, &excep_int3);
   set_syst_gate(0x04, &excep_overflow);
   set_syst_gate(0x05, &excep_bounds);
   set_trap_gate(0x06, &excep_invalid_op);
   set_trap_gate(0x07, &excep_no_coprocessor);
   set_trap_gate(0x08, &excep_double_fault);
   set_trap_gate(0x09, &excep_coprocessor_segment_overrun);
   set_trap_gate(0x0a, &excep_invalid_TSS);
   set_trap_gate(0x0b, &excep_segment_not_present);
   set_trap_gate(0x0c, &excep_stack_segment);
   set_trap_gate(0x0d, &excep_general_protection);
   // page fault
   set_trap_gate(0x0f, &excep_reserved);
   set_trap_gate(0x10, &excep_coprocessor);
   for (unsigned i = 0x11; i < 0x20; ++i) {
      set_trap_gate(i, &excep_reserved);
   }
   for (unsigned i = 0x20; i < 0xff; ++i) {
      set_intr_gate(i, &intr_unhandled);
   }
   set_intr_gate(0x20, &intr_timer);
   set_intr_gate(0x21, &intr_keyboard);
   set_intr_gate(0x2c, &intr_mouse);
}


