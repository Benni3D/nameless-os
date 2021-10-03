ARCH = i386
CFLAGS ?= -Og -g
CPPFAGS ?=
LDFLAGS ?=
LIBS ?= 
QEMUFLAGS ?= -cpu 486 -accel kvm

CFLAGS := $(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS := $(CPPFLAGS) -Iinclude
LDFLAGS := $(LDFLAGS)
LIBS := $(LIBS) -nostdlib -lgcc

ARCHDIR = arch/$(ARCH)

include $(ARCHDIR)/config.mk

CFLAGS	:= $(CFLAGS) $(KERNEL_ARCH_CFLAGS)
CPPFLAGS := $(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS 	:= $(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS		:= $(LIBS) $(KERNEL_ARCH_LIBS)

KERNEL_OBJS =				\
	$(KERNEL_ARCH_OBJS)	\
	kernel/kernel.o		\
	kernel/tty.o			\
	kernel/printk.o		\
	kernel/errors.o		\
	fs/tarfs.o				\
	lib/string.o			\
	lib/snprintf.o

OBJS =							\
	$(ARCHDIR)/crti.o			\
	$(ARCHDIR)/crtbegin.o	\
	$(KERNEL_OBJS)				\
	$(ARCHDIR)/crtend.o		\
	$(ARCHDIR)/crtn.o

LINK_LIST =						\
	$(LDFLAGS)					\
	$(ARCHDIR)/crti.o			\
	$(ARCHDIR)/crtbegin.o	\
	$(KERNEL_OBJS)				\
	$(LIBS)						\
	$(ARCHDIR)/crtend.o		\
	$(ARCHDIR)/crtn.o		

.PHONY: all clean qemu
.SUFFIXES: .o .c .S

all: kernel.elf

qemu: kernel.elf
	$(QEMU) $(QEMUFLAGS) -kernel $< -serial stdio

$(ARCHDIR)/crtbegin.o $(ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

kernel.elf: $(ARCHDIR)/linker.ld $(OBJS)
	$(CC) -o $@ -T $< $(CFLAGS) $(LINK_LIST)

.c.o:
	$(CC) -c -o $@ $< -std=gnu99 $(CFLAGS) $(CPPFLAGS)

.S.o:
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

clean:
	rm -f kernel.elf
	rm -f $(OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d

-include $(OBJS:.o=.d)
