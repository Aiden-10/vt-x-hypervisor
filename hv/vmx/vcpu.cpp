#include "vcpu.h"
#include "vmx.h"
#include "vmcs.h"

#include "../cpu/cpu.h"
#include "../common/logging.h"

#include <intrin.h>

bool vcpu::allocate(vcpu_t* vcpu)
{
    if (!vcpu)
        return false;

    if (!vmx::allocate_vmxon_region(vcpu))
        goto fail;

    if (!vmx::allocate_vmcs_region(vcpu))
        goto fail;

    if (!vmx::allocate_vmm_stack(vcpu))
        goto fail;

    if (!vmx::allocate_msr_bitmap(vcpu))
        goto fail;

    return true;

fail:
    free(vcpu);
    return false;
}

void vcpu::free(vcpu_t* vcpu)
{
    if (!vcpu)
        return;

    vmx::free_msr_bitmap(vcpu);
    vmx::free_vmm_stack(vcpu);
    vmx::free_vmcs_region(vcpu);
    vmx::free_vmxon_region(vcpu);
}

bool vcpu::start(vcpu_t* vcpu)
{
    if (!vcpu)
        return false;

    if (!cpu::prepare_control_registers_for_vmx())
        return false;

    if (!vmx::enter_vmx_operation(vcpu))
        return false;

    if (!vmx::clear_and_load_vmcs(vcpu))
        return false;

    if (!vmx::initialize_vmcs(vcpu))
        return false;

    return vmx::launch(vcpu);
}

void vcpu::stop(vcpu_t* vcpu)
{
    if (!vcpu)
        return;

    if (vcpu->vmx_active)
        vmx::leave_vmx_operation(vcpu);

    vcpu->launched = false;
}
