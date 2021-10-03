#include <stdint.h>
#include "kernel/errors.h"
#include "kernel/tty.h"
#include "asm/port.h"

#define COM1 0x3f8
#define COM2 0x2f8
#define COM3 0x3e8
#define COM4 0x2e8

void serial_send(int base, char ch) {
   while ((inb(base + 5) & 0x20) == 0);
   outb(base, ch);
}
static char serial_recv(int base) {
   while ((inb(base + 5) & 1) == 0);
   return inb(base);
}


static void serial_write(struct tty_device* dev, unsigned tty) {
   (void)tty;
   while (!TTYQ_EMPTY(dev->write_q)) {
      char ch;
      TTYQ_POP(dev->write_q, ch);
      serial_send(COM1, ch);
   }
}

static void serial_read(struct tty_device* dev, unsigned tty) {
   (void)tty;
   if (TTYQ_FULL(dev->read_q))
      return;
   const char ch = serial_recv(COM1);
   TTYQ_PUSH(dev->read_q, ch);
}

static struct tty_device tty_serial = {
   .write = serial_write,
   .read = serial_read,
   .async = false,
};

static int init_port(int base, int baud) {
   const uint16_t divisor = 115200 / baud;
   outb(base + 1, 0x00);               // disable all interrupts
   outb(base + 3, 0x80);               // enable DLAB (set baud rate divisor)
   outb(base + 0, divisor & 0xff);     // set the lower byte of the divisor
   outb(base + 1, divisor >> 16);      // set the upper byte of the divisor
   outb(base + 3, 0x03);               // 8 bits, no parity, one stop bit
   outb(base + 2, 0xc7);               // enable FIFO, RTS/DSR set
   outb(base + 4, 0x0b);               // IRQs enabled, RTS/DSR set

   outb(base + 4, 0x1e);               // set in loopback mode, test the serial chip
   outb(base + 0, 0xae);               // test the serial chip

   if (inb(base + 0) != 0xae) {
      return -EIO;
   }

   outb(base + 4, 0x0f);               // set to normal mode
   return 0;
}

void serial_init(void) {
   // initialize COM1
   init_port(COM1, 9600);

   register_tty(&tty_serial);
}
