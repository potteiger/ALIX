/*
 * `fb.h` -- framebuffer
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef _FB_H_
#define _FB_H_

#define _Framebuffer struct Framebuffer { \
	uintptr_t	base;		/* base address of framebuffer */     \
	uint64_t	size;		/* size of framebuffer (in pixels) */ \
	uint32_t	width;		/* width in pixels */                 \
	uint32_t	height;		/* height in pixels */                \
	uint32_t	scanlinepx;	/* pixels per scan line */            \
	/*								      \
	 * Number of bits to shift left for the set GOP pixel format and      \
	 * appropriate masks					      	      \
	 */								      \
	uint32_t	redmask;					      \
	uint32_t	greenmask;					      \
	uint32_t	bluemask;					      \
	uint8_t		redshift;					      \
	uint8_t		greenshift; 					      \
	uint8_t		blueshift; }

#ifndef _FB_C_
extern const _Framebuffer FRAMEBUFFER;
#endif

typedef uint32_t Color;

void 	init_fb(struct kargtab *kargtab);
void	fb_plot(uint32_t x, uint32_t y, Color color);
Color	fb_color(Color red, Color green, Color blue);


#endif /* _FB_H_ */

