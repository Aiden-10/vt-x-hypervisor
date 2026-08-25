.code

GUEST_RSP EQU 0681Ch
GUEST_RIP EQU 0681Eh

PUBLIC vmlaunch_asm

vmlaunch_asm PROC

    ; Guest resumes using the current stack
    mov rax, rsp
    mov rcx, GUEST_RSP
    vmwrite rcx, rax

    jc launch_fail
    jz launch_fail

    ; Guest begins here after successful VMLAUNCH
    lea rax, guest_resume
    mov rcx, GUEST_RIP
    vmwrite rcx, rax

    jc launch_fail
    jz launch_fail

    vmlaunch

    ; Reaching here means VMLAUNCH failed
launch_fail:
    mov eax, 1
    ret

guest_resume:
    xor eax, eax
    ret

vmlaunch_asm ENDP

END