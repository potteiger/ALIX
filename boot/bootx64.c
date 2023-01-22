/*
 * `bootx64.c` -- x86-64 EFI boot
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>

#include <elf.h>
#include <boot/efi.h>

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

static efi_system_table *		systab;		/* EFI system table */
static efi_boot_services *		boot_services;	/* boot services */
static efi_loaded_image_protocol *	img_protocol;	/* our EFI app image */
static efi_file_protocol *		filesystem;	/* root of filesystem */
static efi_file_protocol *		kernel_file;	/* kernel file */

/*
 * Prints a unsigned 64-bit value in hexadecimal (a memory address?)
 */

static void
hprint(uint64_t addr)
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
kernel_size()
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
 * Access boot device filesystem and attempt to locate kernel
 */

static efi_status
kernel_find(efi_handle img_handle)
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
		(int16_t *) L"bone.elf",
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
bootx64(efi_handle img_handle, efi_system_table *st)
{
	efi_status 			s;
	uint64_t 			ksz;

	systab = st;
	boot_services = systab->boot_services;

	clear();
	print(L"Bone boot...\r\n");

	if ((s = kernel_find(img_handle)) != EFI_SUCCESS) {
		print(L"No kernel\r\n");
		return 0;
	}

	print(L"Located kernel on boot device...\r\n");

	ksz = kernel_size();

	print(L"Size of kernel: ");
	hprint(ksz);
	print(L"\r\n");

	for(;;);

	return 0;
}

