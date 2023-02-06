/*
 * `uart.h` -- UART/Serial communication (mainly for QEMU).
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef _UART_H_
#define _UART_H_

void	uart_init();
void	uart_putc(char ch);

#endif /* _UART_H_ */

