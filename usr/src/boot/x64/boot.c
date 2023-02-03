/*
 * `boot.c` -- ALIX bootloader (x86-64 EFI), boot phase
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

/* EFI console macros */
#define print(string) systab->console_out->output_string(systab->console_out,\
						(int16_t *) string)
#define println(string) { print(string); print(L"\r\n"); }
#define printhln(val) { printh(val); print(L"\r\n"); }
/* Perhaps this will be done more 'proper' later... EFI is just so damn dumb */
#define fatal(string) { print(string); \
			bootsrv->exit(imghan, EFI_SUCCESS, 0, NULL); }
#define clear() systab->console_out->clear_screen(systab->console_out)

/*
 * First phase of bootloader:
 *  - Organize/save/obtain crucial items from EFI.
 *  - Define basic procedures.
 *  - Locate kernel on boot media.
 *  - Initialize Graphics Output Protocol and obtain framebuffer.
 *  - Populate relevant items in `kargtab` for the kernel.
 */

/* Enter load phase in `load.c` */
extern void			load(void);

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
 * Kernel argument table: Data and pointers required by the kernel.
 * This structure is populated throughout the execution of the bootloader.
 * A pointer to this structure is passed to the kernel.
 */
struct kargtab kargtab;

/* Prints a unsigned 64-bit value in hexadecimal (memory addresses mainly) */
void
printh(uint64_t value)
{
	static int16_t hex[17];
	uint64_t work;
	int i;

	for (i = 15; i >= 0; i--) {
		if (i != 15)
			value = value >> 4;

		work = 0x000000000000000f & value;
		if (work < 10)
			hex[i] = work + '0';
		else
			hex[i] = (work - 10) + 'A';
	}

	print(L"0x");
	print(hex);
}

/*
 * Access boot device filesystem and attempt to locate kernel.
 * Leaves file handle in `kfile` for the load phase.
 */
static int
findk(efi_handle img_handle)
{
	efi_status s;
	efi_guid guid;
	efi_simple_file_system_protocol* sfsp;
	efi_handle devhandle;

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
	if (s != EFI_SUCCESS) {
		fatal(L"Failed to access boot media");
		return 1;
	}

	guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
	s = bootsrv->open_protocol(
		devhandle,
		&guid,
		(void **) &sfsp,
		img_handle,
		0,
		EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
	);
	if (s != EFI_SUCCESS) {
		fatal(L"Failed to access boot media");
		return 1;
	}

	s = sfsp->open_volume(sfsp, &filesys);
	if (s != EFI_SUCCESS) {
		fatal(L"Failed to access boot media");
		return 1;
	}

	println(L"Accessed boot media");

	s = filesys->open(
		filesys,
		&kfile,
		(int16_t *) L"alix.sys",
		EFI_FILE_MODE_READ,
		0
	);
	if (s != EFI_SUCCESS) {
		fatal(L"Failed to locate kernel on boot media");
		return 1;
	}

	println(L"Located kernel on boot media");
	return 0;
}

/* Initialize Graphics Output Protocol. Populate entries in `kargtab`. */
static int
init_gop(void)
{
	efi_guid guid;
	efi_status s;

	guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	s = bootsrv->locate_protocol(
		&guid,
		NULL,
		(void **) &gop
	);
	if (s != EFI_SUCCESS) {
		fatal(L"Failed to access Graphics Output Protocol");
		return 1;
	}

	kargtab.fb.base = gop->mode->framebuffer_base;
	kargtab.fb.size = gop->mode->framebuffer_size;

	println(L"Initialized Graphics Output Protocol");
	print(L"Located framebuffer at address ");
	printhln(kargtab.fb.base);

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

/* Allocate requested amount of (contiguous) pages. */
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
		fatal(L"Failed to allocate page");
		return 0;
	}

	return ptr;
}

/* Retrieve the EFI memory map. Populate entry in `kargtab`. */
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
	if (s != EFI_SUCCESS) {
		fatal(L"Failed to allocate memory for EFI memory map");
		return 1;
	}

	s = bootsrv->get_memory_map(
		&sz,
		mmap,
		&mmapkey,
		&dsz,
		&ver
	);
	if (s != EFI_SUCCESS) {
		fatal(L"Failed to obtain EFI memory map");
		return 1;
	}

	kargtab.memory_map = (uintptr_t) mmap;
	return 0;
}

void
boot(efi_handle img_handle, efi_system_table *st)
{
	efi_status s;

	imghan = img_handle;
	systab = st;
	bootsrv = systab->boot_services;

	clear();
	println(L"ALIX Bootloader:");

	/* Locate kernel on boot media */
	findk(img_handle);

	/* Initialize GOP */
	init_gop();

	/* Enter kernel load phase */
	load();
}

