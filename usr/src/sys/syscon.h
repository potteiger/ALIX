/*
 * `syscon.h` -- System console
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef _SYSCON_H_
#define _SYSCON_H_

void	init_syscon(struct kargtab *kargtab);
void 	syscon_write(char *string);

#endif

