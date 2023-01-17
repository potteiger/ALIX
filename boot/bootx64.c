/*
 * bootx64.c -- x86-64 EFI boot
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <boot/efi.h>

/*
 * x86-64 EFI boot entry point
 */

uint64_t
bootx64(void *img_handle, EFI_SYSTEM_TABLE *systab)
{

	/*
	 * Clears the console then prints "Bones!" and hangs.
	 * State of the art shit.
	 */

	systab->console_out->clear_screen(systab->console_out);
	systab->console_out->output_string(systab->console_out,
			(int16_t *)L"Bones!");

	for(;;);

	return 0;
}

