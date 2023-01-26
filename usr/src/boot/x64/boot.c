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

/*
 * UEFI is god awful (see further rant in header file) so I'm going to try my
 * best to be here for as short of a time as possible. Use EFI as little as
 * possible. Tasks:
 *
 * - boot message
 *
 * - setup execution environment for kernel: allocate space for it, obtain a
 *   memory map, obtain a framebuffer.
 *
 * - load kernel into memory
 *
 * - gtfo: exit_boot_services() and call the kernel
 */

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

/*
 * Prints a unsigned 64-bit value in hexadecimal (a memory address?)
 */
static void
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
 * Return the size of the kernel on boot media
 */
static uint64_t
ksize()
{
	uint8_t buff[256];
	efi_status s;
	uint64_t bufsz;
	efi_file_info *info;
	efi_guid guid;

	guid = EFI_FILE_INFO_ID;
	bufsz = 256;

	s = kernel_file->get_info(
		kernel_file,
		&guid,
		&bufsz,
		(void *) buff
	);

	info = (efi_file_info*) buff;
	return info->file_size;
}

/*
 * Retrieve memory map from EFI
 */
static efi_status
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
	 * too account for this allocation)
	 */
	mmap_size += mdesc_size * 2;
	boot_services->allocate_pool(
		efi_loader_data,
		mmap_size,
		(void **) &mmap
	);

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

	return s;
}

/*
 * Access boot device filesystem and attempt to locate kernel
 */
static efi_status
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
		return s;

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
		return s;

	/*
	 * Open the volume to obtain the file protocol
	 */
	s = sfsp->open_volume(sfsp, &filesystem);

	if (s != EFI_SUCCESS)
		return s;

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
		return s;

	return 0;
}

/*
 * x86-64 EFI boot entry point
 */
efi_status
boot(efi_handle img_handle, efi_system_table *st, void *pagedir)
{
	efi_status 			s;
	uint64_t 			ksz;

	systab = st;
	boot_services = systab->boot_services;

	clear();
	print(L"Bone boot...\r\n");

	/*
	 * Locate kernel and it's size on boot media
	 */
	if ((s = kfind(img_handle)) != EFI_SUCCESS) {
		print(L"No kernel\r\n");
		return 0;
	}

	print(L"Located kernel on boot device...\r\n");

	ksz = ksize();

	print(L"Size of kernel: ");
	printh(ksz);
	print(L"\r\n");

	/*
	 * Retrieve our memory map
	 */
	if (getmmap() != EFI_SUCCESS) {
		print(L"Cannot obtain memory map from EFI\r\n");
		return 0;
	}

	/*
	 * Print the address of the page directory
	 */
	printh((uint64_t) pagedir);

	for(;;);

	return 0;
}

