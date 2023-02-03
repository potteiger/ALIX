/*
 * `load.c` -- ALIX bootloader (x86-64 EFI), load phase
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

/* EFI console macros */
#define print(string) systab->console_out->output_string(systab->console_out,\
						(int16_t *) string)
#define println(string) { print(string); print(L"\r\n"); }
#define printhln(val) { printh(val); print(L"\r\n"); }
/* Perhaps this will be done more 'proper' later... EFI is just so damn dumb */
#define fatal(string) { print(string); \
			bootsrv->exit(imghan, EFI_SUCCESS, 0, NULL); }

/* From `boot.c` */
extern void 				printh(uint64_t value);
extern void 				memzero(uintptr_t from, uintptr_t to);
extern uintptr_t			palloc(int count);
extern int 				getmmap(void);

extern efi_system_table *		systab;	/* EFI system table */
extern efi_handle			imghan; /* EFI app image handle */
extern efi_boot_services *		bootsrv;/* boot services */
extern efi_file_protocol *		kfile;  /* kernel file handle */
extern efi_graphics_output_protocol *	gop;    /* Graphics Output Protocol */
extern uint64_t				mmapkey;/* EFI mmap key */
extern struct kargtab 			kargtab;/* Kernel argument table */

/* From `er.s` */

/*
 * Gets access to paging, required to map the kernel.
 * - Ensures bit #16 of `cr0` is disabled.
 * - Returns the value of `cr3`
 */
extern uintptr_t 			pgaccess();

/*
 * Epilogue to the whole bootloader, the last phase.
 * - Clears paging cache so our kernel mapping takes effect
 * - Transitions from shitty EFI ABI to SysV ABI
 * - setup stack
 * - pass appropriate arguments to kernel entry and call it
 */
extern void				er(uintptr_t entry, uintptr_t kargtab);

/*
 * Second phase of bootloader:
 *  - Parse Elf headers, validate the kernel object has valid fields
 *  - Locate, load, and map loadable segments from kernel binary
 *  - Prepare to call kernel
 *  - Call kernel and pass it arguments
 */

/* ELF header structures used for parsing kernel executable */
static Elf64_Ehdr ehdr;         /* Elf header */
static Elf64_Phdr *phdrs;       /* Program headers */

/* Fresh PDPT used to map the kernel (pointer later inserted in PML4) */
static uint64_t *pdpt;

/* Reads Elf program headers */
static void
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
	if (s != EFI_SUCCESS)
		fatal(L"Failed pool allocation");

	kfile->set_position(kfile, ehdr.e_phoff);
	s = kfile->read(
		kfile,
		&bufsz,
		phdrs
	);
	if (s != EFI_SUCCESS)
		fatal(L"Failed file read on kernel binary");
}

/* Reads the Elf header */
static void
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
	if (s != EFI_SUCCESS)
		fatal(L"Failed file read on kernel binary");

	/*
	 * After reading Elf header perform checks on the data, make sure this
	 * is a valid executable for our kernel
	 */
	if ((ehdr.e_ident[EI_MAG0] != ELFMAG0)
	|| (ehdr.e_ident[EI_MAG1] != ELFMAG1)
	|| (ehdr.e_ident[EI_MAG2] != ELFMAG2)
	|| (ehdr.e_ident[EI_MAG3] != ELFMAG3))
		fatal(L"Invalid Elf magic number in kernel binary");

	if (ehdr.e_ident[EI_CLASS] != ELFCLASS64)
		fatal(L"Invalid Elf class in kernel binary");

	if (ehdr.e_ident[EI_DATA] != ELFDATA2LSB)
		fatal(L"Invalid Elf data type in kernel binary");

	if (ehdr.e_type != ET_EXEC)
		fatal(L"Invalid Elf object type in kernel binary");

	if (ehdr.e_machine != EM_X86_64)
		fatal(L"Invalid Elf machine type in kernel binary");
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

	if (ptr == 0) {
		ptr = palloc(1);
		if (ptr == 0)
			return NULL;
		memzero(ptr, ptr + (0x1000));
		currtab[index] = ptr | 3; /* Present, R/W */
	}

	/* Clear flags if present for, sanitize address */
	ptr = ((ptr >> 12) & 0x7FFFFFFFFF) << 12;
	return (uint64_t *) ptr;
}

/* Map virtual addy to physical addy. */
static void
mapaddr(uintptr_t virt, uintptr_t phys)
{
	uint16_t index;
	uint64_t *table;

	table = pdpt;

	table = pagetab_next(table, ((virt >> 30) & 0x1FF));
	if (table == NULL)
		fatal(L"Failed mapping "); printhln(virt);

	table = pagetab_next(table, ((virt >> 21) & 0x1FF));
	if (table == NULL)
		fatal(L"Failed mapping "); printhln(virt);
	
	/*
	 * Reached the page table, now set the address of the page frame with
	 * appropriate flags.
	 */
	index = ((virt >> 12) & 0x1FF);

	table[index] = phys | 3; /* Present, R/W */
}

/*
 * Load loadable segments from kernel binary into memory and populate new page
 * tables.
 */
static void
loadk(void)
{
	efi_status s;
	uintptr_t page;
	uint64_t pgs, sz;
	int i, j, loadable;

	pdpt = (uint64_t *) palloc(1);
	memzero((uintptr_t) pdpt, (uintptr_t) pdpt + 0x1000);

	for (i = 0; i < ehdr.e_phnum; i++) {
		if (phdrs[i].p_type != PT_LOAD)
			continue;

		/* Allocate the amount of pages we need to load this segment */
		if (phdrs[i].p_memsz < 0x1000)
			pgs = 1;
		else
			pgs = phdrs[i].p_memsz / 0x1000;

		s = bootsrv->allocate_pages(
			allocate_any_pages,
			efi_runtime_services_code,
			pgs,
			(uint64_t *) &page
		);
		if (s != EFI_SUCCESS)
			fatal(L"Failed to allocate memory for kernel");

		/* Read segment content from file into pages if necessary */
		if (phdrs[i].p_filesz != 0) {
			s = kfile->set_position(kfile, phdrs[i].p_offset);
			sz = phdrs[i].p_filesz;
			s = kfile->read(
				kfile,
				&sz,
				(void *) page
			);
			if (s != EFI_SUCCESS)
				fatal(L"Failed to read kernel from boot "
						"media");
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
}

void
load(void)
{
	efi_status s;
	uintptr_t cr3;
	uint64_t *pml4;
	uintptr_t val;

	/* Read the Elf header and verify its validity */
	read_ehdr();

	/* Read program headers to be analyzed */
	read_phdrs();

	/* 
	 * Allocate pages, load kernel into memory, and populate new page
	 * tables
	 */
	loadk();

	/*
	 * Retrieve memory map from EFI and exit EFI boot services
	 * From this point forward we own the machine
	 */
	getmmap();
	bootsrv->exit_boot_services(imghan, mmapkey);

	/*
	 * Get paging access. Ensures bit 16 in `cr0` is off and returns the
	 * value of `cr3` so we can access the PML4 to link in our new mappings.
	 */
	cr3 = pgaccess();

	/* Install our mappings */
	pml4 = (uint64_t *)((cr3 >> 12) << 12);
	pml4[((ehdr.e_entry >> 39) & 0x1FF)] = (((uintptr_t)pdpt) | 3);
	
	/*
	 * And.... We're outta here.
	 * Setup the call to our kernel
	 */
	er(ehdr.e_entry, (uintptr_t) &kargtab);
}

