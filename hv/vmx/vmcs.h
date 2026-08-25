// vmcs.h
#pragma once

struct vcpu_t;

namespace vmx
{
    bool initialize_vmcs(vcpu_t* vcpu);
	bool allocate_vmcs_region(vcpu_t* vcpu);
	void free_vmcs_region(vcpu_t* vcpu);
	bool clear_and_load_vmcs(vcpu_t* vcpu);
	bool setup_vmcs(vcpu_t* vcpu);
}