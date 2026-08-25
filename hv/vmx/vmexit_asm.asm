EXTERN vmexit_handler : PROC

.code

PUBLIC vmexit_entry

vmexit_entry PROC

    ; Save guest GPRs
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; guest_registers_t*
    mov rcx, rsp

    ; 20h shadow space + 8-byte alignment
    sub rsp, 28h

    call vmexit_handler

    test al, al
    jz vmexit_fail

    add rsp, 28h

    ; Restore guest GPRs
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax

    vmresume

    ; VMRESUME failed
    int 3

vmresume_fail:
    pause
    jmp vmresume_fail

vmexit_fail:
    add rsp, 28h
    int 3

vmexit_fail_hang:
    pause
    jmp vmexit_fail_hang

vmexit_entry ENDP

END