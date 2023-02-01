/*
 * `load.c` -- x86-64 EFI bootloader, load phase
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>
#include <stddef.h>

#include <elf.h>
#include <efi.h>
#include <sys/kargtab.h>

#define print(string) systab->console_out->output_string(systab->console_out,\
						(int16_t *) string)

/* From `boot.c` */
extern void 				printh(uint64_t);
extern void 				memzero(uintptr_t from, uintptr_t to);
extern uintptr_t			palloc(int count);
extern int 				getmmap(void);
extern int 				mapaddr(uintptr_t, uintptr_t);

extern efi_system_table *		systab;	/* EFI system table */
extern efi_handle			imghan; /* EFI app image handle */
extern efi_boot_services *		bootsrv;/* boot services */
extern efi_file_protocol *		kfile;  /* kernel file handle */
extern efi_graphics_output_protocol *	gop;    /* Graphics Output Protocol */
extern uint64_t				mmapkey;/* EFI mmap key */
extern struct kargtab 			kargtab;/* Kernel argument table */

/* Enter 'er' phase of bootloader, `er.s` */
extern void 				er(uintptr_t, uintptr_t, uintptr_t,
						uintptr_t);

/* ELF header structures used for parsing kernel executable */
static Elf64_Ehdr ehdr;         /* Elf header */
static Elf64_Phdr *phdrs;       /* Program headers */

/* Reads the program headers */
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

/* Reads the Elf header */
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
kload(void)
{
	void *page;
	uint64_t pgs, sz;
	int i;

	for (i = 0; i < ehdr.e_phnum; i++) {
		if (phdrs[i].p_type != PT_LOAD)
			continue;

		/* Allocate the amount of pages we need to load this segment */
		if (phdrs[i].p_memsz < 0x1000)
			pgs = 1;
		else
			pgs = phdrs[i].p_memsz / 0x1000;

		bootsrv->allocate_pages(
			allocate_any_pages,
			efi_runtime_services_code,
			pgs,
			(uint64_t *) &page
		);

		/* Read segment content from file into pages if necessary */
		if (phdrs[i].p_filesz != 0) {
			kfile->set_position(kfile, phdrs[i].p_offset);
			sz = phdrs[i].p_filesz;
			kfile->read(
				kfile,
				&sz,
				page
			);
		}

		/* Zero unused memory if necessary */
		if (phdrs[i].p_memsz > phdrs[i].p_filesz) {
			memzero((uintptr_t) (page + phdrs[i].p_filesz + 1),
				(uintptr_t)(page + (pgs * 0x1000)));
		}

		/* Map segment */
		if (mapaddr(phdrs[i].p_vaddr, (uintptr_t) page) != 0)
			return 1;

	}

	return 0;
}

efi_status 
load(void)
{
	efi_status s;
	uint64_t bufsz;
	uint64_t mem;
	uint64_t *addy;
	uint64_t *ptr;
	uintptr_t stack;
	uint32_t *fb;
	int i;

	/* Read the Elf header and verify its validity */
	if (read_ehdr() != 0) {
		print(L"Invalid kernel\r\n");
		return 1;
	}

	/* Read program headers to be analyzed */
	if (read_phdrs() != 0)
		return 1;

	/* Load segments into memory and map them properly */
	kload();

	/* Allocate and map a stack */
	stack = palloc(1);
	if (mapaddr(stack, stack) != 0)
		return 0;
	stack = stack + (0x1000 - 16);

	getmmap();
	bootsrv->exit_boot_services(imghan, mmapkey);

	er((uintptr_t) &kargtab, kargtab.pml4_virt, stack, ehdr.e_entry);

	return 0;
}

