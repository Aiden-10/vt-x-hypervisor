// vmcs.cpp

#include "vmcs.h"
#include "vcpu.h"
#include "vmexit.h"
#include "controls.h"

#include "../intel/msr.h"
#include "../intel/vmcs_fields.h"
#include "../intel/vmx_controls.h"
#include "../common/logging.h"
#include "../memory/allocations.h"
#include "../cpu/segments.h"
#include "../cpu/descriptors.h"
#include "../../hypervisor.h"

#include <intrin.h>


bool vmx::initialize_vmcs(vcpu_t* vcpu)
{
    if (!vcpu)
        return false;

    if (!clear_and_load_vmcs(vcpu))
        return false;

    if (!setup_vmcs(vcpu))
        return false;
    
    return true;
}

bool vmx::clear_and_load_vmcs(vcpu_t* vcpu)
{
    if (!vcpu || !vcpu->vmcs_region)
        return false;

    ULONG64 vmcs_physical_address = static_cast<ULONG64>(vcpu->vmcs_physical.QuadPart);

    // VMCLEAR
    if (__vmx_vmclear(&vmcs_physical_address) != 0)
    {
        return false;
    }

    // VMPTRLD
    if (__vmx_vmptrld(&vmcs_physical_address) != 0)
    {
        return false;
    }

    return true;
}

bool vmx::setup_vmcs(vcpu_t* vcpu)
{
    auto* hv = hypervisor::get();

    // VMCS LINK POINTER
    __vmx_vmwrite(intel::VMCS_LINK_POINTER, ~0ULL);

    // CONTROLS AND BITMAPS
    const ULONG64 vmx_basic = __readmsr(intel::IA32_VMX_BASIC);
    const bool has_true_controls = (vmx_basic & (1ull << 55)) != 0;

    const ULONG pinbased_msr = has_true_controls ? intel::IA32_VMX_TRUE_PINBASED_CTLS
        : intel::IA32_VMX_PINBASED_CTLS;

    const ULONG procbased_msr = has_true_controls ? intel::IA32_VMX_TRUE_PROCBASED_CTLS
        : intel::IA32_VMX_PROCBASED_CTLS;

    const ULONG exit_msr = has_true_controls ? intel::IA32_VMX_TRUE_EXIT_CTLS
        : intel::IA32_VMX_EXIT_CTLS;

    const ULONG entry_msr = has_true_controls ? intel::IA32_VMX_TRUE_ENTRY_CTLS
        : intel::IA32_VMX_ENTRY_CTLS;

    __vmx_vmwrite(intel::VM_EXIT_CONTROLS, controls::adjust_controls(intel::VM_EXIT_HOST_ADDRESS_SPACE_SIZE | intel::VM_EXIT_SAVE_IA32_EFER | intel::VM_EXIT_LOAD_IA32_EFER, exit_msr));

    __vmx_vmwrite(intel::VM_ENTRY_CONTROLS, controls::adjust_controls(intel::VM_ENTRY_IA32E_MODE | intel::VM_ENTRY_LOAD_IA32_EFER, entry_msr));

    __vmx_vmwrite(intel::CPU_BASED_VM_EXEC_CONTROL, controls::adjust_controls(intel::CPU_BASED_ACTIVATE_SECONDARY_CONTROLS | intel::CPU_BASED_ACTIVATE_MSR_BITMAP, procbased_msr));

    __vmx_vmwrite(intel::SECONDARY_VM_EXEC_CONTROL, controls::adjust_controls(intel::SECONDARY_ENABLE_RDTSCP | intel::SECONDARY_ENABLE_INVPCID, intel::IA32_VMX_PROCBASED_CTLS2));

    __vmx_vmwrite(intel::PIN_BASED_VM_EXEC_CONTROL, controls::adjust_controls(0, pinbased_msr));

    __vmx_vmwrite(intel::EXCEPTION_BITMAP, 0);
    __vmx_vmwrite(intel::PAGE_FAULT_ERROR_CODE_MASK, 0);
    __vmx_vmwrite(intel::PAGE_FAULT_ERROR_CODE_MATCH, 0);
    __vmx_vmwrite(intel::CR3_TARGET_COUNT, 0);

    __vmx_vmwrite(intel::GUEST_IA32_DEBUGCTL, __readmsr(intel::IA32_DEBUGCTL) & 0xFFFFFFFF);
    __vmx_vmwrite(intel::GUEST_IA32_DEBUGCTL_HIGH, __readmsr(intel::IA32_DEBUGCTL) >> 32);

    __vmx_vmwrite(intel::MSR_BITMAP, vcpu->msr_bitmap_physical.QuadPart);

    // CONTROL REGISTERS

    const ULONG64 cr0_fixed0 = __readmsr(intel::IA32_VMX_CR0_FIXED0);
    const ULONG64 cr0_fixed1 = __readmsr(intel::IA32_VMX_CR0_FIXED1);
    const ULONG64 cr4_fixed0 = __readmsr(intel::IA32_VMX_CR4_FIXED0);
    const ULONG64 cr4_fixed1 = __readmsr(intel::IA32_VMX_CR4_FIXED1);

    ULONG64 guest_cr0 = __readcr0();
    guest_cr0 &= cr0_fixed1;
    guest_cr0 |= cr0_fixed0;

    ULONG64 guest_cr4 = __readcr4();
    guest_cr4 &= cr4_fixed1;
    guest_cr4 |= cr4_fixed0;

    __vmx_vmwrite(intel::GUEST_CR0, guest_cr0);
    __vmx_vmwrite(intel::GUEST_CR4, guest_cr4);
    __vmx_vmwrite(intel::GUEST_CR3, __readcr3());

    ULONG64 host_cr0 = __readcr0();
    host_cr0 &= cr0_fixed1;
    host_cr0 |= cr0_fixed0;

    ULONG64 host_cr4 = __readcr4();
    host_cr4 &= cr4_fixed1;
    host_cr4 |= cr4_fixed0;

    __vmx_vmwrite(intel::HOST_CR0, host_cr0);
    __vmx_vmwrite(intel::HOST_CR4, host_cr4);
    __vmx_vmwrite(intel::HOST_CR3, hv->cr3);

    __vmx_vmwrite(intel::CR0_GUEST_HOST_MASK, 0);
    __vmx_vmwrite(intel::CR4_GUEST_HOST_MASK, 0);
    __vmx_vmwrite(intel::CR0_READ_SHADOW, guest_cr0);
    __vmx_vmwrite(intel::CR4_READ_SHADOW, guest_cr4);

    // GUEST STATE
    const auto gdtr = cpu::read_gdtr();
    const auto idtr = cpu::read_idtr();
    segment_state_t es, cs, ss, ds, fs, gs, ldtr, tr;
    if (!decode_segment(read_es_asm(), gdtr.base, gdtr.limit, &es)) return false;
    if (!decode_segment(read_cs_asm(), gdtr.base, gdtr.limit, &cs)) return false;
    if (!decode_segment(read_ss_asm(), gdtr.base, gdtr.limit, &ss)) return false;
    if (!decode_segment(read_ds_asm(), gdtr.base, gdtr.limit, &ds)) return false;
    if (!decode_segment(read_fs_asm(), gdtr.base, gdtr.limit, &fs)) return false;
    if (!decode_segment(read_gs_asm(), gdtr.base, gdtr.limit, &gs)) return false;
    if (!decode_system_segment(read_ldtr_asm(), gdtr.base, gdtr.limit, &ldtr)) return false;
    if (!decode_system_segment(read_tr_asm(), gdtr.base, gdtr.limit, &tr)) return false;

    __vmx_vmwrite(intel::GUEST_ES_SELECTOR, es.selector);
    __vmx_vmwrite(intel::GUEST_ES_BASE, es.base);
    __vmx_vmwrite(intel::GUEST_ES_LIMIT, es.limit);
    __vmx_vmwrite(intel::GUEST_ES_AR_BYTES, es.access_rights);

    __vmx_vmwrite(intel::GUEST_CS_SELECTOR, cs.selector);
    __vmx_vmwrite(intel::GUEST_CS_BASE, cs.base);
    __vmx_vmwrite(intel::GUEST_CS_LIMIT, cs.limit);
    __vmx_vmwrite(intel::GUEST_CS_AR_BYTES, cs.access_rights);

    __vmx_vmwrite(intel::GUEST_SS_SELECTOR, ss.selector);
    __vmx_vmwrite(intel::GUEST_SS_BASE, ss.base);
    __vmx_vmwrite(intel::GUEST_SS_LIMIT, ss.limit);
    __vmx_vmwrite(intel::GUEST_SS_AR_BYTES, ss.access_rights);

    __vmx_vmwrite(intel::GUEST_DS_SELECTOR, ds.selector);
    __vmx_vmwrite(intel::GUEST_DS_BASE, ds.base);
    __vmx_vmwrite(intel::GUEST_DS_LIMIT, ds.limit);
    __vmx_vmwrite(intel::GUEST_DS_AR_BYTES, ds.access_rights);

    __vmx_vmwrite(intel::GUEST_FS_SELECTOR, fs.selector);
    __vmx_vmwrite(intel::GUEST_FS_BASE, __readmsr(intel::IA32_FS_BASE));
    __vmx_vmwrite(intel::GUEST_FS_LIMIT, fs.limit);
    __vmx_vmwrite(intel::GUEST_FS_AR_BYTES, fs.access_rights);

    __vmx_vmwrite(intel::GUEST_GS_SELECTOR, gs.selector);
    __vmx_vmwrite(intel::GUEST_GS_BASE, __readmsr(intel::IA32_GS_BASE));
    __vmx_vmwrite(intel::GUEST_GS_LIMIT, gs.limit);
    __vmx_vmwrite(intel::GUEST_GS_AR_BYTES, gs.access_rights);

    __vmx_vmwrite(intel::GUEST_LDTR_SELECTOR, ldtr.selector);
    __vmx_vmwrite(intel::GUEST_LDTR_BASE, ldtr.base);
    __vmx_vmwrite(intel::GUEST_LDTR_LIMIT, ldtr.limit);
    __vmx_vmwrite(intel::GUEST_LDTR_AR_BYTES, ldtr.access_rights);

    __vmx_vmwrite(intel::GUEST_TR_SELECTOR, tr.selector);
    __vmx_vmwrite(intel::GUEST_TR_BASE, tr.base);
    __vmx_vmwrite(intel::GUEST_TR_LIMIT, tr.limit);
    __vmx_vmwrite(intel::GUEST_TR_AR_BYTES, tr.access_rights);

    __vmx_vmwrite(intel::GUEST_GDTR_BASE, gdtr.base);
    __vmx_vmwrite(intel::GUEST_IDTR_BASE, idtr.base);
    __vmx_vmwrite(intel::GUEST_GDTR_LIMIT, gdtr.limit);
    __vmx_vmwrite(intel::GUEST_IDTR_LIMIT, idtr.limit);

    // SYSTEM REGISTERS
    __vmx_vmwrite(intel::GUEST_RFLAGS, __readeflags());
    __vmx_vmwrite(intel::GUEST_IA32_EFER, __readmsr(intel::IA32_EFER));
    __vmx_vmwrite(intel::GUEST_DR7, 0x400);

    // MSR/DEBUG STATE
    __vmx_vmwrite(intel::GUEST_IA32_DEBUGCTL, __readmsr(intel::IA32_DEBUGCTL) & 0xFFFFFFFF);
    __vmx_vmwrite(intel::GUEST_IA32_DEBUGCTL_HIGH, __readmsr(intel::IA32_DEBUGCTL) >> 32);
    __vmx_vmwrite(intel::GUEST_SYSENTER_CS, __readmsr(intel::IA32_SYSENTER_CS));
    __vmx_vmwrite(intel::GUEST_SYSENTER_EIP, __readmsr(intel::IA32_SYSENTER_EIP));
    __vmx_vmwrite(intel::GUEST_SYSENTER_ESP, __readmsr(intel::IA32_SYSENTER_ESP));
    __vmx_vmwrite(intel::GUEST_INTERRUPTIBILITY_INFO, 0);
    __vmx_vmwrite(intel::GUEST_ACTIVITY_STATE, 0);

    // HOST STATE
    __vmx_vmwrite(intel::HOST_ES_SELECTOR, read_es_asm() & 0xF8);
    __vmx_vmwrite(intel::HOST_CS_SELECTOR, read_cs_asm() & 0xF8);
    __vmx_vmwrite(intel::HOST_SS_SELECTOR, read_ss_asm() & 0xF8);
    __vmx_vmwrite(intel::HOST_DS_SELECTOR, read_ds_asm() & 0xF8);
    __vmx_vmwrite(intel::HOST_FS_SELECTOR, read_fs_asm() & 0xF8);
    __vmx_vmwrite(intel::HOST_GS_SELECTOR, read_gs_asm() & 0xF8);
    __vmx_vmwrite(intel::HOST_TR_SELECTOR, read_tr_asm() & 0xF8);

    __vmx_vmwrite(intel::HOST_GDTR_BASE, gdtr.base);
    __vmx_vmwrite(intel::HOST_IDTR_BASE, idtr.base);
    __vmx_vmwrite(intel::HOST_FS_BASE, __readmsr(intel::IA32_FS_BASE));
    __vmx_vmwrite(intel::HOST_GS_BASE, __readmsr(intel::IA32_GS_BASE));
    __vmx_vmwrite(intel::HOST_TR_BASE, tr.base);
    __vmx_vmwrite(intel::HOST_IA32_EFER, __readmsr(intel::IA32_EFER));

    __vmx_vmwrite(intel::HOST_IA32_SYSENTER_CS, __readmsr(intel::IA32_SYSENTER_CS));
    __vmx_vmwrite(intel::HOST_IA32_SYSENTER_EIP, __readmsr(intel::IA32_SYSENTER_EIP));
    __vmx_vmwrite(intel::HOST_IA32_SYSENTER_ESP, __readmsr(intel::IA32_SYSENTER_ESP));

    size_t stack_base = reinterpret_cast<size_t>(vcpu->vmm_stack) + VMM_STACK_SIZE;
    stack_base &= ~0xFULL;

    __vmx_vmwrite(intel::HOST_RSP, stack_base);
    __vmx_vmwrite(intel::HOST_RIP, reinterpret_cast<ULONG64>(vmexit_entry));

    return true;
}

bool vmx::allocate_vmcs_region(vcpu_t* vcpu)
{
    vcpu->vmcs_region = memory::allocate_contiguous_page();

    if (!vcpu->vmcs_region)
    {
        log::error("Failed to allocate VMCS region");
        return false;
    }

    RtlZeroMemory(vcpu->vmcs_region, PAGE_SIZE);

    const ULONG64 basic = __readmsr(intel::IA32_VMX_BASIC);

    const ULONG32 revision_id = static_cast<ULONG32>(basic & 0x7FFFFFFF);

    *reinterpret_cast<ULONG32*>(vcpu->vmcs_region) = revision_id;

    vcpu->vmcs_physical = MmGetPhysicalAddress(vcpu->vmcs_region);

    return true;
}

void vmx::free_vmcs_region(vcpu_t* vcpu)
{
    if (!vcpu || !vcpu->vmcs_region)
        return;

    memory::free_contiguous_page(vcpu->vmcs_region);

    vcpu->vmcs_region = nullptr;
    vcpu->vmcs_physical.QuadPart = 0;
}
