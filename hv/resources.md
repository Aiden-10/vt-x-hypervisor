# Learning Resources & Goals
### Goals
1. Check VT-x support			(done)
2. Check IA32_FEATURE_CONTROL   (done)
3. Read VMX capability MSRs     (done)
4. Prepare CR0 / CR4            (done)
5. Allocate VMXON region        (done)
6. Execute VMXON                (done)
7. Allocate VMCS                (done)
8. VMCLEAR + VMPTRLD            (done)
9. Fill minimal guest state     (done)
10. Fill minimal host state     (done)
11. Set execution controls      (done)
12. VMLAUNCH                    (done)
13. Handle one VM exit: CPUID   (done)
14. VMRESUME                    (done)
15. Make VMXOFF/shutdown work   (KINDADONE)

### Resources
1. Intel Software Developer's Manual
Official Developer Manual
https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html

2. Hypervisor From Scratch
Example Hypervisor
https://github.com/SinaKarvandi/Hypervisor-From-Scratch

3. SimpleVisor
A relatively small Intel VT-x hypervisor implementation
https://github.com/ionescu007/SimpleVisor

4. HyperPlatform
A much larger Windows hypervisor project
https://github.com/tandasat/HyperPlatform