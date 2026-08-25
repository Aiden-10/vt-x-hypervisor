// vmx.cpp

#include "vmx.h"
#include "vmexit.h"

#include "../common/logging.h"
#include "../memory/allocations.h"
#include "../intel/msr.h"
#include "../intel/vmcs_fields.h"

#include <ntddk.h>
#include <intrin.h>

bool vmx::allocate_vmxon_region(vcpu_t* vcpu)
{
    vcpu->vmxon_region = memory::allocate_contiguous_page();

    if (!vcpu->vmxon_region)
    {
        log::error("Failed to allocate VMXON region");
        return false;
    }

    RtlZeroMemory(vcpu->vmxon_region, PAGE_SIZE);

    const ULONG64 basic = __readmsr(intel::IA32_VMX_BASIC);

    const ULONG32 revision_id = static_cast<ULONG32>(basic & 0x7FFFFFFF);

    *reinterpret_cast<ULONG32*>(vcpu->vmxon_region) = revision_id;

    vcpu->vmxon_physical = MmGetPhysicalAddress(vcpu->vmxon_region);

    return true;
}

void vmx::free_vmxon_region(vcpu_t* vcpu)
{
    if (!vcpu || !vcpu->vmxon_region)
        return;

    memory::free_contiguous_page(vcpu->vmxon_region);

    vcpu->vmxon_region = nullptr;
    vcpu->vmxon_physical.QuadPart = 0;
}

bool vmx::enter_vmx_operation(vcpu_t* vcpu)
{
    ULONG64 physical = static_cast<ULONG64>(vcpu->vmxon_physical.QuadPart);

    const unsigned char result = __vmx_on(&physical);

    if (result != 0)
    {
        log::error("VMXON failed: %u\n",result);
        return false;
    }

    vcpu->vmx_active = true;

    return true;
}

bool vmx::leave_vmx_operation(vcpu_t* vcpu)
{
    if (!vcpu || !vcpu->vmx_active)
        return false;

    __vmx_off();

    vcpu->vmx_active = false;

    return true;
}

bool vmx::allocate_vmm_stack(vcpu_t* vcpu)
{
    if (!vcpu)
        return false;

    vcpu->vmm_stack = ExAllocatePool2(POOL_FLAG_NON_PAGED, VMM_STACK_SIZE, 'kSVH');

    if (!vcpu->vmm_stack)
        return false;

    RtlZeroMemory(vcpu->vmm_stack, VMM_STACK_SIZE);

    return true;
}

void vmx::free_vmm_stack(vcpu_t* vcpu)
{
    if (!vcpu || !vcpu->vmm_stack)
        return;

    ExFreePool(vcpu->vmm_stack);
    vcpu->vmm_stack = nullptr;
}

bool vmx::allocate_msr_bitmap(vcpu_t* vcpu)
{
    if (!vcpu)
        return false;

    vcpu->msr_bitmap = ExAllocatePool2(POOL_FLAG_NON_PAGED, PAGE_SIZE, 'RSMV');

    if (!vcpu->msr_bitmap)
        return false;

    RtlZeroMemory(vcpu->msr_bitmap, PAGE_SIZE);

    vcpu->msr_bitmap_physical = MmGetPhysicalAddress(vcpu->msr_bitmap);

    return true;
}

void vmx::free_msr_bitmap(vcpu_t* vcpu)
{
    if (!vcpu || !vcpu->msr_bitmap)
        return;

    ExFreePool(vcpu->msr_bitmap);

    vcpu-> msr_bitmap = nullptr;
    vcpu->msr_bitmap_physical.QuadPart = 0;
}

bool vmx::launch(vcpu_t* vcpu)
{
    if (!vcpu)
        return false;

    log::printf("CPU %lu before VMLAUNCH\n", vcpu->processor_index);

    const auto result = vmlaunch_asm();



    if (result != 0)
    {

        return false;
    }

    return true;
}
