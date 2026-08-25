# General CPU State
### RIP
- RIP is the instruction pointer
- Contains the address of the next instruction the cpu will execute
- Important because the Virtual Machine Control Structure (VMCS) will have a guest and host RIP

### RSP
- RSP is the stack pointer
- Points to the current top of the stack
- Also important because the VMCS will have a guest and host RSP

### RFLAGS
- RFLAGS contains CPU Status/control flags like:
	- CF = Carry Flag
	- ZF = Zero Flag
	- IF = Interrupt Flag
	- DF = Direction Flag
	- OF = Overflow Flag
- Also important because the VMCS will have a guest and host RFLAGS

### General Purpose Registers
- VMX does not automatically save all of the registers into VMCS fields
- VM-exit assembly saves them
- The exit handler will then give access to guest register state

# Control Registers
### CR0
- CR0 controls fundamental processor operating modes
- Include things like:
	- PE  Protected Mode Enable
	- PG  Paging Enable
	- WP  Write Protect
- VMX has restrictions on which CR0 bits may be 0 or 1

### CR2
- CR2 contains the virtual address responsible for most recent page fault
- Useful when debugging memory faults

### CR3
- CR3 points to the root of the current page-table hierarchy
- CR3 > PML4 > PDPT > PD > PT > Physical Page
- Important because VMCS will have a guest and host CR3

### CR4
- CR4 enables many optional architectual features
- Include things like:
	- PAE
	- PGE
	- OSXSAVE
	- SMEP
	- SMAP
	- VMXE

# GDT and IDT
These are both CPU tables, but they serve completely different purposes. Both are useful to the guest and host.

### GDT
- GDT is the Global Descriptor Table
- GDT describes segments and special structures used by the processor

### GDTR
- GDTR is the is the register that tells the CPU where the GDT is

### IDT
- IDT is the Interrupt Descriptor Table
- IDT tells the processor where to go when an interrupt or exception occurs
- Include things like:
	- #DE divide error
	- #DB debug
	- NMI
	- #BP breakpoint
	- #UD invalid opcode	
	- #PF page fault

### IDTR
- IDTR is the is the register that tells the CPU where the IDT is

# Segment Registers
### Selector
- A segment register generally contains a selector
- The selector identifies an entry in the GDT/LDT

### Base
- The base is where that segment begins

### Limit
- Defines the usable range of the segment

### Access Rights
- Describe what a segment is

### TR
- TR is the Task Register
- TR points the current Task State Segment (TSS)

### LDTR
- LDTR is the Local Descriptor Table Register
- LDTR points to the Local Descriptor Table (LDT)

### MSR
- MSR is a Model-Specific Register
- Intel exposes most capabilities through MSRs