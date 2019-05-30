/*
 *
 * BertOS - I/O functions header
 * src/include/io.h
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

#include <stdint.h>

static inline uint8_t io_inb(uint16_t port)
{
	uint8_t val;

	asm volatile ("inb %1, %0" : "=a" (val) : "dN" (port));
	return val;
}

static inline uint16_t io_inw(uint16_t port)
{
	uint16_t val;

	asm volatile ("inw %1, %0" : "=a" (val) : "dN" (port));
	return val;
}

static inline uint32_t io_ind(uint16_t port)
{
	uint32_t val;

	asm volatile ("inl %1, %0" : "=a" (val) : "dN" (port));
	return val;
}

static inline void io_insw(uint16_t port, char *buf)
{
	asm volatile ("insw" : "+D" (buf) : "dN" (port) : "memory");
}

static inline void io_outb(uint16_t port, uint8_t val)
{
	asm volatile ("outb %0, %1" : : "a" (val), "dN" (port));
}

static inline void io_outw(uint16_t port, uint16_t val)
{
	asm volatile ("outw %0, %1" : : "a" (val), "dN" (port));
}

static inline void io_outd(uint16_t port, uint32_t val)
{
	asm volatile ("outl %0, %1" : : "a" (val), "dN" (port));
}

