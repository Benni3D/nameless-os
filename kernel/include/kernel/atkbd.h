#ifndef FILE_KERNEL_ATKBD_H
#define FILE_KERNEL_ATKBD_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// bit 0-2: row
// bit 3-7: column
typedef uint8_t atkbd_keycode_t;

// bit 0: LSHIFT
// bit 1: LCTRL
// bit 2: LALT
// bit 3: LGUI
// bit 4: RSHIFT
// bit 5: RCTRL
// bit 6: RALT / ALT GR
// bit 7: RGUI
typedef uint16_t atkbd_mod_t;

struct atkbd_driver {
   const char* name;                         // Name of the driver
   uint8_t ident[2];                         // Identification
   int(*init)(int port, uint8_t ident[2]);   // Gets called by init_atkbd() on discovery
   void(*handle)(int port);                  // Gets called by an interrupt
};

// keymaps[0]  : no modifiers
// keymaps[1]  : SHIFT
// keymaps[2]  : ALTGR
// keymaps[3]  : SHIFT ALTGR
typedef const int atkbd_keymap_t[4][256];

struct atkbd {
   bool ports[2];                            // Health of ports
   const struct atkbd_driver* devs[2];       // Drivers for the connected devices
};

struct atkbd_keyevent {
   uint8_t scancode[6];
   atkbd_keycode_t keycode;
   atkbd_mod_t mod;
   int keysym;
   bool state;
};

extern const struct atkbd_driver atkbd_drivers[];
extern const size_t atkbd_num_drivers;
extern struct atkbd atkbd;
extern const atkbd_keymap_t* atkbd_keymap;
extern void(*atkbd_keyboard_handler)(struct atkbd_keyevent);

int init_atkbd(void);

bool atkbd_is_pressed(atkbd_keycode_t);

#endif /* FILE_KERNEL_ATKBD_H */
