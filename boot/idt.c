/*
 *
 * BertOS - Interrupt Descriptor Table driver
 * src/boot/idt.c
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

/* Software interrupts */
extern void _int0(void);
extern void _int1(void);
extern void _int2(void);
extern void _int3(void);
extern void _int4(void);
extern void _int5(void);
extern void _int6(void);
extern void _int7(void);
extern void _int8(void);
extern void _int9(void);
extern void _int10(void);
extern void _int11(void);
extern void _int12(void);
extern void _int13(void);
extern void _int14(void);
extern void _int15(void);
extern void _int16(void);
extern void _int17(void);
extern void _int18(void);
extern void _int19(void);
extern void _int20(void);
extern void _int21(void);
extern void _int22(void);
extern void _int23(void);
extern void _int24(void);
extern void _int25(void);
extern void _int26(void);
extern void _int27(void);
extern void _int28(void);
extern void _int29(void);
extern void _int30(void);
extern void _int31(void);

/* Hardware interrupts */
extern void _int32(void);
extern void _int33(void);
extern void _int34(void);
extern void _int35(void);
extern void _int36(void);
extern void _int37(void);
extern void _int38(void);
extern void _int39(void);
extern void _int40(void);
extern void _int41(void);
extern void _int42(void);
extern void _int43(void);
extern void _int44(void);
extern void _int45(void);
extern void _int46(void);
extern void _int47(void);

void (*ints[IDT_ENTRIES]) (void) = {
	/* Software interrupts */
	_int0, _int1, _int2, _int3, _int4, _int5, _int6, _int7, _int8, _int9,
	_int10, _int11, _int12, _int13, _int14, _int15, _int16, _int17, _int18,
	_int19, _int20, _int21, _int22, _int23, _int24, _int25, _int26, _int27,
	_int28, _int29, _int30, _int31,

	/* Hardware interrupts */
	_int32, _int33, _int34, _int35, _int36, _int37, _int38, _int39, _int40,
	_int41, _int42, _int43, _int44, _int45, _int46, _int47
};

/*
 * This is a global list of all interrupt vectors:
 * 0x00 - 0x1F	- Nonmaskable interrupts
 * 0x20 - 0x2F	- IRQs
 * 0x20 - 0x7F	- Reserved
 * 0x80		- System calls
 * 0x81 - 0xFD	- Reserved
 * 0xFE	- 0xFF	- Unused
 */

static char *exceptions[SINT_ENTRIES] = {
	"Divide Error Exception (0x00)",
	"Debug Exception (0x01)",
	"NMI Interrupt (0x02)",
	"Breakpoint Exception (0x03)",
	"Overflow Exception (0x04)",
	"BOUND Range Exceeded Exception (0x05)",
	"Illegal instruction (0x06)",
	"Device Not Available Exception (0x07)",
	"Double Fault Exception (0x08)",
	"Reserved (0x09)",
	"Invalid TSS Exception (0x0A)",
	"Segment Not Present (0x0B)",
	"Stack Fault Exception (0x0C)",
	"General Protection Exception (0x0D)",
	"Page-Fault Exception (0x0E)",
	"Reserved (0x0F)",
	"Reserved (0x10)",
	"Alignment Check Exception (0x11)",
	"Machine-Check Exception (0x12)",
	"SIMD Floating-Point Exception (0x13)",
	"Virtualization Exception (0x14)",
	"Reserved (0x15)",
	"Reserved (0x16)",
	"Reserved (0x17)",
	"Reserved (0x18)",
	"Reserved (0x19)",
	"Reserved (0x1A)",
	"Reserved (0x1B)",
	"Reserved (0x1C)",
	"Reserved (0x1D)",
	"Reserved (0x1E)",
	"Reserved (0x1F)",
};

static struct idt_desc idt[IDT_ENTRIES];
static int (*isr_handlers[IDT_ENTRIES]) (struct int_stack *regs);

/*
 * Valid values for type are:
 * - Interrupt Gate	(0b1110)
 * - Task Gate		(0b1001)
 */

void _isr(struct int_stack regs)
{
	if (regs.int_no < SINT_ENTRIES) {
		printf("%s\nThe system has been halted.",
				exceptions[regs.int_no]);

		for (;;)
			asm volatile ("hlt");
	}

	if (isr_handlers[regs.int_no])
		isr_handlers[regs.int_no](&regs);

	/* Send the End Of Interrupt signal to the PIC */
	if (regs.int_no < SINT_ENTRIES + IRQ_ENTRIES - 1)
		pic_eoi(regs.int_no);
}

int isr_handler_reg(const uint8_t int_no, int (*handler) (struct int_stack *))
{
	if (isr_handlers[int_no])
		return -1;

	isr_handlers[int_no] = handler;

	/* Unmask if this is an IRQ */
	if (int_no >= SINT_ENTRIES && int_no < SINT_ENTRIES + IRQ_ENTRIES - 1)
		irq_unmask(int_no - SINT_ENTRIES);

	return 0;
}

static void idt_set(const uint8_t gate, const uint8_t type, const uintptr_t offset)
{
	idt[gate].offset_lo	= offset & 0xFFFF;
	idt[gate].segment	= 0x08;
	idt[gate].zero		= 0;
	idt[gate].flags		= type | 0b10000000;
	idt[gate].offset_hi	= (offset >> 16) & 0xFFFF;
}

void idt_init(void)
{
	int i;

	/* Fill the IDT */
	for (i = 0; i < IDT_ENTRIES; i++) {
		idt_set(i, 0x0E, (uintptr_t) ints[i]);

		/* Mask all IRQs */
		if (i >= SINT_ENTRIES && i < SINT_ENTRIES + IRQ_ENTRIES - 1)
			irq_mask(i - SINT_ENTRIES);
	}

	idt_load(&idt, IDT_ENTRIES * sizeof(struct idt_desc) - 1);
}
