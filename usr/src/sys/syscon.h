/*
 * `syscon.h` -- System console interface
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef _SYSCON_H_
#define _SYSCON_H_

void	syscon_init(struct kargtab *kargtab);
void	kputc(char ch);
void	kputs(char *string);
void	kprintf(const char *fmt, ...);
void 	syscon_write(void *buf, size_t sz);

#endif

