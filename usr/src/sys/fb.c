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

struct Framebuffer {

	uintptr_t	base;		/* base address of framebuffer */
	uint64_t	size;		/* size of framebuffer (in pixels) */
	uint32_t	width;		/* width in pixels */
	uint32_t	height;		/* height in pixels */
	uint32_t	scanlinepx;	/* pixels per scan line */

} FRAMEBUFFER;

/*
 * Populate `FRAMEBUFFER` with information from UEFI GOP
 */
void
init_fb(struct kargtab *kargtab)
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
}

