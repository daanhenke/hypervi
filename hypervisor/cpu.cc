#include "hypervisor/cpu.h"
#include "hypervisor/common.h"
#include "freestanding/libc.h"
#include "hypervisor/ept.h"

cpu_state g_cpu_states[cpu_max];

size_t cpu_alter_control_register(size_t original_value, size_t msr_id_0, size_t msr_id_1)
{
    u32 or_data = read_msr(msr_id_0);
    u32 and_data = read_msr(msr_id_1);

    original_value &= and_data;
    original_value |= or_data;

    return original_value;
}

size_t cpu_alter_vmx_control_register(size_t original, size_t msr)
{
    auto msr_data = read_msr<ia32_vmx_true_ctls_register>(msr);

    original |= msr_data.allowed_0_settings;
    original &= msr_data.allowed_1_settings;

    return original;
}

void cpu_vmxwrite(u64 field, u64 value)
{
    auto result = call_vmwrite(field, value);
    if (result != 0)
    {
        corelog_hex("vmxwrite failed, field: ", field);
    }
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

    // Just a debug thing, can be removed
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

    corelog("enabling vmx root mode...\n");

    // Enable root mode
    auto result = call_vmxon(&state->core_vmxon);
    if (result != 0)
    {
        corelog("failed to enable root mode!!!\n");
        return 1;
    }

    // Clear vmx caches
    corelog("clearing caches...\n")
    ept_invalidate_ept_cache(nullptr);
    ept_invalidate_vpid_cache(0);

    // Set up our vmcs
    corelog_hex("setting up vmcs @ ", reinterpret_cast<size_t>(&state->core_vmcs));
    state->core_vmcs.revision_id = vmx_basic.vmcs_revision_id;
    if (call_vmclear(&state->core_vmcs) != 0 ||
        call_vmptrld(&state->core_vmcs) != 0)
    {
        corelog("failed to set up vmcs!\n");
    }

    // Configure vmexit controls
    ia32_vmx_exit_ctls_register vmexit_ctls;
    vmexit_ctls.flags = 0;
    vmexit_ctls.host_address_space_size = 1;
    vmexit_ctls.load_ia32_efer = true;
    vmexit_ctls.save_ia32_efer = true;
    vmexit_ctls.flags = cpu_alter_vmx_control_register(vmexit_ctls.flags, vmx_basic.vmx_controls != 0 ? IA32_VMX_TRUE_EXIT_CTLS : IA32_VMX_EXIT_CTLS);

    // Configure vmentry controls
    ia32_vmx_entry_ctls_register vmentry_ctls;
    vmentry_ctls.flags = 0;
    vmentry_ctls.load_ia32_efer = true;
    vmentry_ctls.ia32e_mode_guest = true;
    vmentry_ctls.flags = cpu_alter_vmx_control_register(vmentry_ctls.flags, vmx_basic.vmx_controls != 0 ? IA32_VMX_TRUE_ENTRY_CTLS : IA32_VMX_ENTRY_CTLS);

    // Writing state to vmcs
    corelog("Writing guest state to vmcs...\n");
    cpu_vmxwrite(VMCS_CTRL_VMEXIT_CONTROLS, vmexit_ctls.flags);
    cpu_vmxwrite(VMCS_CTRL_VMENTRY_CONTROLS, vmentry_ctls.flags);

    corelog("done!\n");
    return 0;
}
