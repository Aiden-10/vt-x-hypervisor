// cpu.cpp

#include "cpu.h"

#include "../vmx/vcpu.h"
#include "../intel/msr.h"
#include "../common/logging.h"

#include <ntddk.h>
#include <intrin.h>

bool cpu::supports_vmx()
{
    int cpu_info[4]{};

    __cpuid(cpu_info, 1);

    return (static_cast<ULONG>(cpu_info[2]) & (1u << 5)) != 0;
}

bool cpu::vmx_allowed_by_feature_control()
{
    const ULONG64 feature_control = __readmsr(intel::IA32_FEATURE_CONTROL);

    constexpr ULONG64 LOCK_BIT = 1ull << 0;
    constexpr ULONG64 ENABLE_VMX_OUTSIDE_SMX = 1ull << 2;

    const bool locked = (feature_control & LOCK_BIT) != 0;

    const bool vmx_outside_smx_enabled = (feature_control & ENABLE_VMX_OUTSIDE_SMX) != 0;

    return locked && vmx_outside_smx_enabled;
}

bool cpu::prepare_control_registers_for_vmx()
{

    ULONG64 cr0 = __readcr0();
    ULONG64 cr4 = __readcr4();

    const ULONG64 cr0_fixed0 = __readmsr(intel::IA32_VMX_CR0_FIXED0);

    const ULONG64 cr0_fixed1 = __readmsr(intel::IA32_VMX_CR0_FIXED1);

    const ULONG64 cr4_fixed0 = __readmsr(intel::IA32_VMX_CR4_FIXED0);

    const ULONG64 cr4_fixed1 = __readmsr(intel::IA32_VMX_CR4_FIXED1);

    cr0 |= cr0_fixed0;
    cr0 &= cr0_fixed1;

    cr4 |= cr4_fixed0;
    cr4 &= cr4_fixed1;

    // VMXON requires CR4.VMXE = 1.
    // 0000 ... 0001 0000 0000 0000
    constexpr ULONG64 CR4_VMXE = 1ULL << 13;

    cr4 |= CR4_VMXE;

    __writecr0(cr0);
    __writecr4(cr4);

    const ULONG64 actual_cr0 = __readcr0();
    const ULONG64 actual_cr4 = __readcr4();

    const bool cr0_valid =
        ((actual_cr0 & cr0_fixed0) == cr0_fixed0) &&
        ((actual_cr0 & ~cr0_fixed1) == 0);

    const bool cr4_valid =
        ((actual_cr4 & cr4_fixed0) == cr4_fixed0) &&
        ((actual_cr4 & ~cr4_fixed1) == 0);

    if (!cr0_valid)
    {
        log::error("CR0 does not satisfy VMX fixed bits: %llx\n", actual_cr0);

        return false;
    }

    if (!cr4_valid)
    {
        log::error("CR4 does not satisfy VMX fixed bits: %llx\n", actual_cr4);

        return false;
    }

    if ((actual_cr4 & CR4_VMXE) == 0)
    {
        log::error("CR4.VMXE is not set\n");
        return false;
    }

    return true;
}
