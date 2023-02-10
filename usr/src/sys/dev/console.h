/*
 * ALIX: `sys/dev/console.h` -- System console device
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

void	console_init(struct kargtab *kargtab);
void	kputc(char ch);
void	kputs(char *string);
void	kprintf(const char *fmt, ...);
void 	console_write(void *buf, size_t sz);

#endif

