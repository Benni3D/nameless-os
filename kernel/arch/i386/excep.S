.section .text

no_error_code:
xchgl %eax, (%esp)
pushl %ebx
pushl %ecx
pushl %edx
pushl %esi
pushl %edi
pushl %ebp
push %ds
push %es
push %fs
pushl $0
lea 44(%esp), %edx
pushl %edx
movl $0x10, %edx
mov %dx, %ds
mov %dx, %es
mov %dx, %fs
call *%eax
add $8, %esp
pop %fs
pop %es
pop %ds
popl %ebp
popl %edi
popl %esi
popl %edx
popl %ecx
popl %ebx
popl %eax
iret

error_code:
xchgl %eax, 4(%esp)
xchgl %ebx, 0(%esp)
pushl %ecx
pushl %edx
pushl %esi
pushl %edi
pushl %ebp
push %ds
push %es
push %fs
lea 44(%esp), %eax
pushl %eax
movl $0x10, %eax
mov %ax, %ds
mov %ax, %es
mov %ax, %fs
call *%ebx
add $8, %esp
pop %fs
pop %es
pop %ds
popl %ebp
popl %edi
popl %esi
popl %edx
popl %ecx
popl %ebx
popl %eax
iret

# Exceptions without error code

.global excep_divide_error
excep_divide_error:
pushl $handle_divide_error
jmp no_error_code

.global excep_debug
excep_debug:
pushl $handle_int3
jmp no_error_code

.global excep_nmi
excep_nmi:
pushl $handle_nmi
jmp no_error_code

.global excep_int3
excep_int3:
pushl $handle_int3
jmp no_error_code

.global excep_overflow
excep_overflow:
pushl $handle_overflow
jmp no_error_code

.global excep_bounds
excep_bounds:
pushl $handle_bounds
jmp no_error_code

.global excep_invalid_op
excep_invalid_op:
pushl $handle_invalid_op
jmp no_error_code

.global excep_no_coprocessor
excep_no_coprocessor:
pushl $handle_no_coprocessor
jmp no_error_code

.global excep_coprocessor_segment_overrun
excep_coprocessor_segment_overrun:
pushl $handle_coprocessor_segment_overrun
jmp no_error_code

.global excep_reserved
excep_reserved:
pushl $handle_reserved
jmp no_error_code

.global excep_coprocessor
excep_coprocessor:
pushl $handle_coprocessor
jmp no_error_code

.global intr_unhandled
intr_unhandled:
call handle_unhandled

# Exceptions with error code

.global excep_double_fault
excep_double_fault:
pushl $handle_double_fault
jmp error_code

.global excep_invalid_TSS
excep_invalid_TSS:
pushl $handle_invalid_TSS
jmp error_code

.global excep_segment_not_present
excep_segment_not_present:
pushl $handle_segment_not_present
jmp error_code

.global excep_stack_segment
excep_stack_segment:
pushl $handle_stack_segment
jmp error_code

.global excep_general_protection
excep_general_protection:
pushl $handle_general_protection
jmp error_code
