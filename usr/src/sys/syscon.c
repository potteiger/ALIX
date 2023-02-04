/*
 * `syscon.c` -- System console
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>

#include <sys/kargtab.h>
#include <sys/fb.h>
#include <sys/syscon.h>

/* Console font data */
static uint8_t *	font;			/* Font file base address */
static uint8_t *	glyph;			/* Another pointer to move */
static const uint8_t	font_width	= 8;	/* Font width */
static const uint8_t	font_height	= 16;	/* Font height */

/* Colors */
static Color		fg;			/* Foreground (text) color */
static Color		bg;			/* Background color */

/* Total rows and columns, and current row and column location */
static uint16_t		rows;
static uint16_t		cols;
static uint16_t		row;
static uint16_t		col;

/*
 * Initialize the system console using data passed to the kernel and the fb.
 * Initiates initialization of the framebuffer as well.
 */
void
init_syscon(struct kargtab *kargtab)
{	
	uint32_t i;
	uint32_t *fb;

	init_fb(kargtab);

	font = (uint8_t *) kargtab->font_base;
	fb = (uint32_t *) FRAMEBUFFER.base;
	
	/* skip psf1 header */
	font += 4;

	bg = fb_color(0x3b, 0x32, 0x28);
	fg = fb_color(0xD0, 0xC8, 0xC6);

	/* Fill background */
	for (i = 0; i < FRAMEBUFFER.size; i++) {
		fb[i] = bg;
	}

	rows = FRAMEBUFFER.height / font_height;
	cols = FRAMEBUFFER.width / font_width;

	row = 0;
	col = 0;
}

/*
 * Draws glyph to the framebuffer starting at given base framebuffer address.
 */
static void
drawglyph(uint32_t *base, char ch)
{
	uint8_t mask;
	uint8_t x, y;

	glyph = font + (ch * 16);
	for (y = 0; y < font_height; y++) {
		mask = 1 << (font_width - 1); 
		for (x = 0; x < font_width; x++) {
			if (glyph[y] & mask)
				*base = fg;
			else
				*base = bg;

			mask >>= 1;
			base++;
		}

		base += (FRAMEBUFFER.scanlinepx - (font_width));
	}

	base += font_width;
}

/*
 * Write given string to the system console.
 */
void
syscon_write(char *string)
{
	uint32_t x, y;
	uint32_t *base;
		
	for (; *string != '\0'; string++) {
		x = col * font_width;
		y = row * font_height;

		base = (uint32_t *)
		(FRAMEBUFFER.base + (((y * FRAMEBUFFER.width) + x) * 4));

		drawglyph(base, *string);
		col++;
	}
}

