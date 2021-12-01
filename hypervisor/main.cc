#include "freestanding/types.h"
#include "freestanding/x86utils.h"
#include "hypervisor/common.h"
#include "hypervisor/cpu.h"

bool visor_is_vmx_supported()
{
    cpuid_t cpuid_regs;

    cpuid_regs.eax = CPUID_VERSION_INFORMATION;
    call_cpuid(&cpuid_regs);

    if (! cpuid_regs.vmx)
    {
        corelog("cpu/emulator doesn't support vmx!\n");
        return false;
    }

    auto vmx_basic = read_msr<ia32_vmx_basic_register>(IA32_VMX_BASIC);
    auto feature_control = read_msr<ia32_feature_control_register>(IA32_FEATURE_CONTROL);
    corelog_hex("feature control: ", feature_control.enable_vmx_inside_smx);
    if (
        vmx_basic.memory_type != MEMORY_TYPE_WRITE_BACK ||
        ! feature_control.lock_bit ||
        ! feature_control.enable_vmx_outside_smx
    )
    {
        corelog("not all required vmx features are supported\n");
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
    ldr_log("setting up cores...\n");
    ldr_call_on_all_cores(visor_init_core);
    return 0;
}

extern "C" size_t visor_main(hv_init_struct* init_struct)
{
    ldr_log("hello from hypervisor\n");

    size_t status = 0;

    status = visor_init();

    return status;
}
