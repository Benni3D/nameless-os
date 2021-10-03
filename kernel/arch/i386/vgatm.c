#include <stdint.h>
#include "kernel/errors.h"
#include "kernel/atkbd.h"
#include "kernel/tty.h"
#include "string.h"

static volatile uint16_t* vidmem;
static int width, height;
static int posx, posy;
static int tabwidth;
static uint8_t color;

static void put(int x, int y, char ch) {
   vidmem[y * width + x] = ((uint16_t)color << 8) | ch;
}

static void clear_line(int y) {
   for (int x = 0; x < width; ++x) {
      put(x, y, ' ');
   }
}

static void send(char ch) {
   switch (ch) {
   case '\a':
      break;
   case '\b':
      if (posx == 0) {
         if (posy != 0) {
            posx = width - 1;
            --posy;
         }
      } else {
         --posx;
      }
      break;
   case '\f':
      ++posy;
      break;
   case '\n':
      while (posx < width)
         put(posx++, posy, ' ');
      break;
   case '\r':
      clear_line(posy);
      posx = 0;
      break;
   case '\t':
      for (int i = tabwidth - (posx % tabwidth); i != 0; --i)
         put(posx++, posy, ' ');
      break;
   case '\v':
      ++posy;
      break;

   case '\033':
      // TODO
      break;
   default:
      put(posx++, posy, ch);
      break;
   }
   if (posx >= width) {
      posx = 0;
      ++posy;
   }
   if (posy >= height) {
      memmove((void*)vidmem, (const void*)&vidmem[width], (height - 1) * (width * 2));
      clear_line(height - 1);
      posx = 0;
      posy = height - 1;
   }
}

static void vgatm_write(struct tty_device* dev, unsigned tty) {
   (void)tty;
   while (!TTYQ_EMPTY(dev->write_q)) {
      char ch;
      TTYQ_POP(dev->write_q, ch);
      send(ch);
   }
}

static struct tty_device tty_vgatm = {
   .write = vgatm_write,
   .read = NULL,
   .async = true,
};

void vgatm_keyboard_handler(struct atkbd_keyevent ev) {
   int ch = ev.keysym;
   if (ev.state) {
      if (!TTYQ_FULL(tty_vgatm.read_q))
         TTYQ_PUSH(tty_vgatm.read_q, ch);
   }
}

void vgatm_init(void) {
   vidmem = (volatile uint16_t*)0xb8000;
   width = 80;
   height = 25;
   posx = 0;
   posy = 0;
   color = 0x07;
   memset((void*)vidmem, 0, width * height * 2);
   atkbd_keyboard_handler = vgatm_keyboard_handler;
   register_tty(&tty_vgatm);
}
