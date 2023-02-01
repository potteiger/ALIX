/*
 * `main.c` -- kernel entry point
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>

#include <sys/kargtab.h>

void
main(struct kargtab *kargtab)
{
        int i;
        uint32_t *fb;

        /*
         * Paint screen a lovely brown
         */
	fb = (uint32_t *) kargtab->fb.base;
        for (i = 0; i < kargtab->fb.size; i++) {
                fb[i] = 0xff3b3228;
        }

        for(;;);
}

