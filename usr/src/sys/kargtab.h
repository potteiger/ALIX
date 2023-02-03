/*
 * `kargtab.h` -- Structure required by kernel from bootloader
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef _KARGTAB_H_
#define _KARGTAB_H_

/*
 * Contains data required by the kernel, a pointer to it is passed to it at
 * runtime. All pointers are mapped virtual addresses (likely identity maps
 * remaining from UEFI).
 */
struct kargtab {

	uintptr_t	this;		/* pointer to this structure */
	uintptr_t	memory_map;	/* UEFI memory map */
	uintptr_t	gop_mode;	/* GOP mode/info (Framebuffer access) */
	uintptr_t	font_base;	/* Base address of loaded console font*/
	uint64_t	font_size;	/* Size of loaded console font */

};

#endif /* _KARGTAB_H_ */

