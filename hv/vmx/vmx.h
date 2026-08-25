// vmx.h

#pragma once

#include "vcpu.h"

namespace vmx
{
	// VMXON
	bool allocate_vmxon_region(vcpu_t* vcpu);
	void free_vmxon_region(vcpu_t* vcpu);

	// ENTER VMX
	bool enter_vmx_operation(vcpu_t* vcpu);
	bool leave_vmx_operation(vcpu_t* vcpu);

	// VMM STACK
	bool allocate_vmm_stack(vcpu_t* vcpu);
	void free_vmm_stack(vcpu_t* vcpu);

	// MSR BITMAP
	bool allocate_msr_bitmap(vcpu_t* vcpu);
	void free_msr_bitmap(vcpu_t* vcpu);

	// VMM LAUNCH
	bool launch(vcpu_t* vcpu);
}

