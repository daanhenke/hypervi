#include "freestanding/types.h"
#include "freestanding/x86utils.h"
#include "hypervisor/common.h"
#include "hypervisor/cpu.h"
#include "hypervisor/ept.h"

bool visor_is_vmx_supported()
{
    cpuid_t cpuid_regs;
    corelog("dying now\n");

    cpuid_regs.eax = CPUID_VERSION_INFORMATION;
    corelog_hex("dying now ", reinterpret_cast<size_t>(call_cpuid));
    call_cpuid(&cpuid_regs);

    if (! cpuid_regs.vmx)
    {
        corelog("cpu/emulator doesn't support vmx!\n");
        return false;
    }

    auto vmx_basic = read_msr<ia32_vmx_basic_register>(IA32_VMX_BASIC);
    auto feature_control = read_msr<ia32_feature_control_register>(IA32_FEATURE_CONTROL);
    corelog_hex("feature control: ", feature_control.flags);
    if (
        vmx_basic.memory_type != MEMORY_TYPE_WRITE_BACK ||
        feature_control.lock_bit == 0 ||
        feature_control.enable_vmx_outside_smx == 0
    )
    {
        corelog("not all required vmx features are supported\n");
        return false;
    }

    auto ept_cap = read_msr<ia32_vmx_ept_vpid_cap_register>(IA32_VMX_EPT_VPID_CAP);
    if (
        !ept_cap.page_walk_length_4 ||
        !ept_cap.memory_type_write_back ||
        !ept_cap.invept ||
        !ept_cap.invept_single_context ||
        !ept_cap.invept_all_contexts ||
        !ept_cap.invvpid ||
        !ept_cap.invvpid_individual_address ||
        !ept_cap.invvpid_single_context ||
        !ept_cap.invvpid_all_contexts ||
        !ept_cap.invvpid_single_context_retain_globals
    )
    {
        corelog("not all required ept features are supported\n");
        return false;
    }

    return true;
}

void visor_init_core()
{
    corelog("checking for hardware support...\n");
    if (! visor_is_vmx_supported())
    {
        corelog("no support!\n");
        return;
    }

    cpu_init();
}

size_t visor_init()
{
    ept_init_mtrr();
    corelog("setting up cores...\n");
    ldr_call_on_all_cores(visor_init_core);
    return 0;
}

extern "C" size_t visor_main(hv_init_struct* init_struct)
{
    corelog("hello from hypervisor\n");

    size_t status = 0;

    status = visor_init();

    return status;
}
