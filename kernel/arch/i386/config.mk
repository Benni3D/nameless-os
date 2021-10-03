KERNEL_ARCH_CFLAGS=
KERNEL_ARCH_CPPFLAGS=
KERNEL_ARCH_LDFLAGS=
KERNEL_ARCH_LIBS=

CROSS_COMPILE ?= i386-elf-

CC=$(CROSS_COMPILE)gcc
QEMU=qemu-system-i386

KERNEL_ARCH_OBJS =			\
	$(ARCHDIR)/loader.o		\
	$(ARCHDIR)/entry.o		\
	$(ARCHDIR)/excep.o		\
	$(ARCHDIR)/int.o			\
	$(ARCHDIR)/timer.o		\
	$(ARCHDIR)/traps.o		\
	$(ARCHDIR)/serial.o		\
	$(ARCHDIR)/vgatm.o		\
	$(ARCHDIR)/atkbd.o
