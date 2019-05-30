/*
 *
 * BertOS - VGA driver header
 * src/include/vga.h
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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#define VGA_WIDTH	80
#define VGA_HEIGHT	25

struct vga_cursor {
	int x, y;
};

struct vga_cursor vga_curget(void);
void vga_curset(struct vga_cursor cur, bool relative);

void vga_clear(void);

void vga_put(const char c);
void vga_init(void);
