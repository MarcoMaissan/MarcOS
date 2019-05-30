/*
 *
 * BertOS - Multiboot entry
 * src/boot/boot.S
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

.section .multiboot

/* MultiBoot header */
.set MB_FLAGS,	(1 << 0) | (1 << 1)
.set MB_MAGIC,	0x1BADB002

.align 4
mboot_header:
	.long MB_MAGIC
	.long MB_FLAGS
	.long -(MB_MAGIC + MB_FLAGS)


.section .bss

/* 16 KiB stack */
.align 16
stack_bottom:
.skip 16384
stack_top:


.section .text

.global _start
_start:
	cli

	mov $stack_top, %esp

	lgdt gdtp

	call entry

	cli
halt:
	hlt
	jmp halt


.section .data

/* Global Descriptor Table - A flat memory model is used for simplicity */
gdt:
	.quad 0				/* 0x00 - NULL */
	.long 0x0000FFFF		/* 0x08 - Code SV */
	.long 0x00C09A00
	.long 0x0000FFFF		/* 0x10 - Data SV */
	.long 0x00C09200

gdtp:
	.word . - gdt - 1
	.long gdt
