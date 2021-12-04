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
    }
}
