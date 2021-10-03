#ifndef FILE_ASM_PORT_H
#define FILE_ASM_PORT_H
#include <stdint.h>

inline static void outb(uint16_t port, uint8_t data) {
   __asm volatile("outb %0, %1" :: "a"(data), "Nd"(port));
}
inline static void outw(uint16_t port, uint16_t data) {
   __asm volatile("outw %0, %1" :: "a"(data), "Nd"(port));
}
inline static void outl(uint16_t port, uint32_t data) {
   __asm volatile("outl %0, %1" :: "a"(data), "Nd"(port));
}

inline static uint8_t inb(uint16_t port) {
   uint8_t data;
   __asm volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
   return data;
}
inline static uint16_t inw(uint16_t port) {
   uint16_t data;
   __asm volatile("inw %1, %0" : "=a"(data) : "Nd"(port));
   return data;
}
inline static uint32_t inl(uint32_t port) {
   uint32_t data;
   __asm volatile("inl %1, %0" : "=a"(data) : "Nd"(port));
   return data;
}

#define io_wait() outb(0x80, 0x00)

inline static void send_EOI(uint8_t n) {
   if (n >= 0x28) {
      outb(0xA0, 0x20);
   }
   outb(0x20, 0x20);
}

#endif /* FILE_ASM_PORT_H */
