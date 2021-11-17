#pragma once

#include "freestanding/types.h"
#include "freestanding/elf.h"

class elf_mapper
{
public:
    elf_mapper(void* elf_file);

    size_t get_mapped_size();
    void map_to(void* target_mem);

    template <typename T>
    T get_entrypoint()
    {
        return reinterpret_cast<T>(m_mapped + reinterpret_cast<size_t>(m_ehdr->e_entry));
    }

protected:
    char* m_file;
    elf64_ehdr* m_ehdr;
    elf64_phdr* m_phdrs;
    elf64_shdr* m_shdrs;

    char* m_mapped;
};
