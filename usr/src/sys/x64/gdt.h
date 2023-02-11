/*
 * ALIX: `sys/x64/gdt.h` -- x64 Global Descriptor Table
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef _X64_GDT_H_
#define _X64_GDT_H_

/* A segment descriptor is 8 bytes */
typedef uint64_t Segdesc;

/*
 * Macros to setup segment descriptors
 * Bitwise OR these values together
 */

/* 0=system, 1=code/data */
#define SEGDESC_TYPE(x) (((Segdesc) x << 12) << 32)
/* data: 1=write enable, code: 1=read enable */
#define SEGDESC_RW(x)	(((Segdesc) x << 9) << 32)
/* Privilege level */
#define SEGDESC_DPL(x)	(((Segdesc) x << 13) << 32)
/* Segment present */
#define SEGDESC_PRES(x)	(((Segdesc) x << 15) << 32)
/* Long mode? 1=yes */
#define SEGDESC_LONG(x) (((Segdesc) x << 21) << 32)

void	gdt_init(void);

#endif /* _X64_GDT_H_ */

