#include "hypervisor/cpu.h"
#include "hypervisor/common.h"
#include "freestanding/libc.h"
#include "hypervisor/ept.h"
#include "hypervisor/host.h"

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

    original &= msr_data.allowed_1_settings;
    original |= msr_data.allowed_0_settings;

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

u64 cpu_vmxread(u64 field)
{
    u64 value = 0x1337420;

    auto result = call_vmread(field, &value);
    if (result != 0)
    {
        corelog_hex("vmxread failed, field: ", field);
    }

    return value;
}

typedef struct { size_t index; const char* name; } vmcsdumpe;
vmcsdumpe keys_to_dump[] =
{
    { VMCS_HOST_RSP, "host rsp" },
    { VMCS_HOST_RIP, "host rip" },

    { VMCS_GUEST_RSP, "guest rsp" },
    { VMCS_GUEST_RIP, "guest rip" },
    { VMCS_GUEST_ES_SELECTOR, "guest es sel" },
    { VMCS_GUEST_CS_SELECTOR, "guest cs sel" },
    { VMCS_GUEST_SS_SELECTOR, "guest ss sel" },
    { VMCS_GUEST_DS_SELECTOR, "guest ds sel" },
    { VMCS_GUEST_FS_SELECTOR, "guest fs sel" },
    { VMCS_GUEST_GS_SELECTOR, "guest gs sel" },
    { VMCS_GUEST_LDTR_SELECTOR, "guest ldtr sel" },
    { VMCS_GUEST_TR_SELECTOR, "guest tr sel" },

    { VMCS_GUEST_VMCS_LINK_POINTER, "guest vmc link" },
    { VMCS_GUEST_EFER, "guest efer" },
    { VMCS_GUEST_ES_LIMIT, "guest es limit" },
    { VMCS_GUEST_CS_LIMIT, "guest cs limit" },
    { VMCS_GUEST_SS_LIMIT, "guest ss limit" },
    { VMCS_GUEST_DS_LIMIT, "guest ds limit" },
    { VMCS_GUEST_FS_LIMIT, "guest fs limit" },
    { VMCS_GUEST_GS_LIMIT, "guest gs limit" },
    { VMCS_GUEST_LDTR_LIMIT, "guest ldtr limit" },
    { VMCS_GUEST_TR_LIMIT, "guest tr limit" },
    { VMCS_GUEST_GDTR_LIMIT, "guest gdtr limit" },
    { VMCS_GUEST_IDTR_LIMIT, "guest idtr limit" },
    { VMCS_GUEST_ES_ACCESS_RIGHTS, "guest es ar" },
    { VMCS_GUEST_CS_ACCESS_RIGHTS, "guest cs ar" },
    { VMCS_GUEST_SS_ACCESS_RIGHTS, "guest ss ar" },
    { VMCS_GUEST_DS_ACCESS_RIGHTS, "guest ds ar" },
    { VMCS_GUEST_FS_ACCESS_RIGHTS, "guest fs ar" },
    { VMCS_GUEST_GS_ACCESS_RIGHTS, "guest gs ar" },
    { VMCS_GUEST_LDTR_ACCESS_RIGHTS, "guest ldtr ar" },
    { VMCS_GUEST_TR_ACCESS_RIGHTS, "guest tr ar" },
    { VMCS_GUEST_SYSENTER_CS, "guest sysenter cs" },
    { VMCS_GUEST_CR0, "guest cr0", },
    { VMCS_GUEST_CR3, "guest cr3", },
    { VMCS_GUEST_CR4, "guest cr4", },
    { VMCS_GUEST_FS_BASE, "guest fs base", },
    { VMCS_GUEST_GS_BASE, "guest gs base", },
    { VMCS_GUEST_LDTR_BASE, "guest ldtr base", },
    { VMCS_GUEST_TR_BASE, "guest tr base", },
    { VMCS_GUEST_GDTR_BASE, "guest gdtr base", },
    { VMCS_GUEST_IDTR_BASE, "guest idtr base", },
    { VMCS_GUEST_RSP, "guest rsp", },
    { VMCS_GUEST_RIP, "guest rip", },
    { VMCS_GUEST_RFLAGS, "guest rflags", },
    { VMCS_GUEST_SYSENTER_ESP, "guest sysenter esp", },
    { VMCS_GUEST_SYSENTER_EIP, "guest sysenter eip", },
};

void cpu_dumpvmcs()
{
    const size_t entry_count = sizeof(keys_to_dump) / sizeof(vmcsdumpe);

    for (size_t i = 0; i < entry_count; i++)
    {
        corelog_hex(keys_to_dump[i].name, cpu_vmxread(keys_to_dump[i].index));
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
    memset(&state->host_stack, 0, sizeof(state->host_stack));

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

    ia32_vmx_pinbased_ctls_register pinbased_ctls;
    pinbased_ctls.flags = 0;
    pinbased_ctls.nmi_exiting = 1;
    pinbased_ctls.virtual_nmi = 1;
    pinbased_ctls.flags = cpu_alter_vmx_control_register(pinbased_ctls.flags, vmx_basic.vmx_controls != 0 ? IA32_VMX_TRUE_PINBASED_CTLS : IA32_VMX_PINBASED_CTLS);

    ia32_vmx_procbased_ctls_register procbased_ctls;
    procbased_ctls.flags = 0;
    procbased_ctls.use_msr_bitmaps = 0;
    procbased_ctls.activate_secondary_controls = 1; // TODO: FLIP AND FIX SECONDARY CTRLS
    procbased_ctls.flags = cpu_alter_vmx_control_register(procbased_ctls.flags, vmx_basic.vmx_controls != 0 ? IA32_VMX_TRUE_PROCBASED_CTLS : IA32_VMX_PROCBASED_CTLS);

    ia32_vmx_procbased_ctls2_register procbased2_ctls;
    procbased2_ctls.flags = 0;
    procbased2_ctls.enable_ept = 0;
    procbased2_ctls.enable_vpid = 1;
    procbased2_ctls.enable_rdtscp = 1;
    procbased2_ctls.unrestricted_guest = 0;
    procbased2_ctls.enable_invpcid = 1;
    procbased2_ctls.enable_xsaves = 0;
    procbased2_ctls.flags = cpu_alter_vmx_control_register(procbased2_ctls.flags, IA32_VMX_PROCBASED_CTLS2);

    segment_descriptor_register_64 gdtr;
    segment_descriptor_register_64 idtr;
    read_gdtr(&gdtr);
    read_idtr(&idtr);

    auto saved_stack = get_rsp();
    auto saved_rip = get_rip();

    corelog("writing guest state to vmcs...\n");

    // 16 bit host state fields
    const u32 selector_mask = 0x7;
    cpu_vmxwrite(VMCS_HOST_ES_SELECTOR, read_es() & ~selector_mask);
    cpu_vmxwrite(VMCS_HOST_CS_SELECTOR, read_cs() & ~selector_mask);
    cpu_vmxwrite(VMCS_HOST_SS_SELECTOR, read_ss() & ~selector_mask);
    cpu_vmxwrite(VMCS_HOST_DS_SELECTOR, read_ds() & ~selector_mask);
    cpu_vmxwrite(VMCS_HOST_FS_SELECTOR, read_fs() & ~selector_mask);
    cpu_vmxwrite(VMCS_HOST_GS_SELECTOR, read_gs() & ~selector_mask);
    cpu_vmxwrite(VMCS_HOST_TR_SELECTOR, read_tr() & ~selector_mask);

    // 32 & 64 bit host state fields
    cpu_vmxwrite(VMCS_HOST_EFER, read_msr(IA32_EFER));
    cpu_vmxwrite(VMCS_HOST_SYSENTER_CS, read_msr(IA32_SYSENTER_CS));

    // natural width host state fields
    cpu_vmxwrite(VMCS_HOST_CR0, read_cr0());
    cpu_vmxwrite(VMCS_HOST_CR3, read_cr3());
    cpu_vmxwrite(VMCS_HOST_CR4, read_cr4());
    cpu_vmxwrite(VMCS_HOST_FS_BASE, read_msr(IA32_FS_BASE));
    cpu_vmxwrite(VMCS_HOST_GS_BASE, read_msr(IA32_GS_BASE));
    cpu_vmxwrite(VMCS_HOST_TR_BASE, get_segment_base(gdtr.base_address, read_tr()));
    cpu_vmxwrite(VMCS_HOST_GDTR_BASE, gdtr.base_address);
    cpu_vmxwrite(VMCS_HOST_IDTR_BASE, idtr.base_address);
    cpu_vmxwrite(VMCS_HOST_SYSENTER_ESP, read_msr(IA32_SYSENTER_ESP));
    cpu_vmxwrite(VMCS_HOST_SYSENTER_EIP, read_msr(IA32_SYSENTER_EIP));
    // TODO: ACTUALLY FILL THESE
    cpu_vmxwrite(VMCS_HOST_RSP, reinterpret_cast<u64>(state->host_stack));
    cpu_vmxwrite(VMCS_HOST_RIP, reinterpret_cast<u64>(host_hv_entrypoint));

    // 16 bit control fields
    cpu_vmxwrite(VMCS_CTRL_VIRTUAL_PROCESSOR_IDENTIFIER, 1);

    // 64 bit control fields
    cpu_vmxwrite(VMCS_CTRL_MSR_BITMAP_ADDRESS, 0);
    cpu_vmxwrite(VMCS_CTRL_EPT_POINTER, 0);

    // 32 bit control fields width control fields
    cpu_vmxwrite(VMCS_CTRL_EXCEPTION_BITMAP, 0);
    cpu_vmxwrite(VMCS_CTRL_PIN_BASED_VM_EXECUTION_CONTROLS, pinbased_ctls.flags);
    cpu_vmxwrite(VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, procbased_ctls.flags);
    cpu_vmxwrite(VMCS_CTRL_VMEXIT_CONTROLS, vmexit_ctls.flags);
    cpu_vmxwrite(VMCS_CTRL_VMENTRY_CONTROLS, vmentry_ctls.flags);
    cpu_vmxwrite(VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, procbased2_ctls.flags);

    // natural width control fields
    cpu_vmxwrite(VMCS_CTRL_CR0_GUEST_HOST_MASK, CR0_NUMERIC_ERROR_FLAG | CR0_PAGING_ENABLE_FLAG);
    cpu_vmxwrite(VMCS_CTRL_CR0_READ_SHADOW, read_cr0());
    cpu_vmxwrite(VMCS_CTRL_CR4_GUEST_HOST_MASK, CR4_VMX_ENABLE_FLAG);
    cpu_vmxwrite(VMCS_CTRL_CR4_READ_SHADOW, read_cr4());

    // 16 bit guest state fields
    cpu_vmxwrite(VMCS_GUEST_ES_SELECTOR, read_es());
    cpu_vmxwrite(VMCS_GUEST_CS_SELECTOR, read_cs());
    cpu_vmxwrite(VMCS_GUEST_SS_SELECTOR, read_ss());
    cpu_vmxwrite(VMCS_GUEST_DS_SELECTOR, read_ds());
    cpu_vmxwrite(VMCS_GUEST_FS_SELECTOR, read_fs());
    cpu_vmxwrite(VMCS_GUEST_GS_SELECTOR, read_gs());
    cpu_vmxwrite(VMCS_GUEST_LDTR_SELECTOR, read_ldtr());
    cpu_vmxwrite(VMCS_GUEST_TR_SELECTOR, read_tr());

    cpu_vmxwrite(VMCS_GUEST_ES_BASE, 0);
    cpu_vmxwrite(VMCS_GUEST_CS_BASE, 0);
    cpu_vmxwrite(VMCS_GUEST_SS_BASE, 0);
    cpu_vmxwrite(VMCS_GUEST_DS_BASE, 0);

    // 64 bit guest state fields
    cpu_vmxwrite(VMCS_GUEST_VMCS_LINK_POINTER, 0xFFFFFFFFFFFFFFFF);
    cpu_vmxwrite(VMCS_GUEST_EFER, read_msr(IA32_EFER));

    // 32 bit guest state fields
    cpu_vmxwrite(VMCS_GUEST_ES_LIMIT, get_segment_limit(read_es()));
    cpu_vmxwrite(VMCS_GUEST_CS_LIMIT, get_segment_limit(read_cs()));
    cpu_vmxwrite(VMCS_GUEST_SS_LIMIT, get_segment_limit(read_ss()));
    cpu_vmxwrite(VMCS_GUEST_DS_LIMIT, get_segment_limit(read_ds()));
    cpu_vmxwrite(VMCS_GUEST_FS_LIMIT, get_segment_limit(read_fs()));
    cpu_vmxwrite(VMCS_GUEST_GS_LIMIT, get_segment_limit(read_gs()));
    cpu_vmxwrite(VMCS_GUEST_LDTR_LIMIT, get_segment_limit(read_ldtr()));
    cpu_vmxwrite(VMCS_GUEST_TR_LIMIT, get_segment_limit(read_tr()));
    cpu_vmxwrite(VMCS_GUEST_GDTR_LIMIT, gdtr.limit);
    cpu_vmxwrite(VMCS_GUEST_IDTR_LIMIT, idtr.limit);
    cpu_vmxwrite(VMCS_GUEST_ES_ACCESS_RIGHTS, get_segment_access_rights_vmx(read_es()));
    cpu_vmxwrite(VMCS_GUEST_CS_ACCESS_RIGHTS, get_segment_access_rights_vmx(read_cs()));
    cpu_vmxwrite(VMCS_GUEST_SS_ACCESS_RIGHTS, get_segment_access_rights_vmx(read_ss()));
    cpu_vmxwrite(VMCS_GUEST_DS_ACCESS_RIGHTS, get_segment_access_rights_vmx(read_ds()));
    cpu_vmxwrite(VMCS_GUEST_FS_ACCESS_RIGHTS, get_segment_access_rights_vmx(read_fs()));
    cpu_vmxwrite(VMCS_GUEST_GS_ACCESS_RIGHTS, get_segment_access_rights_vmx(read_gs()));
    cpu_vmxwrite(VMCS_GUEST_LDTR_ACCESS_RIGHTS, get_segment_access_rights_vmx(read_ldtr()));
    cpu_vmxwrite(VMCS_GUEST_TR_ACCESS_RIGHTS, get_segment_access_rights_vmx(read_tr()));
    cpu_vmxwrite(VMCS_GUEST_SYSENTER_CS, read_msr(IA32_SYSENTER_CS));

    // natural width guest state fields
    cpu_vmxwrite(VMCS_GUEST_CR0, read_cr0());
    cpu_vmxwrite(VMCS_GUEST_CR3, read_cr3());
    cpu_vmxwrite(VMCS_GUEST_CR4, read_cr4());
    cpu_vmxwrite(VMCS_GUEST_FS_BASE, read_msr(IA32_FS_BASE));
    cpu_vmxwrite(VMCS_GUEST_GS_BASE, read_msr(IA32_GS_BASE));
    cpu_vmxwrite(VMCS_GUEST_LDTR_BASE, get_segment_base(gdtr.base_address, read_ldtr()));
    cpu_vmxwrite(VMCS_GUEST_TR_BASE, get_segment_base(gdtr.base_address, read_tr()));
    cpu_vmxwrite(VMCS_GUEST_GDTR_BASE, gdtr.base_address);
    cpu_vmxwrite(VMCS_GUEST_IDTR_BASE, idtr.base_address);
    cpu_vmxwrite(VMCS_GUEST_RSP, saved_stack);
    cpu_vmxwrite(VMCS_GUEST_RIP, saved_rip);
    cpu_vmxwrite(VMCS_GUEST_RFLAGS, read_rflags());
    cpu_vmxwrite(VMCS_GUEST_SYSENTER_ESP, read_msr(IA32_SYSENTER_ESP));
    cpu_vmxwrite(VMCS_GUEST_SYSENTER_EIP, read_msr(IA32_SYSENTER_EIP));

    cpu_dumpvmcs();

    host_vmexit_ptr = reinterpret_cast<size_t>(host_vmexit);

    corelog("done! calling vmlaunch\n");
    auto status = call_vmlaunch();
    if (status != 0)
    {
        corelog("oh no, we returned from vmlaunch!!!!!!!!!!!!!!!!!!\n");

        if (status == 1)
        {
            corelog_hex("what happen!!!?: ", cpu_vmxread(VMCS_VM_INSTRUCTION_ERROR));
        }
    }
    else
    {
        corelog("we hypervibing\n");
    }

    return 0;
}
