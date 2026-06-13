section .text

extern exception_handler
extern keyboard_irq

global int_handler
int_handler:
    cli

    mov eax, esp
    call exception_handler
    
.L1:
    hlt
    jmp .L1

global irq_ignore
irq_ignore:
    cli

    mov al, 0x20
    mov dx, 0x20
    out dx, al

    sti
    iret

global irq_keyboard
irq_keyboard:
    cli      ; disable interrupts

    pushad
    push eax ; save registers
    push ebx
    push ecx
    push edx
    push ebp

    mov eax, esp
    call keyboard_irq

    pop ebp  ; restore registers
    pop edx
    pop ecx
    pop ebx
    pop eax
    popad

    sti ; re-enable interrupts
    iret

