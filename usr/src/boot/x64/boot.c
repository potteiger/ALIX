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
 * First phase of bootloader
 * - Accepts EFI data structures and sets them up for the loader
 * - Verifies kernel is on boot media and obtains a EFI file handle for it
 * - enters load phase
 */

#define print(string) systab->console_out->output_string(systab->console_out,\
						(int16_t *) string)

#define clear() systab->console_out->clear_screen(systab->console_out)

efi_system_table *		systab;		/* EFI system table */
efi_boot_services *		bootsrv;	/* boot services */
efi_loaded_image_protocol *	imgpro;		/* our EFI app image */
efi_file_protocol *		filesys;	/* root of filesystem */
efi_file_protocol *		kfile;		/* kernel file handle */
void *				pagetabs;	/* new page tables */

/*
 * Enter load phase in `load.c`
 */
extern efi_status load(void);

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
 * Leaves file handle in `kfile` for the loader
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
	print(L"Accessed boot device...\r\n");

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

	return 0;
}

/*
 * x86-64 EFI boot entry point
 */
efi_status
boot(efi_handle img_handle, efi_system_table *st)
{
	int s;

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
	print(L"Located kernel on boot device...\r\n");

	/*
	 * Enter load phase (and hopefully never return)
	 */
	return load();
}

