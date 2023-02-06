/*
 * `vt.c` -- Virtual (framebuffer) terminal
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>

#include <sys/kargtab.h>
#include <sys/fb.h>

/*
 * This is a suuuuper simple virtual terminal driver, it's shitty honestly.
 * It prints text, and scrolls when the screen edge is reached. It's all that's
 * needed right now so it's all it does...
 * 									- Alan
 */

/* Font data */
static uint8_t *	font;		/* Base address of 8x16 PSF1 font */
static const uint8_t	font_width = 8;	/* Width in pixels */
static const uint8_t	font_height=16;	/* Height in pixels */

/* Colors */
static Color 	background;
static Color 	foreground;

/* Total rows and columns, and current row and column location */
static uint16_t		rows;
static uint16_t		cols;
static uint16_t		row;
static uint16_t		col;

/* Initialize virtual terminal */
void
vt_init(struct kargtab *kargtab)
{
	uint32_t *fb;
	int i;

	fb_init(kargtab);

	fb = (uint32_t *) FRAMEBUFFER.base;

	/* skip psf1 header */
	font = (uint8_t *)(kargtab->font_base + 4);

	background = fb_color(0, 0, 0);
	foreground = fb_color(0xd0, 0xd0, 0xd0);

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
	uint32_t *fb;
	uint8_t *glyph;
	uint8_t mask;
	uint8_t x, y;

	fb = (uint32_t *) FRAMEBUFFER.base;

	glyph = font + (ch * 16);
	for (y = 0; y < font_height; y++) {
		mask = 1 << (font_width - 1); 
		for (x = 0; x < font_width; x++) {
			if (glyph[y] & mask)
				*base = foreground;
			else
				*base = background;

			mask >>= 1;
			base++;
		}

		base += (FRAMEBUFFER.scanlinepx - (font_width));
	}
}

/* Scroll when last row is reached */
static void
scroll()
{
	uint32_t *plot, *copy;
	uint32_t *line, *end;

	plot = (uint32_t *) FRAMEBUFFER.base;
	line = (plot + (FRAMEBUFFER.scanlinepx * font_height));
	end = plot + FRAMEBUFFER.size;

	while (line < end) {
		
		copy = line;
		while (plot < line) {
			*plot = *copy;
			plot++;
			copy++;
		}

		line += ((uintptr_t)FRAMEBUFFER.scanlinepx * font_height);
	}

	col = 0;
	row = rows-1;
}

void
vt_putc(char ch)
{
	uint32_t x, y;
	uint32_t *base;

	if (col == cols) {
		col = 0;
		row++;
		vt_putc(ch);
		return;
	}

	if (row == rows) {
		scroll();
		vt_putc(ch);
		return;
	}

	if (ch == '\n') {
		col = 0;
		row++;
		return;
	}

	x = col * font_width;
	y = row * font_height;
	
	base = (uint32_t *)
		(FRAMEBUFFER.base + (((y * FRAMEBUFFER.width) + x) * 4));

	drawglyph(base, ch);

	col++;
}

