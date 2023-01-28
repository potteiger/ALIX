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
extern void printh(uint64_t);

/*
 * ELF header structures used for parsing
 */
static Elf64_Ehdr ehdr;         /* Elf header */
static Elf64_Phdr *phdrs;       /* Program headers */

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

/*
 * Loader driver
 */
efi_status 
load(void)
{
        efi_status s;
        uint64_t bufsz;

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

        return 0;
}
