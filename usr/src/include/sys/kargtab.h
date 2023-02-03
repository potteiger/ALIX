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
 * runtime. All pointers are virtual addresses mapped by the bootloader
 */
struct kargtab {

	uintptr_t	this;

	/*
	 * Framebuffer
	 */
	struct {

		uint32_t 	horizontal;	/* dimension in pixels */
		uint32_t 	vertical;	/* dimension in pixels */
		uintptr_t 	base;		/* base address */
		uint64_t	size;		/* total size */

		/*
		 * Console font
		 */
		struct {
			
			uintptr_t	base;
			uint64_t	size;

		} font;

	} fb;

	uintptr_t 	memory_map;	/* EFI memory map */
	uintptr_t	pml4_virt;	/* PML4 table */

};

#endif /* _KARGTAB_H_ */

