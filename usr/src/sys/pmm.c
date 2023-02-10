/*
 * ALIX: `sys/pmm.c` -- Physical Memory Manager
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>
#include <stddef.h>

#include <efi.h>
#include <sys/kargtab.h>
#include <sys/dev/console.h>

#define PAGE_SIZE	0x1000

/* UEFI Memory Map */
static efi_memory_descriptor *	mmap;		/* Root descriptor */
static uint64_t			mmap_sz;	/* Total mmap size */
static uint64_t 		mmap_dsz;	/* Size of single descriptor */

void
pmm_init(struct kargtab *kargtab)
{
	mmap = (efi_memory_descriptor *) kargtab->mmap;
	mmap_sz = kargtab->mmap_sz;
	mmap_dsz = kargtab->mmap_dsz;
	
	
}

