/*
 *
 * BertOS - Main entry point
 * src/boot/entry.c
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

#include <sys/idt.h>
#include <sys/pic.h>
#include <sys/print.h>
#include <sys/ps2.h>
#include <sys/vga.h>

extern void main(void);

void entry(void)
{
	idt_init();
	pic_init();

	vga_init();
	ps2_init();

	/* Enable hardware interrupts */
	asm volatile ("sti");

	printf("Welcome to BertOS!\n");

	main();
}
