#include "freestanding/elf_mapper.h"
#include "freestanding/libc.h"

elf_mapper::elf_mapper(void* elf_file)
{
    m_file = reinterpret_cast<char*>(elf_file);
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
    for (size_t i = 0; i < m_ehdr->e_shnum; i++)
    {
        auto section = m_shdrs[i];
        auto name = reinterpret_cast<char*>(m_file + string_section.sh_offset + section.sh_name);
    }
}
