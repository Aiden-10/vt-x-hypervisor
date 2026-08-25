#pragma once

#include <ntddk.h>
#include "./vmx/vcpu.h"

struct hypervisor_t
{
    bool initialized;
    ULONG processor_count;
    volatile LONG start_failures;
    vcpu_t* vcpus;
    ULONG64 cr3;
};

namespace hypervisor
{
    bool initialize();

    void shutdown();

    hypervisor_t* get();
}