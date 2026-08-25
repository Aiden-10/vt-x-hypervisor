#include "hypervisor.h"

#include "./cpu/processor.h"
#include "./cpu/cpu.h"
#include "./common/logging.h"

#include <ntddk.h>
#include <intrin.h>

hypervisor_t g_hypervisor{};

//
// Runs the VMX startup routine on a specific logical processor.
//
// A DPC (Deferred Procedure Call) is a Windows kernel mechanism for
// scheduling a callback to run on a specific processor at DISPATCH_LEVEL.
// We use DPCs here because VMX state is local to each logical processor,
// so every CPU must execute its own VMXON and VMCS initialization.
//
// DeferredContext contains the vcpu state associated with that CPU.
//
void start_callback_dpc( _In_ struct _KDPC* Dpc, _In_opt_ PVOID DeferredContext, _In_opt_ PVOID SystemArgument1, _In_opt_ PVOID SystemArgument2)
{
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    auto* vcpu = static_cast<vcpu_t*>(DeferredContext);

    if (!vcpu)
        return;

    // Verify that Windows actually executed this DPC on the CPU
    // that owns this vcpu structure.
    PROCESSOR_NUMBER current{};
    KeGetCurrentProcessorNumberEx(&current);

    const ULONG current_index = KeGetProcessorIndexFromNumber(&current);

    if (current_index != vcpu->processor_index) 
    {
        log::error("DPC targeted CPU %lu but executed on CPU %lu\n", vcpu->processor_index, current_index);

        vcpu->start_succeeded = false;

        // Wake the initialization thread even though startup failed.
        KeSetEvent(&vcpu->start_complete, IO_NO_INCREMENT, FALSE);

        return;
    }

    // Perform VMX initialization on this logical processor.
    vcpu->start_succeeded = vcpu::start(vcpu);

    if (!vcpu->start_succeeded)
    {
        log::error("CPU %lu virtualization failed\n");
    }

    // Tell the initialization thread that this CPU has finished
    // attempting VMX startup.
    KeSetEvent(&vcpu->start_complete, IO_NO_INCREMENT, FALSE);
}

//
// Creates and queues one startup DPC for every logical processor.
//
// Each vcpu gets its own DPC and completion event. The DPC is targeted
// to the processor that owns that vCPU so that vcpu::start() executes
// on the correct CPU.
//
static bool queue_cpu_start_dpcs(hypervisor_t* hv)
{
    if (!hv || !hv->vcpus)
        return false;

    for (ULONG i = 0; i < hv->processor_count; ++i)
    {
        auto* vcpu = &hv->vcpus[i];

        PROCESSOR_NUMBER target{};

        // Convert our flat processor index into the processor
        // group/number representation used by Windows.
        const NTSTATUS processor_status = KeGetProcessorNumberFromIndex(i, &target);
        if (!NT_SUCCESS(processor_status))
        {
            log::error("KeGetProcessorNumberFromIndex(%lu) failed: %08X\n", i, processor_status);

            return false;
        }

        vcpu->processor_index = i;
        vcpu->start_succeeded = false;

        // start_complete begins unsignaled. The DPC will signal it
        // after this CPU finishes attempting VMX startup.
        KeInitializeEvent(&vcpu->start_complete, NotificationEvent, FALSE);

        // Associate this vcpu's DPC with our startup callback and
        // pass the vcpu itself as the callback context.
        KeInitializeDpc(&vcpu->start_dpc, start_callback_dpc, vcpu);

        // Force this DPC to run on the logical processor associated with this vcpu.
        const NTSTATUS target_status = KeSetTargetProcessorDpcEx(&vcpu->start_dpc, &target);
        if (!NT_SUCCESS(target_status))
        {
            log::error("KeSetTargetProcessorDpcEx(%lu) failed: %08X\n", i, target_status);

            return false;
        }

        // Queue the DPC for execution on the selected processor.
        if (!KeInsertQueueDpc(&vcpu->start_dpc, nullptr, nullptr))
        {
            log::error("KeInsertQueueDpc(%lu) failed\n", i);

            return false;
        }
    }

    return true;
}

//
// Waits until every logical processor has finished attempting VMX startup.
//
// Queueing a DPC is asynchronous, so initialize() cannot assume that
// virtualization is ready immediately after queue_cpu_start_dpcs()
// returns. Each DPC signals its own completion event when it finishes.
//
static bool wait_for_cpu_start_dpcs(hypervisor_t* hv)
{
    if (!hv || !hv->vcpus)
        return false;

    for (ULONG i = 0; i < hv->processor_count; ++i)
    {
        auto* vcpu = &hv->vcpus[i];

        // Sleep until this CPU's startup DPC signals completion.
        const NTSTATUS status = KeWaitForSingleObject(&vcpu->start_complete, Executive, KernelMode, FALSE, nullptr);
        if (!NT_SUCCESS(status))
        {
            log::error("Waiting for CPU %lu failed: %08X\n", i, status);

            return false;
        }

        // The DPC completed, but VMX startup may still have failed.
        if (!vcpu->start_succeeded)
        {
            log::error("CPU %lu failed virtualization\n", i);

            return false;
        }

        log::printf("CPU %lu startup complete\n", i);
    }

    return true;
}


bool hypervisor::initialize()
{
    auto* hv = &g_hypervisor;

    if (hv->initialized)
        return true;

    if (KeGetCurrentIrql() != PASSIVE_LEVEL)
    {
        log::error("initialize must run at PASSIVE_LEVEL\n");
        return false;
    }

    if (!cpu::supports_vmx())
    {
        log::error("VMX unsupported\n");
        return false;
    }

    if (!cpu::vmx_allowed_by_feature_control())
    {
        log::error("VMX blocked by IA32_FEATURE_CONTROL\n");
        return false;
    }

    hv->cr3 = __readcr3();

    // Allocate one vcpu for each logical processor.
    const ULONG processor_count = KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);

    if (processor_count == 0)
    {
        log::error("No active processors\n");
        return false;
    }

    if (processor_count > SIZE_MAX / sizeof(vcpu_t))
    {
        log::error("vcpu allocation size overflow\n");
        return false;
    }

    const SIZE_T bytes = static_cast<SIZE_T>(processor_count) * sizeof(vcpu_t);

    auto* vcpus = static_cast<vcpu_t*>(ExAllocatePool2(POOL_FLAG_NON_PAGED, bytes, 'vcpu'));

    if (!vcpus)
    {
        log::error("Failed allocating vCPU array\n");
        return false;
    }

    RtlZeroMemory(vcpus, bytes);

    hv->processor_count = processor_count;
    hv->vcpus = vcpus;

    ULONG allocated_count = 0;

    // Allocate VMX resources for each processor.
    for (ULONG i = 0; i < processor_count; ++i)
    {
        vcpus[i].processor_index = i;

        if (!vcpu::allocate(&vcpus[i]))
        {
            log::error("Failed allocating CPU %lu resources\n", i);

            goto failure;
        }

        ++allocated_count;
    }

    // Start VMX on each processor.
    if (!queue_cpu_start_dpcs(hv))
    {
        log::error("Failed queueing CPU startup DPCs\n");
        goto failure;
    }

    if (!wait_for_cpu_start_dpcs(hv))
    {
        log::error("Failed starting one or more CPUs\n");
        goto failure;
    }

    hv->initialized = true;

    log::printf("Virtualization initialized on %lu CPUs\n", processor_count);

    return true;

failure:

    // TODO:
    // stop_started_vcpus(hv);

    // Release resources that were successfully allocated.
    for (ULONG i = 0; i < allocated_count; ++i)
        vcpu::free(&vcpus[i]);

    ExFreePoolWithTag(vcpus, 'vcpu');

    hv->vcpus = nullptr;
    hv->processor_count = 0;
    hv->initialized = false;

    return false;
}

void hypervisor::shutdown()
{
    auto* hv = &g_hypervisor;

    if (!hv->initialized)
        return;

    log::printf("Hypervisor successfully shut down and memory freed.\n");
}

hypervisor_t* hypervisor::get()
{
    return &g_hypervisor;
}