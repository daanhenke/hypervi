#include "hypervisor/host.h"
#include "hypervisor/common.h"
#include "hypervisor/cpu.h"

void host_vmexit(u64 rsp)
{
    vmx_vmexit_reason reason;
    reason.flags = cpu_vmxread(VMCS_EXIT_REASON);
    corelog("vmexit called!\n");
    corelog_hex("exit reason: ", reason.basic_exit_reason);

    if (reason.vm_entry_failure)
    {
        corelog("failed to launch vm! big sad :(\n");
        corelog_hex("exit qualifications: ", cpu_vmxread(VMCS_EXIT_QUALIFICATION));
        corelog_hex("int err code: ", cpu_vmxread(VMCS_VMEXIT_INSTRUCTION_INFO));
        corelog_hex("int err code: ", cpu_vmxread(VMCS_VMEXIT_INTERRUPTION_ERROR_CODE));
    }

    if (reason.basic_exit_reason == VMX_EXIT_REASON_EPT_MISCONFIGURATION)
    {
        corelog("ept misconfigured!\n");

        vmx_exit_qualification_ept_violation qualification;
        qualification.flags = cpu_vmxread(VMCS_EXIT_QUALIFICATION);

        u64 failed_address = cpu_vmxread(VMCS_EXIT_GUEST_LINEAR_ADDRESS);
        ept_pml4* pml4;
        epdpte* pdpt;
        epde* pd;
        epte* pt;
        ept_resolve_entries(reinterpret_cast<void*>(failed_address), &pml4, &pdpt, &pd, &pt);
    }

    cpu_vmxcheck();
}
