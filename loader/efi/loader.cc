#include "loader/efi/loader.h"
#include "freestanding/elf.h"
#include "freestanding/libc.h"

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

void efi_loader_map()
{
    auto elf_src = reinterpret_cast<elf64_ehdr*>(visor_real);
    auto shdrs_src = reinterpret_cast<elf64_shdr*>(visor_real + elf_src->e_shoff);
    auto phdrs_src = reinterpret_cast<elf64_phdr*>(visor_real + elf_src->e_phoff);

    size_t elf_sections_size = 0;
    for (size_t i = 0; i < elf_src->e_phnum; i++)
    {
        auto off = phdrs_src[i].p_vaddr + phdrs_src[i].p_memsz;
        if (off > elf_sections_size) elf_sections_size = off;
    }

    log("elf size: ");
    efi_console_hex(elf_sections_size);
    log("\n");

    // Using uefi allocate to make this memory runtime moemory instead of boottime
    gST->boot_services->allocate_pages(efi_allocate_type::allocate_any_pages, efi_memory_type::runtime_services_code, NUM_PAGES(elf_sections_size), reinterpret_cast<void**>(&visor_mapped));

    log("elf base: ");
    efi_console_hex(reinterpret_cast<size_t>(visor_mapped));
    log("\n");

    for (size_t i = 0; i < elf_src->e_phnum; i++)
    {
        auto section = phdrs_src[i];
        auto off = visor_mapped + section.p_vaddr;

        _movsb(off, visor_real + section.p_offset, section.p_filesz);
        log("copied sector ");
        efi_console_hex(i);
        log("\n");
    }

    auto string_section = shdrs_src[elf_src->e_shstrndx];
    for (size_t i = 0; i < elf_src->e_shnum; i++)
    {
        auto section = shdrs_src[i];
        auto name = reinterpret_cast<char*>(visor_real + string_section.sh_offset + section.sh_name);

        log("found section: ");
        log(name);
        log("\n");
    }

    hypervisor_main = reinterpret_cast<decltype(hypervisor_main)>(visor_mapped + reinterpret_cast<u64>(elf_src->e_entry));
    log("calling hv: ");
    efi_console_hex(hypervisor_main());
    log("\n");
}
