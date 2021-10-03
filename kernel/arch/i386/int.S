.section .text


.global intr_timer
intr_timer:
pushal
push %ds
push %es
push %fs
push %gs
movl $0x10, %eax
mov %ax, %ds
mov %ax, %es
mov %ax, %fs
mov %ax, %gs
movl %esp, %eax
pushl %eax

call handle_timer
add $4, %esp

pop %gs
pop %fs
pop %es
pop %ds
popa
iret

.global intr_keyboard
intr_keyboard:
pushal
push %ds
push %es
push %fs
push %gs
movl $0x10, %eax
mov %ax, %ds
mov %ax, %es
mov %ax, %fs
mov %ax, %gs

call handle_keyboard

pop %gs
pop %fs
pop %es
pop %fs
popal
iret

.global intr_mouse
intr_mouse:
pushal
push %ds
push %es
push %fs
push %gs
movl $0x10, %eax
mov %ax, %ds
mov %ax, %es
mov %ax, %fs
mov %ax, %gs

call handle_mouse

pop %gs
pop %fs
pop %es
pop %fs
popal
iret
