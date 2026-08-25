// msr.h

#pragma once

#include <ntddk.h>

namespace intel
{   
    constexpr ULONG IA32_FEATURE_CONTROL = 0x3A;
    constexpr ULONG IA32_VMX_BASIC = 0x480;
    constexpr ULONG IA32_VMX_PINBASED_CTLS = 0x481;
    constexpr ULONG IA32_VMX_PROCBASED_CTLS = 0x482;
    constexpr ULONG IA32_VMX_EXIT_CTLS = 0x483;
    constexpr ULONG IA32_VMX_ENTRY_CTLS = 0x484;

    constexpr ULONG IA32_VMX_CR0_FIXED0 = 0x486;
    constexpr ULONG IA32_VMX_CR0_FIXED1 = 0x487;
    constexpr ULONG IA32_VMX_CR4_FIXED0 = 0x488;
    constexpr ULONG IA32_VMX_CR4_FIXED1 = 0x489;

    constexpr ULONG IA32_VMX_PROCBASED_CTLS2 = 0x48B;

    constexpr ULONG IA32_VMX_TRUE_PINBASED_CTLS = 0x48D;
    constexpr ULONG IA32_VMX_TRUE_PROCBASED_CTLS = 0x48E;
    constexpr ULONG IA32_VMX_TRUE_EXIT_CTLS = 0x48F;
    constexpr ULONG IA32_VMX_TRUE_ENTRY_CTLS = 0x490;

    constexpr ULONG IA32_PAT = 0x277;
    constexpr ULONG IA32_DEBUGCTL = 0x1D9;

    constexpr ULONG IA32_SYSENTER_CS = 0x174;
    constexpr ULONG IA32_SYSENTER_ESP = 0x175;
    constexpr ULONG IA32_SYSENTER_EIP = 0x176;

    constexpr ULONG IA32_EFER = 0xC0000080;

    constexpr ULONG IA32_FS_BASE = 0xC0000100;
    constexpr ULONG IA32_GS_BASE = 0xC0000101;

    union ia32_feature_control_msr
    {
        ULONG64 value;

        struct
        {
            ULONG64 lock : 1;                      // Bit 0
            ULONG64 enable_vmx_inside_smx : 1;     // Bit 1
            ULONG64 enable_vmx_outside_smx : 1;    // Bit 2

            ULONG64 reserved1 : 5;                 // Bits 3-7
            ULONG64 senter_local : 7;              // Bits 8-14
            ULONG64 senter_global : 1;             // Bit 15

            ULONG64 reserved2 : 48;                // Bits 16-63
        } bits;
    };
    static_assert(sizeof(ia32_feature_control_msr) == sizeof(ULONG64));

    union ia32_vmx_basic_msr
    {
        ULONG64 value;

        struct
        {
            ULONG64 revision_id : 31;       // Bits 0-30
            ULONG64 reserved0 : 1;          // Bit 31

            ULONG64 region_size : 13;       // Bits 32-44
            ULONG64 reserved1 : 3;          // Bits 45-47

            ULONG64 phys_addr_width : 1;    // Bit 48
            ULONG64 dual_monitor : 1;       // Bit 49

            ULONG64 memory_type : 4;        // Bits 50-53
            ULONG64 vmexit_report : 1;      // Bit 54
            ULONG64 true_controls : 1;      // Bit 55

            ULONG64 reserved2 : 8;
        } bits;
    };
    static_assert(sizeof(ia32_vmx_basic_msr) == 8);
}
