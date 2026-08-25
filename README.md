# VT-x Hypervisor

A lightweight Windows x86-64 hypervisor built in C++ using Intel VT-x. It runs as a kernel driver and virtualizes the currently running Windows system.

## Program Flow

1. Load the Windows kernel driver.
2. Check VT-x support and IA32_FEATURE_CONTROL.
3. Allocate per-CPU VMX resources.
4. Enter VMX operation with VMXON.
5. Configure and load the VMCS.
6. Launch Windows into VMX non-root mode with VMLAUNCH.
7. Handle VM exits and return to Windows with VMRESUME.

## VM Exits

Currently supports/intercepts:

1. CPUID
2. MSR access using VMX MSR bitmaps

Instruction-based exits can advance guest RIP using the VM-exit instruction length before resuming execution.

## Requirements

* Windows x86-64
* Intel CPU with VT-x enabled
* Visual Studio + Windows Driver Kit

## Disclaimer

This project is still in development and was created for educational and systems research purposes. It is experimental software and is not intended to be run on real hardware. Bugs may cause system crashes or data loss. Use only in a test environment or virtual machine with nested virtualization enabled, and at your own risk.
