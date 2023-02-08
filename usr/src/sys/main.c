/*
 * `main.c` -- kernel entry point
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <efi.h>
#include <sys/kargtab.h>
#include <sys/syscon.h>
#include <sys/pmm.h>

extern uintptr_t *kbase;	/* Kernel base address defined in `link.ld`. */

void
main(struct kargtab *kargtab)
{
	syscon_init(kargtab);	/* Initialize system console and dependencies */
	kprintf("ALIX...\n");	/* We can talk */

	if (kargtab != NULL)
		kprintf("Received arguments from bootloader\n");
	kprintf("Kernel loaded at %lx\n", &kbase);

	pmm_init(kargtab);	/* Physical memory manager */	
        
	for(;;);
}

