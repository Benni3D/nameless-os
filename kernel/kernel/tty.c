#include "kernel/errors.h"
#include "kernel/util.h"
#include "kernel/tty.h"
#include "asm/port.h"

// dummy/null TTY
static void null_write(struct tty_device* dev, unsigned tty) {
   char ch;
   (void)tty;
   while (!TTYQ_EMPTY(dev->write_q)) {
      TTYQ_POP(dev->write_q, ch);
   }
}

static void null_read(struct tty_device* dev, unsigned tty) {
   const char ch = '\0';
   (void)tty;
   while (!TTYQ_FULL(dev->read_q)) {
      TTYQ_PUSH(dev->read_q, ch);
   }
}

static struct tty_device null_tty = {
   .write = null_write,
   .read = null_read,
   .async = false,
};

struct tty_device* tty_devices[32];
size_t num_tty_devices = 0;

void serial_init(void);
void vgatm_init(void);
void tty_init(void) {
   register_tty(&null_tty);
   serial_init();
   vgatm_init();
}

int register_tty(struct tty_device* dev) {
   if (num_tty_devices >= arraylen(tty_devices))
      return -EOVERFLOW;

   tty_devices[num_tty_devices] = dev;

   return (int)(num_tty_devices++);
}

int tty_write(unsigned tty, const char* buf, size_t num) {
   size_t nw = 0;

   if (tty >= num_tty_devices)
      return -ENODEV;

   struct tty_device* dev = tty_devices[tty];

   while (nw < num) {
      while (nw < num && !TTYQ_FULL(dev->write_q)) {
         const char ch = buf[nw];
         TTYQ_PUSH(dev->write_q, ch);
         ++nw;
      }
      dev->write(dev, tty);
      while (TTYQ_FULL(dev->write_q));
   }
   return (int)nw;
}

int tty_read(unsigned tty, char* buf, size_t num) {
   size_t nr = 0;

   if (tty >= num_tty_devices)
      return -ENODEV;

   struct tty_device* dev = tty_devices[tty];

   while (nr < num) {
      if (TTYQ_EMPTY(dev->read_q)) {
         if (dev->async) {
            while (TTYQ_EMPTY(dev->read_q));
         } else {
            dev->read(dev, tty);
            if (TTYQ_EMPTY(dev->read_q))
               return -EIO;
         }
      }
      while (!TTYQ_EMPTY(dev->read_q)) {
         char ch;
         TTYQ_POP(dev->read_q, ch);
         buf[nr++] = ch;
      }
   }
   return nr;
}
