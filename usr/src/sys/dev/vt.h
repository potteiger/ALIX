/*
 * ALIX: `sys/dev/vt.h` -- Virtual (framebuffer) terminal
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef _VT_H_
#define _VT_H_

void	vt_init(struct kargtab *kargtab);
void	vt_putc(char ch);

#endif /* _VT_H_ */

