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

extern uintptr_t 			prep_map();
extern void				er(uintptr_t, uintptr_t);

/* ELF header structures used for parsing kernel executable */
static Elf64_Ehdr ehdr;         /* Elf header */
static Elf64_Phdr *phdrs;       /* Program headers */

static uint64_t *pdpt;

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

/* Return next page table in hierarchy, create a new one if necessary. */
static uint64_t *
pagetab_next(uint64_t *currtab, uint16_t index)
{
	uintptr_t ptr;
	uintptr_t page;

	ptr = (uintptr_t) currtab[index];

	/*
	 * If entry is not occupied, we allocate and zero a table for the next
	 * table in the hierarchy then fill the entry, sanitisze address, and
	 * return.
	 */

	//printh(index);
	//print(L"\r\n");

	if (ptr == 0) {
		ptr = palloc(1);
		if (ptr == 0)
			return NULL;
		memzero(ptr, ptr + (0x1000));
		currtab[index] = ptr | 3; /* Present, R/W */
	}

	/* Clear flags if present for clear address */
	ptr = ((ptr >> 12) & 0x7FFFFFFFFF) << 12;
	return (uint64_t *) ptr;
}

/* Map virtual addy to physical addy. */
int
mapaddr(uintptr_t virt, uintptr_t phys)
{
	uint16_t index;
	uint64_t *table;

	table = pdpt;

	table = pagetab_next(table, ((virt >> 30) & 0x1FF));
	if (table == NULL) {
		print(L"Failed mapping "); printh(virt); print(L"\r\n");
		return 1;
	}
	table = pagetab_next(table, ((virt >> 21) & 0x1FF));
	if (table == NULL) {
		print(L"Failed mapping "); printh(virt); print(L"\r\n");
		return 1;
	}
	
	/*
	 * Reached the page table, now set the address of the page frame with
	 * appropriate flags.
	 */
	index = ((virt >> 12) & 0x1FF);
	printh(phys);
	table[index] = phys | 3; /* Present, R/W */
	return 0;
}

static int
loadk(void)
{
	void *page;
	uint64_t pgs, sz;
	int i, j, loadable;

	pdpt = (uint64_t *)palloc(1);
	memzero((uintptr_t)pdpt, (uintptr_t)pdpt + 0x1000);

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
		printh((uintptr_t) page);
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

		/* Map pages in the tables we're building */
		for (j = 0; j < pgs; j++)
			mapaddr(phdrs[i].p_vaddr + (j * 0x1000),
					((uintptr_t) page) + (j * 0x1000));
	}

	return 0;
}

efi_status 
load(void)
{
	efi_status s;
	uintptr_t cr3;
	uint64_t *pml4;
	uintptr_t val;

	/* Read the Elf header and verify its validity */
	if (read_ehdr() != 0) {
		print(L"Invalid kernel\r\n");
		return 1;
	}

	/* Read program headers to be analyzed */
	if (read_phdrs() != 0)
		return 1;

	/* Allocate pages and load segments into memory */
	loadk();

	printh(kargtab.fb.size);

	getmmap();
	bootsrv->exit_boot_services(imghan, mmapkey);

	/*
	 * Prepare to map kernel pages. Ensures bit 16 in `cr0` is off and
	 * returns the value of `cr3` so we can access PML4.
	 */
	cr3 = prep_map();
	pml4 = (uint64_t *)((cr3 >> 12) << 12);
	pml4[((ehdr.e_entry >> 39) & 0x1FF)] = (((uintptr_t)pdpt) | 3);

	er(ehdr.e_entry, (uintptr_t) &kargtab);
	return 0;
}

