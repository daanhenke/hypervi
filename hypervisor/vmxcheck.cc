#include "hypervisor/cpu.h"
#include "hypervisor/common.h"

#define check_bad(name, value, bad) if (! (value)) { corelog_hex("check failed: " name, bad); return false; }
#define check(name, value) check_bad(name, value, 0x1337);

#define is_flag_set(thing, flag) (((thing) & (flag)) != 0)

enum class segment_type
{
    cs,
    ss,
    ds,
    es,
    fs,
    gs
};

bool check_segment_access_rights(segment_type seg, u32 access_rights, u32 limit, u16 selector, bool ia32_mode_guest, bool unrestriced_guest)
{
    segment_selector selector_val;
    selector_val.flags = selector;

    cr0 guest_cr0;
    guest_cr0.flags = cpu_vmxread(VMCS_GUEST_CR0);

    vmx_segment_access_rights ar, ar_ss, ar_cs;
    ar.flags = access_rights;
    ar_ss.flags = static_cast<u32>(cpu_vmxread(VMCS_GUEST_SS_ACCESS_RIGHTS));
    ar_cs.flags = static_cast<u32>(cpu_vmxread(VMCS_GUEST_CS_ACCESS_RIGHTS));

    switch (seg)
    {
    case segment_type::cs:
        if (! unrestriced_guest)
        {
            check("ar type",
                ar.type == SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_ONLY_ACCESSED ||
                ar.type == SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_READ_ACCESSED ||
                ar.type == SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_ONLY_CONFORMING_ACCESSED ||
                ar.type == SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_READ_CONFORMING_ACCESSED
            );
        }
        else
        {
            check_bad("ar type",
                ar.type == SEGMENT_DESCRIPTOR_TYPE_DATA_READ_WRITE_ACCESSED ||
                ar.type == SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_ONLY_ACCESSED ||
                ar.type == SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_READ_ACCESSED ||
                ar.type == SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_ONLY_CONFORMING_ACCESSED ||
                ar.type == SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_READ_CONFORMING_ACCESSED,
                ar.type
            );
        }
        break;

    case segment_type::ss:
        if (ar.unusable == 0)
        {
            check("ar type",
                ar.type == SEGMENT_DESCRIPTOR_TYPE_DATA_READ_WRITE_ACCESSED ||
                ar.type == SEGMENT_DESCRIPTOR_TYPE_DATA_READ_WRITE_EXPAND_DOWN_ACCESSED
            );
        }
        break;

    default:
        if (ar.unusable == 0)
        {
            check("ar accessed", is_flag_set(ar.type, (1 << 0)));
            if (is_flag_set(ar.type, (1 << 3)))
            {
                check("ar readable", is_flag_set(ar.type, (1 << 1)));
            }
        }
        break;
    }

    if (seg == segment_type::cs || ar.unusable == 0)
    {
        check("ar desc type", ar.descriptor_type == 1);
    }

    switch (seg)
    {
    case segment_type::cs:
        switch (ar.type)
        {
        // TODO: ADD MORE TYPES
        case SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_ONLY_ACCESSED:
        case SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_READ_ACCESSED:
            check("ar dpl", ar.descriptor_privilege_level == ar_ss.descriptor_privilege_level);
            break;

        default:
            return false;
        }
        break;

    case segment_type::ss:
        if (! unrestriced_guest)
        {
            check("ar dpl", ar.descriptor_privilege_level == selector_val.request_privilege_level);
        }

        if (ar_cs.type == SEGMENT_DESCRIPTOR_TYPE_DATA_READ_WRITE_ACCESSED || guest_cr0.protection_enable)
        {
            check("ar dpl", ar.descriptor_privilege_level == 0);
        }
        break;

    default:
        if (! unrestriced_guest && ar.unusable == 0 && ar.type << 11)
        {
            check("ar dpl", ar.descriptor_privilege_level >= selector_val.request_privilege_level);
        }
        break;
    }

    // P (7)
    if (seg == segment_type::cs || ar.unusable == 0)
    {
        check("ar present", ar.present);
    }

    // Reserved (11:8 & 31:17)
    if (seg == segment_type::cs || ar.unusable == 0)
    {
        check("ar reserved", ar.reserved1 == 0 && ar.reserved2 == 0);
    }

    // D/B (14)
    if (seg == segment_type::cs)
    {
        if (ia32_mode_guest == 1 && ar.long_mode)
        {
            check("ar default_big", ar.default_big == 0);
        }
    }

    // G (15)
    if (seg == segment_type::cs || ar.unusable == 0)
    {
        if (! is_flag_set(limit, 0xfff))
        {
            check("granularity", ar.granularity == 0);
        }

        if (is_flag_set(limit, 0xfff00000))
        {
            check("granularity", ar.granularity);
        }

    }

    return true;
}

bool cpu_vmxcheck_guest()
{
    rflags guest_rflags;
    guest_rflags.flags = cpu_vmxread(VMCS_GUEST_RFLAGS);

    ia32_vmx_entry_ctls_register vmentry;
    vmentry.flags = cpu_vmxread(VMCS_CTRL_VMENTRY_CONTROLS);

    ia32_vmx_procbased_ctls_register procbased;
    procbased.flags = cpu_vmxread(VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS);
    ia32_vmx_procbased_ctls2_register procbased2;
    procbased2.flags = cpu_vmxread(VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS);

    bool unrestricted_guest = procbased.activate_secondary_controls && procbased2.unrestricted_guest;

    cr0 guest_cr0;
    guest_cr0.flags = cpu_vmxread(VMCS_GUEST_CR0);
    cr4 guest_cr4;
    guest_cr4.flags = cpu_vmxread(VMCS_GUEST_CR4);

    if (guest_cr0.paging_enable && !unrestricted_guest)
    {
        check("cr0", guest_cr0.protection_enable == 1);
    }

    check("vmentry", vmentry.load_ia32_perf_global_ctrl == 0);

    if (vmentry.ia32e_mode_guest)
    {
        check("cr0", guest_cr0.paging_enable);
        check("cr4", guest_cr4.physical_address_extension);
    }

    if (vmentry.load_ia32_efer)
    {
        ia32_efer_register efer;
        efer.flags = cpu_vmxread(VMCS_GUEST_EFER);

        check("efer", efer.reserved1 == 0);
        check("efer", efer.reserved2 == 0);
        check("efer", efer.reserved3 == 0);
        check("efer", efer.ia32e_mode_active == vmentry.ia32e_mode_guest);

        if (guest_cr0.paging_enable)
        {
            check("efer", efer.ia32e_mode_active == efer.ia32e_mode_enable);
        }
    }

    check("vmentry", vmentry.load_ia32_bndcfgs == 0);
    check("vmentry", vmentry.load_ia32_rtit_ctl == 0);
    check("vmentry", vmentry.load_cet_state == 0);

    segment_selector selector;
    vmx_segment_access_rights access;
    u32 seg_limit;

    selector.flags = static_cast<u16>(cpu_vmxread(VMCS_GUEST_TR_SELECTOR));
    check("tr sel", selector.table == 0);

    access.flags = static_cast<u32>(cpu_vmxread(VMCS_GUEST_LDTR_ACCESS_RIGHTS));
    if (access.unusable == 0)
    {
        selector.flags = static_cast<u16>(cpu_vmxread(VMCS_GUEST_LDTR_SELECTOR));
        check("ldtr sel", selector.table == 0);
    }

    check("cs ar", check_segment_access_rights(
        segment_type::cs,
        static_cast<u32>(cpu_vmxread(VMCS_GUEST_CS_ACCESS_RIGHTS)),
        static_cast<u32>(cpu_vmxread(VMCS_GUEST_CS_LIMIT)),
        static_cast<u16>(cpu_vmxread(VMCS_GUEST_CS_SELECTOR)),
        vmentry.ia32e_mode_guest,
        unrestricted_guest
    ));

    check("ss ar", check_segment_access_rights(
        segment_type::ss,
        static_cast<u32>(cpu_vmxread(VMCS_GUEST_SS_ACCESS_RIGHTS)),
        static_cast<u32>(cpu_vmxread(VMCS_GUEST_SS_LIMIT)),
        static_cast<u16>(cpu_vmxread(VMCS_GUEST_SS_SELECTOR)),
        vmentry.ia32e_mode_guest,
        unrestricted_guest
    ));

    check("ds ar", check_segment_access_rights(
        segment_type::ds,
        static_cast<u32>(cpu_vmxread(VMCS_GUEST_DS_ACCESS_RIGHTS)),
        static_cast<u32>(cpu_vmxread(VMCS_GUEST_DS_LIMIT)),
        static_cast<u16>(cpu_vmxread(VMCS_GUEST_DS_SELECTOR)),
        vmentry.ia32e_mode_guest,
        unrestricted_guest
    ));

    check("es ar", check_segment_access_rights(
        segment_type::es,
        static_cast<u32>(cpu_vmxread(VMCS_GUEST_ES_ACCESS_RIGHTS)),
        static_cast<u32>(cpu_vmxread(VMCS_GUEST_ES_LIMIT)),
        static_cast<u16>(cpu_vmxread(VMCS_GUEST_ES_SELECTOR)),
        vmentry.ia32e_mode_guest,
        unrestricted_guest
    ));

    check("fs ar", check_segment_access_rights(
        segment_type::fs,
        static_cast<u32>(cpu_vmxread(VMCS_GUEST_FS_ACCESS_RIGHTS)),
        static_cast<u32>(cpu_vmxread(VMCS_GUEST_FS_LIMIT)),
        static_cast<u16>(cpu_vmxread(VMCS_GUEST_FS_SELECTOR)),
        vmentry.ia32e_mode_guest,
        unrestricted_guest
    ));

    check("gs ar", check_segment_access_rights(
        segment_type::gs,
        static_cast<u32>(cpu_vmxread(VMCS_GUEST_GS_ACCESS_RIGHTS)),
        static_cast<u32>(cpu_vmxread(VMCS_GUEST_GS_LIMIT)),
        static_cast<u16>(cpu_vmxread(VMCS_GUEST_GS_SELECTOR)),
        vmentry.ia32e_mode_guest,
        unrestricted_guest
    ));

    access.flags = static_cast<u32>(cpu_vmxread(VMCS_GUEST_TR_ACCESS_RIGHTS));
    seg_limit = static_cast<u32>(cpu_vmxread(VMCS_GUEST_TR_LIMIT));
    if (vmentry.ia32e_mode_guest)
    {
        check("tr ar type", access.type == SEGMENT_DESCRIPTOR_TYPE_CODE_EXECUTE_READ_ACCESSED);
    }
    else
    {
        // TODO: IMPLEMENT
        check("tr ar type", false);
    }

    check("tr ar dt", access.descriptor_type == 0);
    check("tr ar p", access.present);
    check("tr ar res", access.reserved1 == 0);
    check("tr ar res2", access.reserved2 == 0);
    check("tr unusable", access.unusable == 0);
    if (! is_flag_set(seg_limit, 0xfff))
    {
        check("tr ar gran", access.granularity == 0);
    }
    if (is_flag_set(seg_limit, 0xfff00000))
    {
        check("tr ar gran", access.granularity);
    }

    // TODO: LDTR

    vmx_segment_access_rights cs_ar;
    cs_ar.flags = static_cast<u32>(cpu_vmxread(VMCS_GUEST_CS_ACCESS_RIGHTS));
    vmx_segment_access_rights ss_ar;
    ss_ar.flags = static_cast<u32>(cpu_vmxread(VMCS_GUEST_SS_ACCESS_RIGHTS));

    check("rflags",
        guest_rflags.reserved1 == 0 &&
        guest_rflags.reserved2 == 0 &&
        guest_rflags.reserved3 == 0 &&
        guest_rflags.reserved4 == 0 &&
        guest_rflags.read_as_1
    );

    vmx_interruptibility_state interruptibility_state;
    interruptibility_state.flags = cpu_vmxread(VMCS_GUEST_INTERRUPTIBILITY_STATE);

    vmx_guest_activity_state activity_state = static_cast<vmx_guest_activity_state>(cpu_vmxread(VMCS_GUEST_ACTIVITY_STATE));

    check("activity state",
        activity_state == vmx_guest_activity_state::vmx_active ||
        activity_state == vmx_guest_activity_state::vmx_hlt ||
        activity_state == vmx_guest_activity_state::vmx_shutdown ||
        activity_state == vmx_guest_activity_state::vmx_wait_for_sipi
    );

    if (ss_ar.descriptor_privilege_level != 0)
    {
        check("activity state", activity_state != vmx_guest_activity_state::vmx_hlt);
    }

    if (interruptibility_state.blocking_by_sti == 1 || interruptibility_state.blocking_by_mov_ss)
    {
        check("activity state", activity_state != vmx_guest_activity_state::vmx_active);
    }

    if (guest_rflags.interrupt_enable_flag == 0)
    {
        check("interrupt", interruptibility_state.blocking_by_sti == 0);
    }

    if (interruptibility_state.enclave_interruption)
    {
        check("interrpt", interruptibility_state.blocking_by_mov_ss == 0);
    }

    return true;
}

bool cpu_vmxcheck()
{
    // TODO: CHECK HOST STATE
    // TODO: CHECK CTRL STATE
    return cpu_vmxcheck_guest();
}
