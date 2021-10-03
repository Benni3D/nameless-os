#include <stdbool.h>
#include <stdint.h>
#include "kernel/printk.h"
#include "kernel/atkbd.h"
#include "kernel/util.h"
#include "asm/port.h"
#include "asm/int.h"
#include "string.h"

#define ATKBD_DEBUG 0

struct atkbd atkbd;

void handle_keyboard(void) {
#if ATKBD_DEBUG
   printk("atkbd: keyboard interrupt\n");
#endif
   const struct atkbd_driver* drv = atkbd.devs[0];
   if (drv->handle) {
      drv->handle(0);
   }

   send_EOI(0x21);
}

void handle_mouse(void) {
#if ATKBD_DEBUG
   printk("atkbd: mouse interrupt\n");
#endif
   const struct atkbd_driver* drv = atkbd.devs[1];
   if (drv->handle) {
      drv->handle(1);
   }

   send_EOI(0x2C);
}

static void send_cmd(uint8_t cmd) {
#if ATKBD_DEBUG
   printk("atkbd: sending command 0x%x.\n", cmd);
#endif
   while ((inb(0x64) & 2) == 2);
   outb(0x64, cmd);
}
static void send_data(uint8_t data) {
#if ATKBD_DEBUG
   printk("atkbd: sending data 0x%x.\n", data);
#endif
   while ((inb(0x64) & 2) == 2);
   outb(0x60, data);
}

static uint8_t recv_data(void) {
   while ((inb(0x64) & 1) == 0);
   const uint8_t data = inb(0x60);
#if ATKBD_DEBUG
   printk("atkbd: received data 0x%x.\n", data);
#endif
   return data;
}

static void check_port(int port);

static bool in_setup = true;

int init_atkbd(void) {
   // TODO: detect if the 8042 is present
   bool dualchan = false;                    // if this controller has 2 channels
   in_setup = true;

   printk("atkbd: initializing the 8042.\n");
   
   send_cmd(0xAD);                           // disable first PS/2 port
   send_cmd(0xA7);                           // disable second PS/2 port

   inb(0x60);                                // flush buffer

   send_cmd(0x20);                           // request current configuration
   uint8_t cfg = recv_data();                // read configuration
   dualchan = cfg & 0x20;                    // check if this controller has a second PS/2 port
   cfg &= ~((1<<0) | (1<<1) | (1<<6));       // clear bits 0, 1, 6
   send_cmd(0x60);
   send_data(cfg);                           // write-back the configuration

   send_cmd(0xAA);                           // perform self-test
   if (recv_data() != 0x55) {
      printk("atkbd: failed to initialize AT keyboard.\n");
      return -1;
   }

   send_cmd(0x60);                           // the 8042 may have been reset,
   send_data(cfg);                           // restore the configuration

   // check if there are really 2 channels
   if (dualchan) {
      send_cmd(0xA8);                        // enable second PS/2 port
      send_cmd(0x20);                        // request configuration
      cfg = recv_data();                     // read configuration
      dualchan = !(cfg & 0x20);              // check if it is really dual-channel
      if (dualchan) {
         send_cmd(0xA7);                     // disable the second port again
         printk("atkbd: dual-channel is supported.\n");
      }
   }
   
   send_cmd(0xAB);                           // test first port
   atkbd.ports[0] = (recv_data() == 0x00);   // check if first port is usable

   if (dualchan) {
      send_cmd(0xA9);                        // test second port
      atkbd.ports[1] = (recv_data() == 0x00);// check if second port is usable
   } else {
      atkbd.ports[1] = false;
   }

   // check if any port is usable
   if (!(atkbd.ports[0] | atkbd.ports[1])) {
      printk("atkbd: no usable PS/2 ports.\n");
      return -1;
   }

   send_cmd(0x20);                           // request the configuration
   cfg = recv_data();                        // read the configuration

   if (atkbd.ports[0]) {
      printk("atkbd: port 0 is usable.\n");
      send_cmd(0xAE);                        // enable first port if usable
      cfg |= 0x01;                           // enable interrupts for first port
   }

   if (atkbd.ports[1]) {
      printk("atkbd: port 1 is usable.\n");
      send_cmd(0xA8);                        // enable second port if usable
      cfg |= 0x02;                           // enable interrupts for second port
   }
   send_cmd(0x60);                           // write-back the configuration
   send_data(cfg);

   if (atkbd.ports[0]) {
      check_port(0);
   }
   if (atkbd.ports[1]) {
      check_port(1);
   }

   sti();
   io_wait();
   cli();
   in_setup = false;
   printk("atkbd: 8042 initialization sequence complete.\n");
   return 0;
}

static void send_datap(int port, uint8_t data) {
   if (port == 1) {
      send_cmd(0xD4);                        // send next byte to port 1
   }
   send_data(data);                          // send data
}

static uint8_t send_cmdp(int port, uint8_t cmd, bool wait_ACK) {
   uint8_t resp;
   do {
      send_datap(port, cmd);
      resp = recv_data();
   } while (resp == 0xFE);
   if (resp != 0xFA && wait_ACK) {
      while (recv_data() != 0xFA);
   }
   return resp;
}

// receive data with timeout
static bool recv_data_wto(uint8_t* data, unsigned to) {
   while ((inb(0x64) & 1) == 0) {
      if (to) {
         --to;
      } else {
#if ATKBD_DEBUG
         printk("atkbd: response timed out.\n");
#endif
         return false;
      }
   }

   *data = inb(0x60);
#if ATKBD_DEBUG
   printk("atkbd: received data 0x%x.\n", *data);
#endif
   return true;
}

static void check_port(int port) {
   uint8_t resp;
   do {
      send_datap(port, 0xFF);                // reset device
      resp = recv_data();                    // receive status
      if (resp == 0xFA) {
         printk("atkbd: detected device on port %d.\n", port);
         break;
      } else if (resp == 0xFC) {
         printk("atkbd: broken device on port %d.\n", port);
         return;
      } else if (resp == 0xFE) {
         continue;
      } else {
         printk("atkbd: invalid response '0x%02x' from port %d.\n", resp, port);
         return;
      }
   } while (1);
   send_cmdp(port, 0xF5, true);
   send_cmdp(port, 0xF2, true);

   // device identification
   const unsigned timeout = 1000;
   uint8_t ident[2] = { 0xff, 0xff };
   if (recv_data_wto(&ident[0], timeout)) {
      recv_data_wto(&ident[1], timeout);
   }

   atkbd.devs[port] = &atkbd_drivers[0];
   for (size_t i = 1; i < atkbd_num_drivers; ++i) {
      const struct atkbd_driver* drv = &atkbd_drivers[i];
      if (ident[0] == drv->ident[0] && ident[1] == drv->ident[1]) {
         atkbd.devs[port] = drv;
         break;
      }
   }
   const struct atkbd_driver* drv = atkbd.devs[port];
   if (drv == &atkbd_drivers[0]) {
      printk("atkbd: failed to identify device on port %d, ident: 0x%04x.\n",
            port, (ident[0] << 8) | ident[1]);
   } else {
      printk("atkbd: identified device on port %d as '%s'.\n", port, drv->name);
   }

   if (drv->init) {
      if (drv->init(port, ident) != 0) {
         printk("atkbd: failed to initialize '%s' on port %d.\n", drv->name, port);
      }
   }

   // flush buffer
   //inb(0x60);
}

// PS/2 DEVICE DRIVERS

static int mf2_kbd_init(int port, uint8_t ident[2]) {
   (void)ident;

   if (send_cmdp(port, 0xEE, false) != 0xEE) {
      printk("atkbd: 'MF2 keyboard' on port %d didn't respond to echo.\n", port);
   }

   printk("atkbd: initializing 'MF2 keyboard'.\n");
   send_cmdp(port, 0xF4, false);

   return 0;
}
struct keyinput {
   atkbd_keycode_t kc;
   uint8_t sc[6];
   bool state;
};
static bool scan_keycode(struct keyinput* ki_out);
static struct atkbd_keyevent to_keyevent(struct keyinput);
void(*atkbd_keyboard_handler)(struct atkbd_keyevent) = NULL;
static void mf2_kbd_handle(int port) {
   (void)port;
   if (in_setup)
      return;
   const struct keyinput ki;
   if (!scan_keycode(&ki)) {
      return;
   }
   const struct atkbd_keyevent ev = to_keyevent(ki);
#if 1
   if (atkbd_keyboard_handler) {
      atkbd_keyboard_handler(ev);
   }
#else
   if (ki.state) {
      if (ev.keysym) {
         printk("%c", ev.keysym);
      } else if (ev.keycode >= 0xe0) {
         printk("atkbd: %s scancode: %02X",
               ev.keycode == 0xe1 ? "unsupported" : "unknown",
               ev.scancode[0]);
         for (int i = 1; ev.scancode[i]; ++i)
            printk(", %02X", ev.scancode[i]);
         printk("\n");
      } else {
         printk("atkbd: received keycode %02x\n", ev.keycode);
      }
   }
#endif
}

static int mouse_init(int port, uint8_t ident[2]) {
   (void)ident;
   printk("atkbd: initializing 'PS/2 mouse'.\n");
   //send_cmdp(port, 0xF4, false);

   return 0;
}
static void mouse_handle(int port) {
   (void)port;
   if (in_setup)
      return;
   uint8_t data[3];
   data[0] = recv_data();
   data[1] = recv_data();
   data[2] = recv_data();
   //printk("x: %d\ny: %d\n", (char)data[1], (char)data[2]);
}

const struct atkbd_driver atkbd_drivers[] = {
   {
      .name    = "Unknown",
      .ident   = { 0xFF, 0xFF },
      .init    = NULL,
      .handle  = NULL,
   },
   {
      .name    = "MF2 keyboard",
      .ident   = { 0xAB, 0x83 },
      .init    = mf2_kbd_init,
      .handle  = mf2_kbd_handle,
   },
   {
      .name    = "PS/2 mouse",
      .ident   = { 0x00, 0xFF },
      .init    = mouse_init,
      .handle  = mouse_handle,
   },
   {
      .name    = "Mouse w/ scroll wheel",
      .ident   = { 0x03, 0xFF },
      .init    = NULL,
      .handle  = NULL,
   },
   {
      .name    = "5-button mouse",
      .ident   = { 0x04, 0xFF },
      .init    = NULL,
      .handle  = NULL,
   },
   {
      .name    = "MF2 keyboard w/ translation",
      .ident   = { 0xAB, 0x41 },
      .init    = NULL,
      .handle  = NULL,
   },
   {
      .name    = "MF2 keyboard w/ translation",
      .ident   = { 0xAB, 0xC1 },
      .init    = NULL,
      .handle  = NULL,
   },
   {
      .name    = "ancient AT keyboard w/ translation",
      .ident   = { 0xFF, 0xFF },
      .init    = NULL,
      .handle  = NULL,
   },
};

const size_t atkbd_num_drivers = arraylen(atkbd_drivers);

// SCANCODE TO KEYCODE CONVERSION

#define K(row, col) ((atkbd_keycode_t)(((row & 7) << 5) | (col & 31)))
#define nokc 0xff

static atkbd_keycode_t map0[] = {
   /*0x00*/ K(7, 0), K(0, 9), K(7, 0), K(0, 5),
   /*0x04*/ K(0, 3), K(0, 1), K(0, 2), K(0,12),
   /*0x08*/ K(7, 0), K(0,10), K(0, 8), K(0, 6),
   /*0x0c*/ K(0, 4), K(2, 0), K(1,13), K(7, 0),
   /*0x10*/ K(7, 0), K(5, 3), K(4, 0), K(7, 0),
   /*0x14*/ K(5, 0), K(2, 1), K(1, 1), K(7, 0),
   /*0x18*/ K(7, 0), K(7, 0), K(4, 2), K(3, 2),
   /*0x1c*/ K(3, 1), K(2, 2), K(1, 2), K(7, 0),
   /*0x20*/ K(7, 0), K(4, 4), K(4, 3), K(3, 3),
   /*0x24*/ K(2, 3), K(1, 4), K(1, 3), K(7, 0),
   /*0x28*/ K(7, 0), K(5, 4), K(4, 5), K(3, 4),
   /*0x2c*/ K(2, 5), K(2, 4), K(1, 5), K(7, 0),
   /*0x30*/ K(7, 0), K(4, 7), K(4, 6), K(3, 6),
   /*0x34*/ K(3, 5), K(2, 6), K(1, 6), K(7, 0),
   /*0x38*/ K(7, 0), K(7, 0), K(4, 8), K(3, 7),
   /*0x3c*/ K(2, 7), K(1, 7), K(1, 8), K(7, 0),
   /*0x40*/ K(7, 0), K(4, 9), K(3, 8), K(2, 8),
   /*0x44*/ K(2, 9), K(1,10), K(1, 9), K(7, 0),
   /*0x48*/ K(7, 0), K(4,10), K(4,11), K(3, 9),
   /*0x4c*/ K(3,10), K(2,10), K(1,11), K(7, 0),
   /*0x50*/ K(7, 0), K(7, 0), K(3,11), K(7, 0),
   /*0x54*/ K(2,11), K(1,12), K(7, 0), K(7, 0),
   /*0x58*/ K(3, 0), K(4,12), K(2,13), K(2,12),
   /*0x5c*/ K(7, 0), K(3,12), K(7, 0), K(7, 0),
   /*0x60*/ K(7, 0), K(4, 1), K(7, 0), K(7, 0),
   /*0x64*/ K(7, 0), K(7, 0), K(1,13), K(7, 0),
   /*0x68*/ K(7, 0), K(4,14), K(7, 0), K(3,14),
   /*0x6c*/ K(2,14), K(7, 0), K(7, 0), K(7, 0),
   /*0x70*/ K(5,12), K(5,13), K(4,15), K(3,15),
   /*0x74*/ K(3,16), K(2,15), K(0, 0), K(1,14),
   /*0x78*/ K(0,11), K(2,17), K(4,16), K(1,17),
   /*0x7c*/ K(1,16), K(2,16), K(7, 1), K(7, 0),
   /*0x80*/ K(7, 0), K(7, 0), K(7, 0), K(0, 7),
};

static atkbd_keycode_t map_e0[] = {
   /*0x00*/ K(7, 0), K(7, 0), K(7, 0), K(7, 0),
   /*0x04*/ K(7, 0), K(7, 0), K(7, 0), K(7, 0),
   /*0x08*/ K(7, 0), K(7, 0), K(7, 0), K(7, 0),
   /*0x0c*/ K(7, 0), K(7, 0), K(7, 0), K(7, 0),
   /*0x10*/ K(7, 1), K(5, 5), K(7, 0), K(7, 0),
   /*0x14*/ K(5, 8), K(7, 1), K(7, 0), K(7, 0),
   /*0x18*/ K(7, 1), K(7, 0), K(7, 0), K(7, 0),
   /*0x1c*/ K(7, 0), K(7, 0), K(7, 0), K(5, 2),
   /*0x20*/ K(7, 1), K(7, 1), K(7, 0), K(7, 1),
   /*0x24*/ K(7, 0), K(7, 0), K(7, 0), K(5, 7),
   /*0x28*/ K(7, 1), K(7, 0), K(7, 0), K(7, 1),
   /*0x2c*/ K(7, 0), K(7, 0), K(7, 0), K(7, 1),
   /*0x30*/ K(7, 1), K(7, 0), K(7, 1), K(7, 0),
   /*0x34*/ K(7, 1), K(7, 0), K(7, 0), K(7, 1),
   /*0x38*/ K(7, 1), K(7, 0), K(7, 1), K(7, 1),
   /*0x3c*/ K(7, 0), K(7, 0), K(7, 0), K(7, 1),
   /*0x40*/ K(7, 1), K(7, 0), K(7, 0), K(7, 0),
   /*0x44*/ K(7, 0), K(7, 0), K(7, 0), K(7, 0),
   /*0x48*/ K(7, 1), K(7, 0), K(1,15), K(7, 0),
   /*0x4c*/ K(7, 0), K(7, 1), K(7, 0), K(7, 0),
   /*0x50*/ K(7, 1), K(7, 0), K(7, 0), K(7, 0),
   /*0x54*/ K(7, 0), K(7, 0), K(7, 0), K(7, 0),
   /*0x58*/ K(7, 0), K(7, 0), K(4,17), K(7, 0),
   /*0x5c*/ K(7, 0), K(7, 0), K(7, 1), K(7, 0),
   /*0x60*/ K(7, 0), K(7, 0), K(7, 0), K(7, 0),
   /*0x64*/ K(7, 0), K(7, 0), K(7, 0), K(7, 0),
   /*0x68*/ K(7, 0), K(0,17), K(7, 0), K(5, 9),
   /*0x6c*/ K(0,16), K(7, 0), K(7, 0), K(7, 0),
   /*0x70*/ K(0,14), K(0,15), K(5,10), K(7, 0),
   /*0x74*/ K(5,11), K(4,13), K(7, 0), K(7, 0),
   /*0x78*/ K(7, 0), K(7, 0), K(0,19), K(7, 0),
   /*0x78*/ K(7, 0), K(0,18), K(7, 0), K(7, 0),
};

static bool scan_keycode(struct keyinput* ki) {
   memset(ki, 0, sizeof(ki));
   bool e0 = false, f0 = false;
   uint8_t* p = ki->sc;
   uint8_t b;
   if (!recv_data_wto(&b, 1000)) {
      return false;
   }
   *p++ = b;

   if (b == 0xE0) {
      b = recv_data();
      *p++ = b;
      e0 = true;
   }

   if (b == 0xF0) {
      b = recv_data();
      *p++ = b;
      f0 = true;
   }

   ki->kc = K(7, 2);
   ki->state = !f0;

   if (e0) {
      if (f0 && b == 0x7C) {
         b = recv_data();
         *p++ = b;
         if (b == 0xE0) {
            b = recv_data();
            *p++ = b;
            if (b == 0xF0) {
               b = recv_data();
               *p++ = b;
               if (b == 0x12) {
                  ki->kc = K(0, 13);
               }
            }
         }
      } else if (b == 0x12) {
         b = recv_data();
         *p++ = b;
         if (b == 0xE0) {
            b = recv_data();
            *p++ = b;
            if (b == 0x7C) {
               ki->kc = K(0, 13);
            }
         }
      } else if (b == 0x2A) {
         b = recv_data();
         *p++ = b;
         if (b == 0xE0) {
            b = recv_data();
            *p++ = b;
            if (b == 0x37) {
               ki->kc = K(0, 17);
            }
         }
      } else if (b < arraylen(map_e0)) {
         ki->kc = map_e0[b];
      }
   } else {
      if (b < arraylen(map0)) {
         ki->kc = map0[b];
      }
   }

   return true;
}


// KEYCODE TO KEYEVENT CONVERSION
#define KEY_LSHIFT   K(4, 0)
#define KEY_LCTRL    K(5, 0)
#define KEY_LALT     K(5, 3)
#define KEY_LGUI     K(5, 2)
#define KEY_RSHIFT   K(4,12)
#define KEY_RCTRL    K(5, 8)
#define KEY_RALT     K(5, 5)
#define KEY_RGUI     K(5, 7)
#define KEY_CAPSLOCK K(3, 0)
#define KEY_NUMLOCK  K(1,14)

static bool keys[256] = { false };
static bool capslock = false;
static bool numlock = false;

bool atkbd_is_pressed(atkbd_keycode_t kc) {
   return keys[kc];
}

inline static atkbd_mod_t get_mod(void) {
   return atkbd_is_pressed(KEY_LSHIFT)
      | (atkbd_is_pressed(KEY_LCTRL)   << 1)
      | (atkbd_is_pressed(KEY_LALT)    << 2)
      | (atkbd_is_pressed(KEY_LGUI)    << 3)
      | (atkbd_is_pressed(KEY_RSHIFT)  << 4)
      | (atkbd_is_pressed(KEY_RCTRL)   << 5)
      | (atkbd_is_pressed(KEY_RALT)    << 6)
      | (atkbd_is_pressed(KEY_RGUI)    << 7);
}

static struct atkbd_keyevent to_keyevent(struct keyinput ki) {
   struct atkbd_keyevent ev;
   memcpy(ev.scancode, ki.sc, 6);
   ev.keycode = ki.kc;
   ev.mod = get_mod();
   ev.state = ki.state;
   if (ev.state) {
      if (ev.keycode == KEY_CAPSLOCK) {
         capslock = !capslock;
         //printk("capslock: %d\n", capslock);
      } else if (ev.keycode == KEY_NUMLOCK) {
         numlock = !numlock;
         //printk("numlock: %d\n", numlock);
      }
   }

   int tmp = (*atkbd_keymap)[((ev.mod & 1) ^ capslock) | (((ev.mod >> 6) & 1) << 1)][ev.keycode];
   if ((tmp & 0x100) && !numlock) {
      tmp = '\0';
   }
   ev.keysym = tmp;
   keys[ev.keycode] = ev.state;
   return ev;
}

// DEFAULT KEYMAP (en_US)

static atkbd_keymap_t def_keymap = {
   {
      [K(1, 1)] = '1',
      [K(1, 2)] = '2',
      [K(1, 3)] = '3',
      [K(1, 4)] = '4',
      [K(1, 5)] = '5',
      [K(1, 6)] = '6',
      [K(1, 7)] = '7',
      [K(1, 8)] = '8',
      [K(1, 9)] = '9',
      [K(1,10)] = '0',
      [K(1,11)] = '-',
      [K(1,12)] = '=',
      [K(1,13)] = '\b',
      [K(1,15)] = '/',
      [K(1,16)] = '*',
      [K(1,17)] = '-',
      [K(2, 1)] = 'q',
      [K(2, 2)] = 'w',
      [K(2, 3)] = 'e',
      [K(2, 4)] = 'r',
      [K(2, 5)] = 't',
      [K(2, 6)] = 'y',
      [K(2, 7)] = 'u',
      [K(2, 8)] = 'i',
      [K(2, 9)] = 'o',
      [K(2,10)] = 'p',
      [K(2,11)] = '[',
      [K(2,12)] = ']',
      [K(2,13)] = '\n',
      [K(2,14)] = 0x100 | '7',
      [K(2,15)] = 0x100 | '8',
      [K(2,16)] = 0x100 | '9',
      [K(2,17)] = '+',
      [K(3, 1)] = 'a',
      [K(3, 2)] = 's',
      [K(3, 3)] = 'd',
      [K(3, 4)] = 'f',
      [K(3, 5)] = 'g',
      [K(3, 6)] = 'h',
      [K(3, 7)] = 'j',
      [K(3, 8)] = 'k',
      [K(3, 9)] = 'l',
      [K(3,10)] = ';',
      [K(3,11)] = '\'',
      [K(3,12)] = '\\',
      [K(3,14)] = 0x100 | '4',
      [K(3,15)] = 0x100 | '5',
      [K(3,16)] = 0x100 | '6',
      [K(4, 2)] = 'z',
      [K(4, 3)] = 'x',
      [K(4, 4)] = 'c',
      [K(4, 5)] = 'v',
      [K(4, 6)] = 'b',
      [K(4, 7)] = 'n',
      [K(4, 8)] = 'm',
      [K(4, 9)] = ',',
      [K(4,10)] = '.',
      [K(4,11)] = '/',
      [K(4,14)] = 0x100 | '1',
      [K(4,15)] = 0x100 | '2',
      [K(4,16)] = 0x100 | '3',
      [K(4,17)] = '\n',
      [K(5, 4)] = ' ',
      [K(5,12)] = '0',
   },
   {
      [K(1, 1)] = '!',
      [K(1, 2)] = '@',
      [K(1, 3)] = '#',
      [K(1, 4)] = '$',
      [K(1, 5)] = '%',
      [K(1, 6)] = '^',
      [K(1, 7)] = '&',
      [K(1, 8)] = '*',
      [K(1, 9)] = '(',
      [K(1,10)] = ')',
      [K(1,11)] = '_',
      [K(1,12)] = '+',
      [K(1,13)] = '\b',
      [K(1,15)] = '/',
      [K(1,16)] = '*',
      [K(1,17)] = '-',
      [K(2, 1)] = 'Q',
      [K(2, 2)] = 'W',
      [K(2, 3)] = 'E',
      [K(2, 4)] = 'R',
      [K(2, 5)] = 'T',
      [K(2, 6)] = 'Y',
      [K(2, 7)] = 'U',
      [K(2, 8)] = 'I',
      [K(2, 9)] = 'O',
      [K(2,10)] = 'P',
      [K(2,11)] = '{',
      [K(2,12)] = '}',
      [K(2,13)] = '\n',
      [K(2,14)] = 0x100 | '7',
      [K(2,15)] = 0x100 | '8',
      [K(2,16)] = 0x100 | '9',
      [K(2,17)] = '+',
      [K(3, 1)] = 'A',
      [K(3, 2)] = 'S',
      [K(3, 3)] = 'D',
      [K(3, 4)] = 'F',
      [K(3, 5)] = 'G',
      [K(3, 6)] = 'H',
      [K(3, 7)] = 'J',
      [K(3, 8)] = 'K',
      [K(3, 9)] = 'L',
      [K(3,10)] = ':',
      [K(3,11)] = '\"',
      [K(3,12)] = '|',
      [K(3,14)] = 0x100 | '4',
      [K(3,15)] = 0x100 | '5',
      [K(3,16)] = 0x100 | '6',
      [K(4, 2)] = 'Z',
      [K(4, 3)] = 'X',
      [K(4, 4)] = 'C',
      [K(4, 5)] = 'V',
      [K(4, 6)] = 'B',
      [K(4, 7)] = 'N',
      [K(4, 8)] = 'M',
      [K(4, 9)] = '<',
      [K(4,10)] = '>',
      [K(4,11)] = '?',
      [K(4,14)] = 0x100 | '1',
      [K(4,15)] = 0x100 | '2',
      [K(4,16)] = 0x100 | '3',
      [K(4,17)] = '\n',
      [K(5, 4)] = ' ',
      [K(5,12)] = '0',
   },
};


const atkbd_keymap_t* atkbd_keymap = &def_keymap;
