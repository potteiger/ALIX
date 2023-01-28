/*
 * `main.c` -- kernel entry point
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>

/*
 * Simple procedure to test loader functionality
 */
uint16_t *message = L"Loaded!";
uint16_t *
main()
{
	return message;
}

