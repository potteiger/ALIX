/*
 * `boot.c` -- x86-64 EFI bootloader, boot phase
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

/*
 * Enter the load phase in `load.c`
 */
extern efi_status		load(void);

extern uintptr_t		getcr3(void);

/* EFI handles, protocols, other data structures */
efi_system_table *		systab;		/* EFI system table */
efi_handle 			imghan;		/* EFI app image handle */
efi_loaded_image_protocol *	imgpro;		/* EFI image protocol inteface*/
efi_boot_services *		bootsrv;	/* boot services */
efi_file_protocol *		filesys;	/* root of filesystem */
efi_file_protocol *		kfile;		/* kernel file handle */
efi_graphics_output_protocol *	gop;		/* Graphics Output Protocol */
uint64_t			mmapkey;	/* EFI mmap key */

/*
 * Kernel argument table.
 * Data and pointers required by the kernel, a pointer to this structure is
 * passed to the kernel.
 */
struct kargtab kargtab;

/* Prints a unsigned 64-bit value in hexadecimal (memory addresses mainly) */
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


	/* Open loaded image protocol to obtain the device handle */
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

	/* Open simple filesystem protocol on the device we were booted from */
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

	/* Open the volume to obtain the file protocol */
	s = sfsp->open_volume(sfsp, &filesys);
	if (s != EFI_SUCCESS)
		return 1;
	print(L"Accessed boot device\r\n");

	/* open file */
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

/* Initialize Graphics Output Protocol. */
static int
init_gop(void)
{
	efi_guid guid;
	efi_status s;
	uint64_t sz;
	efi_graphics_output_mode_information *info;
	int i, max;

	/* Access GOP */
	guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	s = bootsrv->locate_protocol(
		&guid,
		NULL,
		(void **) &gop
	);

	gop->set_mode(gop, 0);

	kargtab.fb.base = gop->mode->framebuffer_base;
	kargtab.fb.size = gop->mode->framebuffer_size;

	print(L"Initialized GOP and retrieved framebuffer at ");
	printh(kargtab.fb.base); print(L"\r\n");
	print(L"Framebuffer size: "); printh(kargtab.fb.size); print(L"\r\n");
	return 0;
}

/* Zero memory from and to. */
void
memzero(uintptr_t from, uintptr_t to)
{
	for (; from <= to; from++) {
		*((uint8_t*) from) = 0;
	}
}

/* Allocate requested amount of pages. */
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

/* Retrieve the EFI memory map. */
int
getmmap()
{
	efi_status s;
	uint64_t sz, dsz;
	uint32_t ver;
	int i, count;
	efi_memory_descriptor *mmap;

	mmap = NULL;

	sz = 0;
	s = bootsrv->get_memory_map(
		&sz,
		mmap,
		&mmapkey,
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
		&mmapkey,
		&dsz,
		&ver
	);
	if (s != EFI_SUCCESS) {
		print(L"Failed to obtain memory map\r\n");
		return 1;
	}

	kargtab.memory_map = (uintptr_t) mmap;
	return 0;
}

/* x86-64 EFI bootloader boot phase entry point */
efi_status
boot(efi_handle img_handle, efi_system_table *st)
{
	int s, i;
	uintptr_t cr3;
	uint64_t ptr;
	uint32_t *fb;

	imghan = img_handle;
	systab = st;
	bootsrv = systab->boot_services;

	clear();
	print(L"Bone boot...\r\n");

	/* Locate kernel on boot media */
	if (kfind(img_handle) != 0) {
		print(L"No kernel\r\n");
		return 0;
	}

	/* Initialize GOP */
	if (init_gop() != 0)
		return 0;

	/* Enter load phase (and hopefully never return) */
	return load();
}

