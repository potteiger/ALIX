/*
 * ALIX: `boot/boot.c` -- ALIX bootloader (x86-64 EFI), boot phase
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
#define fatal(string) { println(string); \
			println(L"Exiting in 10 seconds..."); \
			bootsrv->stall(10000000);\
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
Efi_system_table *		systab;		/* EFI system table */
Efi_handle 			imghan;		/* EFI app image handle */
Efi_loaded_image_protocol *	imgpro;		/* EFI image protocol inteface*/
Efi_boot_services *		bootsrv;	/* boot services */
Efi_file_protocol *		filesys;	/* root of filesystem */
Efi_file_protocol *		kfile;		/* kernel file handle */
Efi_graphics_output_protocol *	gop;		/* Graphics Output Protocol */
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
	Efi_status s;
	uintptr_t ptr;

	s = bootsrv->allocate_pages(
		Efi_allocate_any_pages,
		Efi_runtime_services_data,
		count,
		&ptr
	);
	if (s != EFI_SUCCESS)
		fatal(L"Failed to allocate page");

	return ptr;
}

/* Initializes boot media filesystem access */
static void
filesys_init()
{
	Efi_status s;
	Efi_guid guid;
	Efi_simple_file_system_protocol * sfsp;
	Efi_handle devhandle;

	guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	s = bootsrv->open_protocol(
		imghan,
		&guid,
		(void **) &imgpro,
		imghan,
		0,
		EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
	);
	devhandle = imgpro->device_handle;
	if (s != EFI_SUCCESS)
		fatal(L"Failed to access boot media");

	guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
	s = bootsrv->open_protocol(
		devhandle,
		&guid,
		(void **) &sfsp,
		imghan,
		0,
		EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
	);
	if (s != EFI_SUCCESS)
		fatal(L"Failed to access boot media");

	s = sfsp->open_volume(sfsp, &filesys);
	if (s != EFI_SUCCESS)
		fatal(L"Failed to access boot media");

	println(L"Accessed boot media");
}

/* Attempt to locate kernel and leave file handle in `kfile` for load phase. */
static void
find_kernel()
{
	Efi_status s;
	
	s = filesys->open(
		filesys,
		&kfile,
		(int16_t *) L"alix.sys",
		EFI_FILE_MODE_READ,
		0
	);
	if (s != EFI_SUCCESS)
		fatal(L"Failed to locate kernel on boot media");

	println(L"Located kernel on boot media");
}

/* Locate and load console font for kernel and populate `kargtab` fields. */
static void
find_font()
{
	Efi_status s;
	Efi_guid guid;
	Efi_file_protocol *d, *f;
	Efi_file_info *info;
	uint64_t sz;
	uintptr_t pgs;

	/* Find font file */
	s = filesys->open(
		filesys,
		&d,
		(int16_t *) L"font",
		EFI_FILE_MODE_READ,
		0
	);
	if (s != EFI_SUCCESS)
		fatal(L"Failed to locate kernel console font on boot media");

	s = d->open(
		d,
		&f,
		(int16_t *) L"spleen-8x16.psfu",
		EFI_FILE_MODE_READ,
		0
	);
	if (s != EFI_SUCCESS)
		fatal(L"Failed to locate kernel console font on boot media");

	println(L"Located kernel console font on boot media");
	
	/* Determine file size */
	guid = EFI_FILE_INFO_ID;
	sz = 0;
	info = NULL;
	f->get_info(f, &guid, &sz, (void *) info);
	s = bootsrv->allocate_pool(
		Efi_runtime_services_data,
		sz,
		(void **) &info
	);
	if (s != EFI_SUCCESS)
		fatal(L"Memory allocation fail");

	s = f->get_info(
		f,
		&guid,
		&sz,
		(void *) info
	);
	if (s!= EFI_SUCCESS)
		fatal(L"Failed font file access");
	sz = info->file_size;

	/* allocate pages and read file */
	pgs = palloc((sz / 0x1000) + 1);

	s = f->read(
		f,
		&sz,
		(void *) pgs
	);
	if (s != EFI_SUCCESS)
		fatal(L"Failed to read font from boot media");

	println(L"Font read from boot media");
	kargtab.font_base = (uintptr_t) pgs;
	kargtab.font_size = sz;
}

/* Initialize Graphics Output Protocol. Populate entries in `kargtab`. */
static void
gop_init(void)
{
	Efi_guid guid;
	Efi_status s;

	guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	s = bootsrv->locate_protocol(
		&guid,
		NULL,
		(void **) &gop
	);
	if (s != EFI_SUCCESS)
		fatal(L"Failed to access Graphics Output Protocol");

	kargtab.gop_mode = (uintptr_t) gop->mode;

	println(L"Initialized Graphics Output Protocol");
}

/* Retrieve the EFI memory map. Populate entry in `kargtab`. */
void
getmmap()
{
	Efi_status s;
	uint64_t sz, dsz;
	uint32_t ver;
	int i, count;
	Efi_memory_descriptor *mmap;

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
		Efi_runtime_services_data,
		sz,
		(void **) &mmap
	);
	if (s != EFI_SUCCESS)
		fatal(L"Failed to allocate memory for EFI memory map");

	s = bootsrv->get_memory_map(
		&sz,
		mmap,
		&mmapkey,
		&dsz,
		&ver
	);
	if (s != EFI_SUCCESS)
		fatal(L"Failed to obtain EFI memory map");

	kargtab.mmap = (uintptr_t) mmap;
	kargtab.mmap_sz = (uint64_t) sz;
	kargtab.mmap_dsz = (uint64_t) dsz;
}

void
boot(Efi_handle img_handle, Efi_system_table *st)
{
	Efi_status s;

	imghan = img_handle;
	systab = st;
	bootsrv = systab->boot_services;

	clear();
	println(L"ALIX Bootloader...");
	
	filesys_init();	/* Access boot media filesystem */
	find_kernel();	/* Locate kernel on boot media */
	find_font();	/* Locate and load kernel console font */
	gop_init();	/* Initialize GOP and obtain framebuffer */

	kargtab.runtime_srv = (uintptr_t) systab->runtime_services;

	/* Enter kernel load phase */
	load();
}

