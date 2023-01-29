/*
 * `load.c` -- x86-64 EFI kernel loader
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>

#include <elf.h>
#include <efi.h>

/*
 * Second phase of bootloader
 * - 
 */

#define print(string) systab->console_out->output_string(systab->console_out,\
                                                (int16_t *) string)

/*
 * From boot.c
 */
extern efi_system_table *               systab;         /* EFI system table */
extern efi_boot_services *              bootsrv;        /* boot services */
extern efi_file_protocol *              kfile;          /* kernel file handle */
extern void *                           pagetabs;       /* new page tables */

extern void printh(uint64_t);

/*
 * ELF header structures used for parsing
 */
static Elf64_Ehdr ehdr;         /* Elf header */
static Elf64_Phdr *phdrs;       /* Program headers */

static void *PML4;

/*
 * Zero memory from and to
 */
static void
memzero(void *from, void *to)
{
        uint8_t *addr;

        for (; from <= to; from++) {
                addr = (uint8_t*) from;
                *addr = 0;
        }
}

/*
 * Read the program headers
 */
static int
read_phdrs(void)
{
        efi_status s;
        uint64_t bufsz;

        bufsz = ehdr.e_phentsize * ehdr.e_phnum;
        s = bootsrv->allocate_pool(
                efi_loader_data,
                bufsz,
                (void **) &phdrs
        );

        if (s != EFI_SUCCESS) {
                print(L"Failed pool allocation\r\n");
                return 1;
        }

        kfile->set_position(kfile, ehdr.e_phoff);
        s = kfile->read(
                kfile,
                &bufsz,
                phdrs
        );

        if (s != EFI_SUCCESS) {
                print(L"Failed read\r\n");
                return 1;
        }

        return 0;
}

/*
 * Read the Elf header and perform potential checks
 */
static int
read_ehdr(void)
{
        efi_status s;
        uint64_t bufsz;

        bufsz = sizeof(Elf64_Ehdr);
        s = kfile->read(
                kfile,
                &bufsz,
                &ehdr
        );

        if (s != EFI_SUCCESS) {
                print(L"Failed read\r\n");
                return 1;
        }

        /*
         * After reading Elf header perform checks on the data, make sure this
         * is a valid executable for our kernel
         */
        if ((ehdr.e_ident[EI_MAG0] != ELFMAG0)
        || (ehdr.e_ident[EI_MAG1] != ELFMAG1)
        || (ehdr.e_ident[EI_MAG2] != ELFMAG2)
        || (ehdr.e_ident[EI_MAG3] != ELFMAG3))
                return 1;

        if (ehdr.e_ident[EI_CLASS] != ELFCLASS64)
                return 1;

        if (ehdr.e_ident[EI_DATA] != ELFDATA2LSB)
                return 1;

        if (ehdr.e_type != ET_EXEC)
                return 1;

        if (ehdr.e_machine != EM_X86_64)
                return 1;

        return 0;
}

static int
loadsegs(void)
{
        void *page;
        uint64_t pgs, sz;
        int i;

        for (i = 0; i < ehdr.e_phnum; i++) {
                if (phdrs[i].p_type != PT_LOAD)
                        continue;

                /*
                 * Allocate the amount of pages we need to load this segment
                 */
                if (phdrs[i].p_memsz < 0x1000)
                        pgs = 1;
                else
                        pgs = phdrs[i].p_memsz / 0x1000;

                bootsrv->allocate_pages(
                        allocate_any_pages,
                        efi_reserved_memory_type,
                        pgs,
                        (uint64_t *) &page
                );
                
                /*
                 * Save physical address in the structure, we will loop through
                 * again later to perform final mappings
                 */
                phdrs[i].p_paddr = (uint64_t) page;

                /*
                 * Read segment content from file into pages if necessary
                 */
                if (phdrs[i].p_filesz != 0) {
                        kfile->set_position(kfile, phdrs[i].p_offset);
                        sz = phdrs[i].p_filesz;
                        kfile->read(
                                kfile,
                                &sz,
                                page
                        );
                }

                /*
                 * Zero unused memory if necessary
                 */
                if (phdrs[i].p_memsz > phdrs[i].p_filesz) {
                        memzero(page + phdrs[i].p_filesz + 1,
                                page + (pgs * 0x1000));
                }

        }

        return 0;
}

/*
 * Loader driver
 */
efi_status 
load(void)
{
        efi_status s;
        uint64_t bufsz;
        uint64_t mem;

        /*
         * Read the Elf header and verify its validity
         */
        if (read_ehdr() != 0) {
                print(L"Invalid kernel\r\n");
                return 1;
        }

        /*
         * Read program headers to be analyzed
         */
        if (read_phdrs() != 0)
                return 1;

        /*
         * Allocate page for PML4
         */
        /*bootsrv->allocate_pages(
                allocate_any_pages,
                efi_reserved_memory_type,
                1,
                &mem
        );
        PML4 = mem;*/

        loadsegs();

        for(;;);

        return 0;
}
