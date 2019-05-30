/*
 *
 * BertOS - VGA driver
 * src/boot/vga.c
 *
 * Copyright (C) 2019 Bastiaan Teeuwen <bastiaan@mkcl.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 *
 */

#include <sys/io.h>
#include <sys/vga.h>

static uint16_t *buf = (uint16_t *) 0xB8000;
static uint8_t x, y;

static void vga_putc(char c, uint8_t x, uint8_t y) {
	buf[y * VGA_WIDTH + x] = ((uint16_t) c) | ((uint16_t) 7 | 0 << 4) << 8;
}

static void vga_scroll(int n)
{
	int i;

	if (n < 0)
		return;

	y--;

	for (i = 0; i < (VGA_HEIGHT - n) * VGA_WIDTH; i++)
		buf[i] = buf[i + VGA_WIDTH];

	while (i < VGA_HEIGHT * VGA_WIDTH)
		buf[i++] = ((uint16_t) ' ') | ((uint16_t) 7 | 0 << 4) << 8;
}

struct vga_cursor vga_curget(void)
{
	struct vga_cursor cur;

	cur.x = x;
	cur.y = y;

	return cur;
}

void vga_curset(struct vga_cursor cur, bool relative)
{
	uint16_t off;
	int rx;

	if (relative) {
		y += cur.y + ((x + cur.x) / VGA_WIDTH);
		rx = (x + cur.x) % VGA_WIDTH;
		if (rx < 0) {
			x = (uint8_t) rx + VGA_WIDTH;
			/* FIXME */
			y--;
		} else {
			x = (uint8_t) rx;
		}

		if (y >= VGA_HEIGHT)
			vga_scroll(1);
	} else {
		if (cur.x < 0 || cur.y < 0 ||
				cur.x > VGA_WIDTH || cur.y > VGA_HEIGHT)
			return;

		x = (uint8_t) cur.x;
		y = (uint8_t) cur.y;
	}

	off = y * VGA_WIDTH + x;

	/* Set the cursor position */
	io_outb(0x3D4, 0x0E);
	io_outb(0x3D5, off >> 8);
	io_outb(0x3D4, 0x0F);
	io_outb(0x3D5, off);
}

void vga_clear(void)
{
	struct vga_cursor cur;
	uint8_t xi, yi;

	for (xi = 0; xi < VGA_WIDTH; xi++)
		for (yi = 0; yi < VGA_HEIGHT; yi++)
			vga_putc(' ', xi, yi);

	cur.x = (uint8_t) 0;
	cur.y = (uint8_t) 0;

	vga_curset(cur, false);
}

void vga_put(const char c)
{
	struct vga_cursor cur;
	cur.x = 0;
	cur.y = 0;

	switch (c) {
	case '\b':
		cur.x--;

		if ((int) x - 1 < 0)
			vga_putc(' ', VGA_WIDTH - 1, y - 1);
		else
			vga_putc(' ', x - 1, y);
		break;
	case '\n':
		cur.x = -x;
		cur.y++;
		break;
	case '\t':
		cur.x += 8;
		break;
	case '\r':
		cur.x = -x;
		break;
	default:
		vga_putc(c, x, y);
		cur.x++;
	}

	vga_curset(cur, true);
}

void vga_init(void)
{
	uint16_t off;

	/* Get the cursor position */
	io_outb(0x3D4, 0x0E);
	off = io_inb(0x3D5) << 8;
	io_outb(0x3D4, 0x0F);
	off |= io_inb(0x3D5);

	y = off / 80;
	x = off % 80;
}
