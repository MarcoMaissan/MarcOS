/*
 *
 * BertOS - Interrupt Descriptor Table driver header
 * src/include/idt.h
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

#define IDT_ENTRIES	48
#define SINT_ENTRIES	32
#define IRQ_ENTRIES	16

#define IRQ_KBD	0x01

struct desc_register {
	uint16_t	limit;
	void		*base;
} __attribute__ ((packed));

struct idt_desc {
	uint16_t	offset_lo;
	uint16_t	segment;
	uint8_t		zero;
	uint8_t		flags;
	uint16_t	offset_hi;
} __attribute__ ((packed));

struct int_stack {
	uint16_t	ds;
	uint16_t	reserved0;
	uint32_t	edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t	int_no;
	uint32_t	err_code;
	uint32_t	eip;
	uint16_t	cs;
	uint16_t	reserved2;
	uint32_t	eflags;
	uint32_t	usresp;
	uint16_t	ss;
	uint16_t	reserved3;
} __attribute__ ((packed));

static inline void idt_load(void *base, uint16_t limit)
{
	struct desc_register idt_ptr = {
		limit,
		base
	};

	asm volatile ("lidt %0" : : "m" (idt_ptr));
}

int isr_handler_reg(const uint8_t int_no, int (*handler) (struct int_stack *));
#define irq_handler_reg(i, h) isr_handler_reg(SINT_ENTRIES + i, h)

void _isr(struct int_stack regs);
void idt_init(void);
