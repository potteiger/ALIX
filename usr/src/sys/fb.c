/*
 * `fb.c` -- framebuffer
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>

#include <efi.h>
#include <sys/kargtab.h>

#define _FB_C_
#include <sys/fb.h>

_Framebuffer FRAMEBUFFER;

/*
 * Populate `FRAMEBUFFER` with information from UEFI GOP
 */
void
fb_init(struct kargtab *kargtab)
{
	efi_graphics_output_protocol_mode *mode;

	mode = (efi_graphics_output_protocol_mode *) kargtab->gop_mode;

	FRAMEBUFFER = (struct Framebuffer) {
		.base = (uintptr_t) mode->framebuffer_base,
		.size = (uint64_t) mode->framebuffer_size,
		.width = mode->info->horizontal_resolution,
		.height = mode->info->vertical_resolution,
		.scanlinepx = mode->info->pixels_per_scan_line,
	};


	switch (mode->info->pixel_format) {

	case PixelRedGreenBlueReserved8BitPerColor:
		FRAMEBUFFER.redshift = 0;
		FRAMEBUFFER.redmask = 0x000000FF;

		FRAMEBUFFER.greenshift = 8;
		FRAMEBUFFER.greenmask = 0x0000FF00;

		FRAMEBUFFER.blueshift = 16;
		FRAMEBUFFER.bluemask = 0x00FF0000;
		break;
	
	/* TODO: Support custom pixel bitmasks from EFI, just in case */

	default:
	case PixelBlueGreenRedReserved8BitPerColor:
		FRAMEBUFFER.blueshift = 0;
		FRAMEBUFFER.bluemask = 0x000000FF;

		FRAMEBUFFER.greenshift = 8;
		FRAMEBUFFER.greenmask = 0x0000FF00;

		FRAMEBUFFER.redshift = 16;
		FRAMEBUFFER.redmask = 0x00FF0000;
		break;

	}
}

/* Returns `Color` comprised of all provided pieces */
Color
fb_color(Color red, Color green, Color blue)
{
	red = red << FRAMEBUFFER.redshift;
	red = red & FRAMEBUFFER.redmask;

	green = green << FRAMEBUFFER.greenshift;
	green = green & FRAMEBUFFER.greenmask;

	blue = blue << FRAMEBUFFER.blueshift;
	blue = blue & FRAMEBUFFER.bluemask;

	return (red | green) | blue;
}

