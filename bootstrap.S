section .multiboot
align 4

; Multiboot header
dd 0x1BADB002                     ; magic
dd 1 | (1 << 1)                   ; length
dd -(0x1BADB002 + (1 | (1 << 1))) ; checksum

section .bss
align 16

stack_bottom:
    resb 16384
stack_top:

section .text

extern kernel_main

global _start
_start:
    mov esp, $stack_top ; setup the stack

    call kernel_main

    cli
.L1:
    hlt
    jmp .L1

