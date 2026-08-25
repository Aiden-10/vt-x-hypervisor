// vcpu.h
#pragma once

#include <ntddk.h>

constexpr SIZE_T VMM_STACK_SIZE = PAGE_SIZE * 4;

struct vcpu_t
{
    ULONG processor_index = 0;

    KDPC start_dpc{};
    KEVENT start_complete{};
    bool start_succeeded = false;

    PVOID vmxon_region = nullptr;
    PHYSICAL_ADDRESS vmxon_physical{};

    PVOID vmcs_region = nullptr;
    PHYSICAL_ADDRESS vmcs_physical{};

    PVOID vmm_stack = nullptr;

    PVOID msr_bitmap = nullptr;
    PHYSICAL_ADDRESS msr_bitmap_physical{};

    bool vmx_active = false;
    bool launched = false;
};

namespace vcpu
{
    bool allocate(vcpu_t* vcpu);
    void free(vcpu_t* vcpu);

    bool start(vcpu_t* vcpu);
    void stop(vcpu_t* vcpu);
}