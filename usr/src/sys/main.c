/*
 * `main.c` -- kernel entry point
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>

#include <efi.h>
#include <sys/kargtab.h>
#include <sys/syscon.h>

void
main(struct kargtab *kargtab)
{
        uint64_t i;
        uint32_t *fb;
	
	init_syscon(kargtab);

	syscon_write("Peanut butter!");

        for(;;);
}

