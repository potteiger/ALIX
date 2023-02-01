/*
 * `boot.c` -- x86-64 EFI boot
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>
#include <stddef.h>

#include <efi.h>
#include <sys/kargtab.h>

#define print(string) systab->console_out->output_string(systab->console_out,\
						(int16_t *) string)
#define clear() systab->console_out->clear_screen(systab->console_out)

efi_system_table *		systab;		/* EFI system table */
efi_handle 			imghand;	/* EFI app image handle */
efi_loaded_image_protocol *	imgpro;		/* EFI image protocol inteface*/
efi_boot_services *		bootsrv;	/* boot services */
efi_file_protocol *		filesys;	/* root of filesystem */
efi_file_protocol *		kfile;		/* kernel file handle */
efi_graphics_output_protocol *	gop;		/* Graphics Output Protocol */
uint64_t			mmap_key;	/* EFI mmap key */

extern void setcr3(uintptr_t);
extern uintptr_t getrsp();

struct kargtab kargtab;

extern efi_status 	load(void);	/* Enter load phase in `load.c` */

/*
 * Prints a unsigned 64-bit value in hexadecimal (a memory address?)
 */
void
printh(uint64_t addr)
{
	static int16_t hex[17];
	uint64_t work;
	int i;

	for (i = 15; i >= 0; i--) {
		if (i != 15)
			addr = addr >> 4;

		work = 0x000000000000000f & addr;
		if (work < 10)
			hex[i] = work + '0';
		else
			hex[i] = (work - 10) + 'A';
	}

	print(L"0x");
	print(hex);
}

/*
 * Access boot device filesystem and attempt to locate kernel
 * Leaves file handle in `kfile` for the load phase
 */
static int
kfind(efi_handle img_handle)
{
	efi_status s;
	efi_guid guid;
	efi_simple_file_system_protocol* sfsp;
	efi_handle devhandle;


	/*
	 * Open loaded image protocol to obtain the device handle
	 */
	guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	s = bootsrv->open_protocol(
		img_handle,
		&guid,
		(void **) &imgpro,
		img_handle,
		0,
		EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
	);
	devhandle = imgpro->device_handle;

	if (s != EFI_SUCCESS)
		return 1;

	/*
	 * Open simple filesystem protocol on the device we were booted from
	 */
	guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
	s = bootsrv->open_protocol(
		devhandle,
		&guid,
		(void **) &sfsp,
		img_handle,
		0,
		EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
	);

	if (s != EFI_SUCCESS)
		return 1;

	/*
	 * Open the volume to obtain the file protocol
	 */
	s = sfsp->open_volume(sfsp, &filesys);
	if (s != EFI_SUCCESS)
		return 1;
	print(L"Accessed boot device\r\n");

	/*
	 * Open file
	 */
	s = filesys->open(
		filesys,
		&kfile,
		(int16_t *) L"bonex64.sys",
		EFI_FILE_MODE_READ,
		0
	);
	if (s != EFI_SUCCESS)
		return 1;

	print(L"Located kernel on boot device\r\n");
	return 0;
}

/*
 * Initialize Graphics Output Protocol
 */
static int
gopinit(void)
{
	efi_guid guid;
	efi_status s;
	uint64_t sz;
	efi_graphics_output_mode_information *info;
	int i, max;

	/*
	 * Access GOP
	 */
	guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	s = bootsrv->locate_protocol(
		&guid,
		NULL,
		(void **) &gop
	);

	/*
	 * TODO: verification
	 */

	print(L"Initialized GOP and retrieved framebuffer\r\n");

	kargtab.fb.base = gop->mode->framebuffer_base;
	kargtab.fb.size = gop->mode->framebuffer_size;

	return 0;
}

/*
 * Zero memory from and to
 */
void
memzero(uintptr_t from, uintptr_t to)
{
	for (; from <= to; from++) {
		*((uint8_t*) from) = 0;
	}
}

/*
 * Allocate requested amount of pages
 */
uintptr_t
palloc(int count)
{
	efi_status s;
	uintptr_t ptr;

	s = bootsrv->allocate_pages(
		allocate_any_pages,
		efi_runtime_services_data,
		count,
		&ptr
	);
	if (s != EFI_SUCCESS) {
		print(L"Failed to allocate page\r\n");
		return 0;
	}

	return ptr;
}

/*
 * Retrieve the EFI memory map
 */
int
getmmap()
{
	efi_status s;
	uint64_t sz, dsz;
	uint32_t ver;
	int i;
	efi_memory_descriptor *mmap;

	mmap = NULL;

	sz = 0;
	s = bootsrv->get_memory_map(
		&sz,
		mmap,
		&mmap_key,
		&dsz,
		&ver
	);

	sz += (dsz * 2);
	s = bootsrv->allocate_pool(
		efi_runtime_services_data,
		sz,
		(void **) &mmap
	);

	s = bootsrv->get_memory_map(
		&sz,
		mmap,
		&mmap_key,
		&dsz,
		&ver
	);

	kargtab.memory_map = (uintptr_t) mmap;
	return 0;
}

/*
 * Map virtual addy to physical addy
 */
int
map(uintptr_t virt, uintptr_t phys)
{
	uint16_t pml4_off, pdpt_off, pd_off, pt_off;
	uint64_t *table;
	uint64_t work;

	pml4_off = (uint16_t)((virt >> 39) & 0x1FF);
	pdpt_off = (uint16_t)((virt >> 30) & 0x1FF);
	pd_off   = (uint16_t)((virt >> 21) & 0x1FF);
	pt_off   = (uint16_t)((virt >> 12) & 0x1FF);

	print(L"mapping: "); printh(virt); print(L"\r\n");

	table = (uint64_t *) kargtab.pml4_virt;
	work = (((table[pml4_off] >> 12) & 0x7FFFFFFFFF) << 12);
	if (work == 0) {
		if ((work = (uint64_t) palloc(1)) == 0)
			return 1;
		memzero(work, work + 0x1000);
		table[pml4_off] = (work | 3);
	}

	table = (uint64_t *) work;
	work = (((table[pdpt_off] >> 12) & 0x7FFFFFFFFF) << 12);
	if (work == 0) {
		if ((work = (uint64_t) palloc(1)) == 0)
			return 1;
		memzero(work, work + 0x1000);
		table[pdpt_off] = (work | 3);
	}

	table = (uint64_t *) work;
	work = (((table[pd_off] >> 12) & 0x7FFFFFFFFF) << 12);
	if (work == 0) {
		if ((work = (uint64_t) palloc(1)) == 0)
			return 1;
		memzero(work, work + 0x1000);
		table[pd_off] = (work | 3);
	}

	table = (uint64_t *) work;
	table[pt_off] = phys | 3;

	return 0;
}

/*
 * Create initial page tables and mappings as required by the kernel.
 * At this stage we map ourselves (the bootloader cause it contains `kargtab`),
 * the framebuffer, and the loaded console font. 
 */
static int
init_mappings(void)
{
	uintptr_t base;
	uint64_t sz;
	uint32_t i;

	/*
	 * Allocate and zero a page for the PML4 and identity map it
	 */
	kargtab.pml4_virt = palloc(1);
	memzero(kargtab.pml4_virt, kargtab.pml4_virt + 0x1000);
	map(kargtab.pml4_virt, kargtab.pml4_virt);

	/*
	 * Identity map EFI app image (this bootloader)
	 */
	base = (uintptr_t) imgpro->image_base;
	sz = imgpro->image_size;
	i = (sz / 0x1000)-1;
	for (; i > 0; i--)
		map(base + (0x1000 * i), base + (0x1000 * i));
	map(base, base);

	/*
	 * Identity map the framebuffer
	 */
	base = (uintptr_t) kargtab.fb.base;
	sz = kargtab.fb.size;
	i = (sz / 0x1000)-1;
	printh(i);
	for (; i > 0; i--)
		map(base + (0x1000 * i), base + (0x1000 * i));
	map(base, base);

	return 0;
}

/*
 * x86-64 EFI boot entry point
 */
efi_status
boot(efi_handle img_handle, efi_system_table *st)
{
	int s, i;
	uint64_t cr3;
	uint64_t ptr;
	uint32_t *fb;

	imghand = img_handle;
	systab = st;
	bootsrv = systab->boot_services;

	clear();
	print(L"Bone boot...\r\n");

	/*
	 * Locate kernel on boot media
	 */
	if (kfind(img_handle) != 0) {
		print(L"No kernel\r\n");
		return 0;
	}

	/*
	 * Initialize GOP
	 */
	if (gopinit() != 0)
		return 0;

	/*
	 * Perform initial page mappings required by the kernel
	 */
	init_mappings();

	/*
	 * Enter load phase (and hopefully never return)
	 */
	return load();
}

