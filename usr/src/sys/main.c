/*
 * ALIX: `sys/main.c` -- kernel entry point
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
#include <sys/x64/gdt.h>
#include <sys/dev/console.h>

extern uintptr_t *kbase;	/* Kernel base address defined in `link.ld`. */

void
main(struct kargtab *kargtab)
{
	console_init(kargtab);
	kprintf("ALIX...\n");
	kprintf("Kernel loaded at %lx\n", &kbase);

	gdt_init();
        
	for(;;);
}

