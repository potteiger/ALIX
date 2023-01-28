/*
 * `boot.c` -- x86-64 EFI boot
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>

#include <elf.h>
#include <efi.h>

#define print(string) systab->console_out->output_string(systab->console_out,\
						(int16_t *) string)

#define clear() systab->console_out->clear_screen(systab->console_out)

/*
 * String table of memory types
 */
static const uint16_t *memory_type[] = {
	[efi_reserved_memory_type]	= L"Reserved",
	[efi_loader_code]		= L"Loader code",
	[efi_loader_data]		= L"Loader data",
	[efi_boot_services_code]	= L"Boot services code",
	[efi_boot_services_data]	= L"Boot services data",
	[efi_runtime_services_code]	= L"Runtime services code",
	[efi_runtime_services_data]	= L"Runtime services data",
	[efi_conventional_memory]	= L"Conventional memory",
	[efi_unusable_memory]		= L"Unusable memory",
	[efi_ACPI_reclaim_memory]	= L"ACPI reclaim memory",
	[efi_ACPI_memoryNVS]		= L"ACPI memory NVS",
	[efi_memory_mapped_IO]		= L"Memory mapped I/O",
	[efi_memory_mapped_IO_port_space]= L"Memory mapped I/O port space",
	[efi_pal_code]			= L"Pal code",
	[efi_persistent_memory]		= L"Persistent memory",
	[efi_unaccepted_memory_type]	= L"Unaccepted memory type",
	[efi_max_memory_type]		= L"Max memory type"

};

efi_system_table *		systab;		/* EFI system table */
efi_boot_services *		boot_services;	/* boot services */
efi_loaded_image_protocol *	img_protocol;	/* our EFI app image */
efi_file_protocol *		filesystem;	/* root of filesystem */

efi_file_protocol *		kernel_file;	/* kernel file */

static efi_memory_descriptor *		mmap;		/* memory map */
static uint64_t				mmap_size;
static uint64_t				mdesc_size;
static uint64_t				mkey;

/* load.c */
extern int load();

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
 * Retrieve memory map from EFI
 */
static int
getmmap()
{
	efi_status s;
	uint32_t ver;
	
	/*
	 * Call get_memory_map() with bufsz 0 to get the required size
	 */
	mmap_size = 0;
	mmap = 0;
	s = boot_services->get_memory_map(
		&mmap_size,
		mmap,
		&mkey,
		&mdesc_size,
		&ver
	);

	/*
	 * Allocate a pool large enough for the memory map (+2 descriptor sizes
	 * to account for this allocation)
	 */
	mmap_size += mdesc_size * 2;
	s = boot_services->allocate_pool(
		efi_loader_data,
		mmap_size,
		(void **) &mmap
	);

	if (s != EFI_SUCCESS)
		return 1;

	/*
	 * Now call for a memory map with everything we need
	 */
	s = boot_services->get_memory_map(
		&mmap_size,
		mmap,
		&mkey,
		&mdesc_size,
		&ver
	);

	if (s != EFI_SUCCESS)
		return 1;

	return 0;
}

/*
 * Access boot device filesystem and attempt to locate kernel
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
	s = boot_services->open_protocol(
		img_handle,
		&guid,
		(void **) &img_protocol,
		img_handle,
		0,
		EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
	);
	devhandle = img_protocol->device_handle;

	if (s != EFI_SUCCESS)
		return 1;

	/*
	 * Open simple filesystem protocol on the device we were booted from
	 */
	guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
	s = boot_services->open_protocol(
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
	s = sfsp->open_volume(sfsp, &filesystem);

	if (s != EFI_SUCCESS)
		return 1;
	print(L"Accessed boot device...\r\n");

	/*
	 * Open file
	 */
	s = filesystem->open(
		filesystem,
		&kernel_file,
		(int16_t *) L"bonex64.sys",
		EFI_FILE_MODE_READ,
		0
	);

	if (s != EFI_SUCCESS)
		return 1;

	return 0;
}

/*
 * x86-64 EFI boot entry point
 */
efi_status
boot(efi_handle img_handle, efi_system_table *st, void *pagedir)
{
	int s;

	systab = st;
	boot_services = systab->boot_services;

	clear();
	print(L"Bone boot...\r\n");

	/*
	 * Locate kernel on boot media
	 */
	if (kfind(img_handle) != 0)
		print(L"No kernel\r\n");
	print(L"Located kernel on boot device...\r\n");

	/*
	 * Attempt to load kernel into memory
	 */
	s = load();
	if (s != 0)
		print(L"Failed to load kernel\r\n");

	/*
	 * Retrieve our memory map
	 */
	if (getmmap() != 0)
		print(L"Cannot obtain memory map from EFI\r\n");

	for(;;);

	return EFI_SUCCESS;
}

