#include "loader/efi/loader.h"
#include "freestanding/elf_mapper.h"

const efi_guid file_info_guid = efi_file_info_guid;

efi_fp* visor_file = nullptr;
char* visor_real = nullptr;
char* visor_mapped = nullptr;

size_t (*hypervisor_main)();

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

    log("visor file size: ");
    efi_console_hex(visor_info.file_size);
    log("\n");

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

elf_import efi_loader_imports[] =
{
    { "ldr_log", imp_log }
};

void efi_loader_map()
{
    elf_mapper mapper(visor_real, 1, efi_loader_imports);

    // Using uefi allocate to make this memory runtime moemory instead of boottime
    gST->boot_services->allocate_pages(
        efi_allocate_type::allocate_any_pages,
        efi_memory_type::runtime_services_code,
        NUM_PAGES(mapper.get_mapped_size()),
        reinterpret_cast<void**>(&visor_mapped)
    );

    log("import table: ");
    efi_console_hex(reinterpret_cast<size_t>(mapper.get_section_by_name(".got.plt")));

    mapper.map_to(visor_mapped);
    hypervisor_main = mapper.get_entrypoint<decltype(hypervisor_main)>();

    log("calling hv: ");
    efi_console_hex(hypervisor_main());
    log("\n");
}
