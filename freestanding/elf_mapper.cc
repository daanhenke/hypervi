#include "freestanding/elf_mapper.h"
#include "freestanding/libc.h"

elf_mapper::elf_mapper(void* elf_file, size_t import_count, elf_import* imports)
{
    m_file = reinterpret_cast<char*>(elf_file);
    m_imports = imports;
    m_import_count = import_count;

    m_ehdr = reinterpret_cast<elf64_ehdr*>(m_file);
    m_phdrs = reinterpret_cast<elf64_phdr*>(m_file + m_ehdr->e_phoff);
    m_shdrs = reinterpret_cast<elf64_shdr*>(m_file + m_ehdr->e_shoff);
}

size_t elf_mapper::get_mapped_size()
{
    size_t result = 0;

    for (size_t i = 0; i < m_ehdr->e_phnum; i++)
    {
        auto off = m_phdrs[i].p_vaddr + m_phdrs[i].p_memsz;
        if (off > result) result = off;
    }

    return result;
}

#ifndef VIOLET
#include "loader/efi/common.h"
#else
#define log(x)
#define log_hex(x, y)
#endif
void elf_mapper::map_to(void* target_mem)
{
    m_mapped = reinterpret_cast<char*>(target_mem);

    for (size_t i = 0; i < m_ehdr->e_phnum; i++)
    {
        auto section = m_phdrs[i];
        auto target = m_mapped + section.p_vaddr;

        _movsb(target, m_file + section.p_offset, section.p_filesz);
    }

    auto string_section = m_shdrs[m_ehdr->e_shstrndx];

    // Find dynamic string section
    elf64_shdr* dynstr_section = nullptr;
    for (size_t i = 0; i < m_ehdr->e_shnum; i++)
    {
        auto section = m_shdrs[i];

        if (section.sh_type != elf_section_type::strtab) continue;
        auto name = m_file + string_section.sh_offset + section.sh_name;
        if (strcmp(name, ".dynstr") != 0) continue;

        dynstr_section = &section;
        break;
    }


    // Find symbols
    elf64_sym* symbols = nullptr;
    size_t symbol_count = 0;
    for (size_t i = 0; i < m_ehdr->e_shnum; i++)
    {
        auto section = m_shdrs[i];

        if (section.sh_type != elf_section_type::symtab) continue;

        symbol_count = section.sh_size / sizeof(elf64_sym);
        symbols = reinterpret_cast<elf64_sym*>(m_file + section.sh_offset);
        break;
    }

    // Find dynamic symbols
    elf64_sym* dynsyms = nullptr;
    size_t dynsym_count = 0;
    for (size_t i = 0; i < m_ehdr->e_shnum; i++)
    {
        auto section = m_shdrs[i];

        if (section.sh_type != elf_section_type::dynsym) continue;

        dynsym_count = section.sh_size / sizeof(elf64_sym);
        dynsyms = reinterpret_cast<elf64_sym*>(m_file + section.sh_offset);

        break;
    }

    // Log symbols
    // for (size_t i = 0; i < symbol_count; i++)
    // {
    //     log("found symbol: ");
    //     auto text = m_file + string_section.sh_offset + symbols[i].st_name;
    //     log(text);
    //     log("\n");
    // }


    // for (size_t i = 0; i < dynsym_count; i++)
    // {
    //     log("found dynsym: ");
    //     auto text = m_file + dynstr_section->sh_offset + dynsyms[i].st_name;
    //     log(text);
    //     log("\n");
    // }

    // Relocations
    for (size_t i = 0; i < m_ehdr->e_shnum; i++)
    {
        auto section = m_shdrs[i];

        if (section.sh_type == elf_section_type::rela)
        {
            auto reloc_count = section.sh_size / sizeof(elf64_rela);
            auto relocs = reinterpret_cast<elf64_rela*>(m_file + section.sh_offset);

            for (size_t reli = 0; reli < reloc_count; reli++)
            {
                auto info = relocs[reli].r_info;
                auto type = elf64_r_get_type(info);
                auto sym = elf64_r_get_sym(info);

                auto reloc_name = m_file + dynstr_section->sh_offset + dynsyms[sym].st_name;
                size_t* reloc_ptr = nullptr;

                switch (type)
                {
                case elf64_r_type::x86_64_junp_slot:
                case elf64_r_type::x86_64_glob_dat:
                    reloc_ptr = reinterpret_cast<size_t*>(m_mapped + relocs[reli].r_offset);
                    *reloc_ptr = reinterpret_cast<size_t>(resolve_import(reloc_name));
                    break;

                default:
                    log("invalid reloc\n");
                    break;
                }

                log("relocated sym '");
                log(reloc_name);
                log_hex("' to ", *reloc_ptr);
            }
        }
    }
}


void* elf_mapper::resolve_import(const char* name)
{
    for (size_t i = 0; i < m_import_count; i++)
    {
        // log("trying import: ");
        // log(m_imports[i].name);
        // log("\n");
        if (strcmp(m_imports[i].name, name) == 0) return m_imports[i].address;
    }

    return nullptr;
}

char* elf_mapper::get_string(size_t offset)
{
    auto string_section = m_shdrs[m_ehdr->e_shstrndx];
    return reinterpret_cast<char*>(m_file + string_section.sh_offset + offset);
}

char* elf_mapper::get_section_by_name(const char* target_name)
{
    for (size_t i = 0; i < m_ehdr->e_shnum; i++)
    {
        auto section = m_shdrs[i];
        auto name = get_string(section.sh_name);

        //log(name);
        //log("\n");

        if (strcmp(name, target_name) == 0)
        {
            return m_file + section.sh_offset;
        }
    }

    return nullptr;
}
