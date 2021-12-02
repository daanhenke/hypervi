#include "hypervisor/cpu.h"
#include "hypervisor/common.h"
#include "freestanding/libc.h"

cpu_state g_cpu_states[cpu_max];

size_t cpu_alter_control_register(size_t original_value, size_t msr_id_0, size_t msr_id_1)
{
    u32 or_data = read_msr(msr_id_0);
    u32 and_data = read_msr(msr_id_1);

    original_value &= and_data;
    original_value |= or_data;

    return original_value;
}

size_t cpu_init()
{
    auto index = ldr_core_whoami();
    auto state = &g_cpu_states[index];

    // Initialize our core state
    state->core_index = index;
    state->status_code = 0;
    memset(&state->core_vmxon, 0, sizeof(vmxon));
    memset(&state->core_vmcs, 0, sizeof(vmcs));

    corelog("setting control registers\n");
    cr4 new_cr4;
    new_cr4.flags = read_cr4();
    new_cr4.vmx_enable = true;
    write_cr4(new_cr4.flags);

    // Setup the control registers that need to be changed
    write_cr0(cpu_alter_control_register(read_cr0(), IA32_VMX_CR0_FIXED0, IA32_VMX_CR0_FIXED1));
    write_cr4(cpu_alter_control_register(read_cr4(), IA32_VMX_CR4_FIXED0, IA32_VMX_CR4_FIXED1));

    cpuid_t regs;
    regs.eax = 0x80000008U;
    call_cpuid(&regs);

    auto phys_size = static_cast<u16>(regs.eax);
    corelog_hex("physical size: ", phys_size);

    // Preparing the vmcs structure for VMXON
    corelog_hex("vmxon @ ", reinterpret_cast<u64>(&state->core_vmxon));
    auto vmx_basic = read_msr<ia32_vmx_basic_register>(IA32_VMX_BASIC);
    state->core_vmxon.revision_id = vmx_basic.vmcs_revision_id;
    state->core_vmxon.must_be_zero = 0;
    corelog_hex("vmcs revision id: ", state->core_vmxon.revision_id);

    corelog("Enabling vmx root mode...\n");
    // Enable root mode
    //auto vmx_ptr = &state->core_vmxon;
    auto result = call_vmxon(&state->core_vmxon);
    if (result == 0)
    {
        corelog("Failed to enable root mode!!!\n");
        return 1;
    }

    corelog("done!\n");
    return 0;
}
