/*
 * ALIX: `sys/x64/gdt.c` -- x64 Global Descriptor Table
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>

#include <sys/x64/gdt.h>

/* Global Descriptor table */
Segdesc GDT[6];

/*
 * Initialize the Global Descriptor Table
 */
void
gdt_init(void)
{
	/* Null descriptor. */
	GDT[0] = 0;

	/* Kernel code segment. */
	GDT[1] = SEGDESC_TYPE(1) | SEGDESC_RW(1) | SEGDESC_DPL(0)
		| SEGDESC_PRES(1) | SEGDESC_LONG(1);
	/* Kernel data segment. */
	GDT[2] = SEGDESC_TYPE(1) | SEGDESC_RW(1) | SEGDESC_DPL(0)
		| SEGDESC_PRES(1) | SEGDESC_LONG(1);

	/* User code segment. */
	GDT[3] = SEGDESC_TYPE(1) | SEGDESC_RW(1) | SEGDESC_DPL(3)
		| SEGDESC_PRES(1) | SEGDESC_LONG(1);
	/* User data segment. */
	GDT[4] = SEGDESC_TYPE(1) | SEGDESC_RW(1) | SEGDESC_DPL(3)
		| SEGDESC_PRES(1) | SEGDESC_LONG(1);

	return;	
}

