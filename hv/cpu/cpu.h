// cpu.h

#pragma once

#include <ntddk.h>

struct vcpu_t;

namespace cpu
{
	struct CPUID
	{
		ULONG eax;
		ULONG ebx;
		ULONG ecx;
		ULONG edx;
	};

	bool supports_vmx();
	bool vmx_allowed_by_feature_control();
	bool prepare_control_registers_for_vmx();
}