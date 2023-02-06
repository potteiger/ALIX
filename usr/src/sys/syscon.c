/*
 * `syscon.c` -- System console interface
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#include <sys/kargtab.h>
#include <sys/uart.h>
#include <sys/syscon.h>
#include <sys/vt.h>

/*
 * System Console interface for the kernel. Currently not focussing on a virtual
 * terminal until the system matures to a point where it would be beneficial.
 * For the time being `syscon` will only interface with UART. Once a virtual
 * terminal is implemented this interface will change very little.
 * 									-Alan
 */

void
syscon_init(struct kargtab *kargtab)
{
	uart_init();		/* Initialize UART for messaging */
	vt_init(kargtab);	/* Virtual terminal */
}

/*
 * Prints 64-bit number to the system console in specified base and signedness.
 * `base`:  10 | 16
 * `sign: unsigned=0 signed=1
 */
static void
printval(int64_t sval, int base, int sign)
{
	static char chars[] = "0123456789ABCDEF";
	static char buf[32];
	uint64_t value;
	int i;

	if (sval == 0) {
		if (base == 10)
			kputc('0');
		else if (base == 16)
			kputs("0x0");
		return;
	}

	if (sign && (sign = (sval < 0)))
		value = -sval;
	else
		value = sval;

	if (base == 16)
		sign = 0;

	buf[31] = '\0';
	for(i = 30; value && i ; i--, (value /= base))
		buf[i] = chars[value % base];
	i++;

	if (base == 16) {
		buf[i-2] = '0';
		buf[i-1] = 'x';
		i -= 2;
	}

	if (sign) {
		buf[i-1] = '-';
		i--;
	}

	while (buf[i] != '\0') {
		kputc(buf[i]);
		i++;
	}
}

void
kputc(char ch)
{
	uart_putc(ch);
	vt_putc(ch);
}

/* Print a null terminated string */
void
kputs(char *string)
{
	for (; *string != '\0'; string++)
		kputc(*string);
}

/*
 * Basic `printf` implementation.
 * Formatting syntax: "%[size] format"
 * Sizes:
 * 	- 'hh':	 8-bit (promoted to 32-bit int)
 * 	- 'h': 	16-bit (promoted to 32-bit int)
 * 	- 'l': 	64-bit
 * 	- default is 32-bit
 *
 * Formats:
 * 	- 's':	null terminated string
 * 	- 'c':	`char`
 * 	- 'u':	unsigned integer
 * 	- 'x':	hexadecimal
 * 	- 'd':	decimal integer
 */
void
kprintf(const char *fmt, ...)
{
	char *ptr;
	char size;
	char format;
	int base;
	int sign;
	int64_t num;
	va_list args;

	va_start(args, fmt);

	size = format = 0;
	sign = 1;

	while (*fmt != '\0') {
		if (*fmt != '%') {
			kputc(*fmt);
			fmt++;
			continue;
		}
		fmt++;

swtch:
		switch (*fmt) {
		case 'h':
			size = 'h';
			fmt++;
			if (*fmt == 'h')
				fmt++;
			goto swtch;
		case 'l':
			size = 'l';
			fmt++;
			goto swtch;
		case 's':
			if (size != '\0') {
				kputc('%');
				continue;
			}

			kputs(va_arg(args, char*));
			fmt++;
			continue;
		case 'c':
			if (size != '\0') {
				kputc('%');
				continue;
			}

			kputc((char) va_arg(args, int));
			fmt++;
			continue;
		case 'u':
			sign = 0;
		case 'd':
			base = 10;
			fmt++;
			break;
		case 'x':
			base = 16;
			fmt++;
			break;
		default:
			kputc('%');
			fmt++;
			continue;
		}


		if (size == 'l')
			num = va_arg(args, int64_t);
		else
			num = va_arg(args, int32_t);

		printval(num, base, sign);
	}

	va_end(args);
}


void
syscon_write(void *buf, size_t sz)
{
	for (; sz > 0; sz--) {
		kputc(*(char *)buf);
		buf++;
	}
}

