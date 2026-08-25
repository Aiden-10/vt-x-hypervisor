.code

read_gdtr_asm PROC
    sgdt [rcx]
    ret
read_gdtr_asm ENDP

read_cs_asm PROC
    mov ax, cs
    ret
read_cs_asm ENDP

read_ss_asm PROC
    mov ax, ss
    ret
read_ss_asm ENDP

read_ds_asm PROC
    mov ax, ds
    ret
read_ds_asm ENDP

read_es_asm PROC
    mov ax, es
    ret
read_es_asm ENDP

read_fs_asm PROC
    mov ax, fs
    ret
read_fs_asm ENDP

read_gs_asm PROC
    mov ax, gs
    ret
read_gs_asm ENDP

read_tr_asm PROC
    str ax
    ret
read_tr_asm ENDP

read_ldtr_asm PROC
    sldt ax
    ret
read_ldtr_asm ENDP

END