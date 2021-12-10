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

    cpu_vmxcheck();
}
