#include "loader/efi/loader.h"
#include "loader/efi/mp.h"
#include "freestanding/elf_mapper.h"
#include "loader/exports.h"
#include "freestanding/libc.h"

const efi_guid file_info_guid = efi_file_info_guid;

efi_fp* visor_file = nullptr;
char* visor_real = nullptr;
char* visor_mapped = nullptr;

size_t (*hypervisor_main)(hv_init_struct* init);

void efi_loader_init()
{
    visor_file = efi_fs_find_file(L"\\VIOLET\\VIOLET.ELF", static_cast<u64>(efi_fp_mode::read), 0);
    if (visor_file == nullptr)
    {
        log("couldn't find hypervisor on disk\n");
        return;
    }

    efi_fp_file_info visor_info;
    size_t info_size = sizeof(visor_info);

    if (visor_file->get_info(visor_file, const_cast<efi_guid*>(&file_info_guid), &info_size, &visor_info) != efi_status::success)
    {
        log("couldt get visor file size!\n");
        return;
    }

    visor_real = new char[visor_info.file_size];
    if (visor_file->read(visor_file, &visor_info.file_size, visor_real) != efi_status::success)
    {
        log("failed to read hypervisor\n");
        return;
    }
}

void __attribute__((sysv_abi)) imp_log(char* msg)
{
    log(msg);
}


void __attribute__((sysv_abi)) imp_call_on_all_cores(ldr_coac_cb function)
{
    efi_mp_call_on_all_cores(function);
}

elf_import efi_loader_imports[] =
{
    { "ldr_log", imp_log },
    { "ldr_call_on_all_cores", imp_call_on_all_cores }
};

const char* loader_name = "efi_ldr";

void efi_loader_map()
{
    elf_mapper mapper(visor_real, sizeof(efi_loader_imports) / sizeof(elf_import), efi_loader_imports);

    // Using uefi allocate to make this memory runtime moemory instead of boottime
    gST->boot_services->allocate_pages(
        efi_allocate_type::allocate_any_pages,
        efi_memory_type::runtime_services_code,
        NUM_PAGES(mapper.get_mapped_size()),
        reinterpret_cast<void**>(&visor_mapped)
    );

    mapper.map_to(visor_mapped);
    hypervisor_main = mapper.get_entrypoint<decltype(hypervisor_main)>();

    hv_init_struct init_struct = {
        "TEST\0",
        0x1
    };

    log("passing control to hypervisor binary!\n");
    auto result = hypervisor_main(&init_struct);

    log("result: ");
    efi_console_hex(result);
    log("\n");
}
