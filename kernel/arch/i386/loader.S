.set MAGIC, 0x1badb002
.set FLAGS, (1<<0) | (1<<1)
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bss
.align 16
__stack_bottom:
.skip 4096
__stack_top:

.section .text
.global _start
.type _start, @function
_start:
# Initialize the kernel stack
lea (__stack_top), %esp

pushl %eax
pushl %ebx

# Initialize the GDT
lgdt (gdt_ptr)

# Initialize the segment registers
ljmp $0x08, $1f
1:
mov $0x10, %dx
mov %dx, %ds
mov %dx, %es
mov %dx, %fs
mov %dx, %gs

# Load the IDT
lidt (idt_ptr)

# Initialize the PIC
call init_pic

# Call the global constructors
call _init

# Call the kernel "main"
call kernel_entry

.global halt
halt:
cli
hlt
jmp halt

.size _start, . - _start


# Initialize the PIC
.type init_pic, @function
init_pic:
# saves the mask of the PIC1 to dl
inb $0x21, %al
movb %al, %dl

# saves the mask of PIC2 to dh
inb $0xA1, %al
movb %al, %dh

# starts the initialization sequence (cascade mode)
movb $0x11, %al
outb %al, $0x20
call io_wait
movb $0x11, %al
outb %al, $0xA0
call io_wait

# sets the offset of PIC1 to 0x20
movb $0x20, %al
outb %al, $0x21
call io_wait

# sets the offset of PIC2 to 0x28
movb $0x28, %al
outb %al, $0xA1
call io_wait

# tells the PIC1 that there is PIC2
movb $0x04, %al
outb %al, $0x21
call io_wait

# tells PIC2 it's cascade identity
movb $0x02, %al
outb %al, $0xA1
call io_wait

movb $0x01, %al
outb %al, $0x21
call io_wait

movb $0x01, %al
outb %al, $0xA1
call io_wait

# restore mask of PIC1
movb %dl, %al
outb %al, $0x21
call io_wait

# restore mask of PIC2
movb %dh, %al
outb %al, $0xA1
call io_wait

ret
.size init_pic, . - init_pic

.type io_wait, @function
io_wait:
xorb %al, %al
outb %al, $0x80
ret
.size io_wait, . - io_wait

.section .rodata
gdt:
.quad 0x0000000000000000
.quad 0x00CF9A000000FFFF
.quad 0x00CF92000000FFFF
.quad 0x00CFFA000000FFFF
.quad 0x00CFF2000000FFFF
gdt_end:

.global idt
.align 16
idt:
.skip (256 * 8)
idt_end:

gdt_ptr:
.short (gdt_end - gdt) - 1
.long gdt

idt_ptr:
.short (idt_end - idt) - 1
.long idt
