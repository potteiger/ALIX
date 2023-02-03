/*
 * `fb.h` -- framebuffer
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef _FB_H_
#define _FB_H_

extern const struct Framebuffer {

	uintptr_t	base;		/* base address of framebuffer */
	uint64_t	size;		/* size of framebuffer (in pixels) */
	uint32_t	width;		/* width in pixels */
	uint32_t	height;		/* height in pixels */
	uint32_t	scanlinepx;	/* pixels per scan line */

} FRAMEBUFFER;

/*
 * Populate `FRAMEBUFFER` with information from UEFI GOP
 */
void 	init_fb(struct kargtab *kargtab);

#endif /* _FB_H_ */

