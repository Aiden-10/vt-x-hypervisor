// vmexit.h

#pragma once

#include <ntddk.h>

struct guest_registers_t
{
    ULONG64 r15;
    ULONG64 r14;
    ULONG64 r13;
    ULONG64 r12;
    ULONG64 r11;
    ULONG64 r10;
    ULONG64 r9;
    ULONG64 r8;

    ULONG64 rdi;
    ULONG64 rsi;
    ULONG64 rbp;
    ULONG64 rbx;
    ULONG64 rdx;
    ULONG64 rcx;
    ULONG64 rax;
};


extern "C" void vmexit_entry();
extern "C" bool vmexit_handler(guest_registers_t* registers);
extern "C" ULONG32 vmlaunch_asm();