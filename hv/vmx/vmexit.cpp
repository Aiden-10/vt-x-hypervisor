#include "vmexit.h"

#include "../intel/vmcs_fields.h"
#include "../intel/vmexit_reasons.h"
#include "../common/logging.h"

#include <intrin.h>

bool advance_guest_rip()
{
    size_t guest_rip = 0;
    size_t instruction_length = 0;

    if (__vmx_vmread(intel::GUEST_RIP, &guest_rip) != 0)
        return false;

    if (__vmx_vmread(intel::VM_EXIT_INSTRUCTION_LEN, &instruction_length) != 0)
    {
        return false;
    }

    if (instruction_length == 0)
        return false;

    return __vmx_vmwrite(intel::GUEST_RIP, guest_rip + instruction_length) == 0;
}

extern "C"
bool vmexit_handler(guest_registers_t* registers)
{

    if (!registers)
        return false;

    size_t raw_reason = 0;
    size_t qualification = 0;
    size_t guest_rip = 0;
    size_t instruction_length = 0;

    if (__vmx_vmread(intel::VM_EXIT_REASON, &raw_reason) != 0)
    {
        return false;
    }

    if (__vmx_vmread(intel::EXIT_QUALIFICATION, &qualification) != 0)
    {
        return false;
    }

    if (__vmx_vmread(intel::GUEST_RIP, &guest_rip) != 0)
    {
        return false;
    }

    __vmx_vmread(intel::VM_EXIT_INSTRUCTION_LEN, &instruction_length);

    const ULONG32 reason = static_cast<ULONG32>(raw_reason & 0xFFFF);

    const bool entry_failure = (raw_reason & 0x80000000ull) != 0;

    switch (reason)
    {
    case 10: // CPUID
    {
        int cpu_info[4]{};

        __cpuidex(
            cpu_info,
            static_cast<int>(registers->rax),
            static_cast<int>(registers->rcx)
        );

        registers->rax = static_cast<ULONG32>(cpu_info[0]);
        registers->rbx = static_cast<ULONG32>(cpu_info[1]);
        registers->rcx = static_cast<ULONG32>(cpu_info[2]);
        registers->rdx = static_cast<ULONG32>(cpu_info[3]);

        return advance_guest_rip();
    }
    case 12: // HLT
    {
        return advance_guest_rip();
    }
    default:
        log::printf("UNKNOWN VMEXIT raw=%llx reason=%lu entry_failure=%u\n", raw_reason, reason, entry_failure);
        DbgBreakPoint();
        break;
    }
    return true;
}