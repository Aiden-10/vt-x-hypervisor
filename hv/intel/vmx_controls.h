// vmx_controls.h

#pragma once

#include <ntddk.h>

namespace intel
{
    // Primary processor-based controls
    constexpr ULONG CPU_BASED_VIRTUAL_INTR_PENDING = 1u << 2;
    constexpr ULONG CPU_BASED_USE_TSC_OFFSETING = 1u << 3;
    constexpr ULONG CPU_BASED_HLT_EXITING = 1u << 7;
    constexpr ULONG CPU_BASED_INVLPG_EXITING = 1u << 9;
    constexpr ULONG CPU_BASED_MWAIT_EXITING = 1u << 10;
    constexpr ULONG CPU_BASED_RDPMC_EXITING = 1u << 11;
    constexpr ULONG CPU_BASED_RDTSC_EXITING = 1u << 12;
    constexpr ULONG CPU_BASED_CR3_LOAD_EXITING = 1u << 15;
    constexpr ULONG CPU_BASED_CR3_STORE_EXITING = 1u << 16;
    constexpr ULONG CPU_BASED_CR8_LOAD_EXITING = 1u << 19;
    constexpr ULONG CPU_BASED_CR8_STORE_EXITING = 1u << 20;
    constexpr ULONG CPU_BASED_TPR_SHADOW = 1u << 21;
    constexpr ULONG CPU_BASED_VIRTUAL_NMI_PENDING = 1u << 22;
    constexpr ULONG CPU_BASED_MOV_DR_EXITING = 1u << 23;
    constexpr ULONG CPU_BASED_UNCOND_IO_EXITING = 1u << 24;
    constexpr ULONG CPU_BASED_ACTIVATE_IO_BITMAP = 1u << 25;
    constexpr ULONG CPU_BASED_MONITOR_TRAP_FLAG = 1u << 27;
    constexpr ULONG CPU_BASED_ACTIVATE_MSR_BITMAP = 1u << 28;
    constexpr ULONG CPU_BASED_MONITOR_EXITING = 1u << 29;
    constexpr ULONG CPU_BASED_PAUSE_EXITING = 1u << 30;
    constexpr ULONG CPU_BASED_ACTIVATE_SECONDARY_CONTROLS = 1u << 31;

    // Secondary processor-based controls
    constexpr ULONG SECONDARY_ENABLE_EPT = 1u << 1;
    constexpr ULONG SECONDARY_ENABLE_RDTSCP = 1u << 3;
    constexpr ULONG SECONDARY_ENABLE_VPID = 1u << 5;
    constexpr ULONG SECONDARY_UNRESTRICTED_GUEST = 1u << 7;
    constexpr ULONG SECONDARY_ENABLE_INVPCID = 1u << 12;
    constexpr ULONG SECONDARY_ENABLE_VMFUNC = 1u << 13;

    // VM-entry controls
    constexpr ULONG VM_ENTRY_IA32E_MODE = 1u << 9;
    constexpr ULONG VM_ENTRY_SMM = 1u << 10;
    constexpr ULONG VM_ENTRY_DEACT_DUAL_MONITOR = 1u << 11;
    constexpr ULONG VM_ENTRY_LOAD_GUEST_PAT = 1u << 14;
    constexpr ULONG VM_ENTRY_LOAD_IA32_EFER = 1u << 15;

    // VM-exit controls
    constexpr ULONG VM_EXIT_HOST_ADDRESS_SPACE_SIZE = 1u << 9;
    constexpr ULONG VM_EXIT_ACK_INTR_ON_EXIT = 1u << 15;
    constexpr ULONG VM_EXIT_SAVE_GUEST_PAT = 1u << 18;
    constexpr ULONG VM_EXIT_LOAD_HOST_PAT = 1u << 19;
    constexpr ULONG VM_EXIT_SAVE_IA32_EFER = 1u << 20;
    constexpr ULONG VM_EXIT_LOAD_IA32_EFER = 1u << 21;

    // Pin-based controls
    constexpr ULONG PIN_BASED_EXTERNAL_INTERRUPT_EXITING = 1u << 0;
    constexpr ULONG PIN_BASED_NMI_EXITING = 1u << 3;
    constexpr ULONG PIN_BASED_VIRTUAL_NMI = 1u << 5;
    constexpr ULONG PIN_BASED_VMX_PREEMPTION_TIMER = 1u << 6;
    constexpr ULONG PIN_BASED_POSTED_INTERRUPTS = 1u << 7;
}