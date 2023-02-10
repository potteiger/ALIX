/*
 * ALIX: `sys/dev/uart.c` -- UART/Serial communication (mainly for QEMU).
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>

#include <sys/x64/io.h>

/* COM1 Port number */
#define COM1 0x3F8

/*
 * Setup COM1 serial line to send messages
 */
void
uart_init(void)
{
	/* TODO: Revisit when enabling interrupts and reads. */
	outb(COM1+1, 0x00); 	/* Disable interrupts (for now). */
	outb(COM1+3, 0x80);	/* Enable DLAB to set baud rate to 115200. */
	outb(COM1,   0x01);	/* (low byte) */
	outb(COM1+1, 0x00);	/* (high byte) */
	outb(COM1+2, 0x00);	/* Disable FIFO mode. */
	outb(COM1+3, 0x03);	/* Line control: 8 bits, disable DLAB */
	outb(COM1+4, 0x00);	/* Modem control: nada */
}

/*
 * Send a character out
 */
void
uart_putc(char ch)
{
	outb(COM1, ch);
}

