#ifndef FILE_KERNEL_TTY_H
#define FILE_KERNEL_TTY_H
#include <stdbool.h>
#include <stddef.h>

#define TTYQ_BUF_LEN 512

// queue used to store input/output
struct tty_queue {
   volatile size_t head;
   volatile size_t tail;
   volatile char buf[TTYQ_BUF_LEN];
};

// increment head/tail
#define TTYQ_INC(a)     ((a) = ((a) + 1) & (TTYQ_BUF_LEN - 1))

// decrement head/tail
#define TTYQ_DEC(a)     ((a) = ((a) - 1) & (TTYQ_BUF_LEN - 1))

// is tty_queue empty?
#define TTYQ_EMPTY(q)   ((q).head == (q).tail)

// how many chars are left?
#define TTYQ_LEFT(q)    (((q).tail - (q).head - 1) & (TTYQ_BUF_LEN - 1))

// get the last char
#define TTYQ_LAST(q)    ((q).buf[((q).head - 1) & (TTYQ_BUF_LEN - 1)])

// is tty_queue full?
#define TTYQ_FULL(q)    (TTYQ_LEFT(q) == 0)

// how many chars can be pushed?
#define TTYQ_CHARS(q)   (((q).head - (q).tail) & (TTYQ_BUF_LEN - 1))

// push a char to the queue
#define TTYQ_PUSH(q, c) \
   (void)({(q).buf[(q).head] = (c); TTYQ_INC((q).head); })

// pop a char from the queue
#define TTYQ_POP(q, c) \
   (void)({(c) = (q).buf[(q).tail]; TTYQ_INC((q).tail); })


// representation of a TTY device
struct tty_device {
   void(*write)(struct tty_device*, unsigned);

   // if !async
   void(*read)(struct tty_device*, unsigned);

   // is this TTY asynchronously filled?
   bool async;
   struct tty_queue write_q;
   struct tty_queue read_q;
};

extern struct tty_device* tty_devices[32];
extern size_t num_tty_devices;

void tty_init(void);
int tty_write(unsigned tty, const char* buf, size_t num);
int tty_read(unsigned tty, char* buf, size_t num);

int register_tty(struct tty_device*);

#endif /* FILE_KERNEL_TTY_H */
