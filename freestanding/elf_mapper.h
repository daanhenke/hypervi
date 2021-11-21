#pragma once

#include "freestanding/types.h"
#include "freestanding/elf.h"
#include "freestanding/llist.h"

typedef struct
{
    const char* name;
    void* address;
} elf_import;

class elf_mapper
{
public:
    elf_mapper(void* elf_file, size_t import_count, elf_import* imports = nullptr);

    size_t get_mapped_size();
    void map_to(void* target_mem);

    template <typename T>
    T get_entrypoint()
    {
        return reinterpret_cast<T>(m_mapped + reinterpret_cast<size_t>(m_ehdr->e_entry));
    }

    char* get_string(size_t offset);
    char* get_section_by_name(const char* name);
    void iterate_imports(void (*callback_func)(char* name));

protected:
    char* m_file;
    elf_import* m_imports;
    size_t m_import_count;

    elf64_ehdr* m_ehdr;
    elf64_phdr* m_phdrs;
    elf64_shdr* m_shdrs;

    char* m_mapped;

    void* resolve_import(const char* name);
};
