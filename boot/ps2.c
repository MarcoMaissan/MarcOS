/*
 *
 * BertOS - PS/2 driver
 * src/boot/ps2.c
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
#include <sys/io.h>
#include <sys/ps2.h>

#define KEYMAP_SIZE	128	/* Size of the keymap */
#define KEYBUF_SIZE	24	/* Key ringbuffer size */

/* PS/2 CMDs */
#define PS2_KEYUP	0x80
#define PS2_CMD		0x64
#define PS2_IO		0x60
#define PS2_RESET	0xFE
#define PS2_EXT		0xE0

/* PS/2 modifier keys */
#define PS2_LSHIFT	0x2A
#define PS2_LSHIFTU	0xAA
#define PS2_RSHIFT	0x36
#define PS2_RSHIFTU	0xB6
#define PS2_LCTRL	0x1D
#define PS2_LCTRLU	0x9D
#define PS2_RCTRL	0x1DE0
#define PS2_RCTRLU	0x9DE0
#define PS2_LALT	0x38
#define PS2_LALTU	0xB8
#define PS2_RALT	0x38E0
#define PS2_RALTU	0xB8E0

/* Other PS/2 keys */
#define PS2_DEL		0x53E0
#define PS2_DELU	0xD0E0
#define PS2_LARROW	0x4BE0
#define PS2_LARROWU	0xCBE0
#define PS2_HOME	0x47E0
#define PS2_HOMEU	0xC7E0
#define PS2_END		0x4FE0
#define PS2_ENDU	0xCFE0
#define PS2_UARROW	0x48E0
#define PS2_UARROWU	0xC8E0
#define PS2_DARROW	0x50E0
#define PS2_DARROWU	0xD0E0
#define PS2_RARROW	0x4DE0
#define PS2_RARROWU	0xCDE0
#define PS2_ESC		0x01
#define PS2_ESCU	0x81

/* Modifier bits */
#define MOD_SHIFT	0x01
#define MOD_CTRL	0x02
#define MOD_SUPER	0x04
#define MOD_ALT		0x08
#define MOD_ARROW	0x10

/*
 * TODO
 * Esc, PrtSc, CapsLock, NumLock, ScrLk, Pause, Ins, Home, PGUP, Del, End, PGDN,
 * Function keys, Media keys, WinMenu
 */

static const char kbd_keymap[KEYMAP_SIZE] = {
	-1,
	0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
	'\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
	0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
	'*',
	0, ' ',
	0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0,
	'7', '8', '9', '-',
	'4', '5', '6', '+',
	'1', '2', '3',
	'0', '.'
};

static const char kbd_keymap_alt[KEYMAP_SIZE] = {
	-1,
	0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
	'\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
	0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
	0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
	'*',
	0, ' ',
	0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0,
	0, 0, 0, '-',
	0, 0, 0, '+',
	0, 0, 0,
	0, 0
};

static struct ringbuf {
	char	value[KEYBUF_SIZE];
	int	head, tail;
} buf;
static volatile uint8_t modifiers, skip;

static void ringbuf_push(char value)
{
	int next = buf.head + 1;

	if (next >= KEYBUF_SIZE)
		next = 0;

	if (next == buf.tail)
		return;

	buf.value[buf.head] = value;
	buf.head = next;
}

static char ringbuf_pop(void)
{
	int next = buf.tail + 1;
	char value;

	if (buf.head == buf.tail)
		return -1;

	if (next >= KEYBUF_SIZE)
		next = 0;

	value = buf.value[buf.tail];
	buf.tail = next;

	return value;
}

static int int_handler(struct int_stack *regs)
{
	uint16_t scancode;
	char c;
	(void) regs;

	scancode = io_inb(PS2_IO);

	if (skip) {
		skip--;
		return 1;
	}

	if (scancode == PS2_EXT) {
		scancode |= io_inb(PS2_IO) << 8;

		switch (scancode) {
		case PS2_RCTRL:
			modifiers |= MOD_CTRL;
			break;
		case PS2_RCTRLU:
			modifiers &= ~MOD_CTRL;
			break;
		case PS2_RALT:
			modifiers |= MOD_ALT;
			break;
		case PS2_RALTU:
			modifiers &= ~MOD_ALT;
			break;
		case PS2_DEL:
			ringbuf_push('\e');
			ringbuf_push('[');
			ringbuf_push('P');
			break;
		case PS2_LARROW:
			ringbuf_push('\e');
			ringbuf_push('[');
			ringbuf_push('D');
			break;
		case PS2_HOME:
			ringbuf_push('\e');
			ringbuf_push('[');
			ringbuf_push('H');
			break;
		case PS2_END:
			ringbuf_push('\e');
			ringbuf_push('[');
			ringbuf_push('F');
			break;
		case PS2_UARROW:
			ringbuf_push('\e');
			ringbuf_push('[');
			ringbuf_push('A');
			break;
		case PS2_DARROW:
			ringbuf_push('\e');
			ringbuf_push('[');
			ringbuf_push('B');
			break;
		case PS2_RARROW:
			ringbuf_push('\e');
			ringbuf_push('[');
			ringbuf_push('C');
			break;
		}

		skip++;
	} else {
		switch (scancode) {
		case PS2_LSHIFT:
		case PS2_RSHIFT:
			modifiers |= MOD_SHIFT;
			break;
		case PS2_LSHIFTU:
		case PS2_RSHIFTU:
			modifiers &= ~MOD_SHIFT;
			break;
		case PS2_LCTRL:
			modifiers |= MOD_CTRL;
			break;
		case PS2_LCTRLU:
			modifiers &= ~MOD_CTRL;
			break;
		case PS2_LALT:
			modifiers |= MOD_ALT;
			break;
		case PS2_LALTU:
			modifiers &= ~MOD_ALT;
			break;
		case PS2_ESC:
			ringbuf_push('\e');
			ringbuf_push('[');
			ringbuf_push('[');
			break;
		default:
			if ((scancode & PS2_KEYUP))
				break;

			if (modifiers & MOD_SHIFT) {
				if ((c = kbd_keymap_alt[scancode]) == '^')
					ringbuf_push('^');
				ringbuf_push(kbd_keymap_alt[scancode]);
			} else if (modifiers & MOD_CTRL) {
				ringbuf_push('^');
				ringbuf_push(kbd_keymap_alt[scancode]);
			} else if (modifiers & MOD_ALT) {
				ringbuf_push('\e');
				ringbuf_push('[');
				ringbuf_push(kbd_keymap[scancode]);
			} else {
				ringbuf_push(kbd_keymap[scancode]);
			}
		}
	}

	return 1;
}

/* Halt the system until a key is pressed */
char ps2_getch(void)
{
	char value;

	while ((value = ringbuf_pop()) < 0)
		asm volatile ("hlt");

	return value;
}

void ps2_init(void)
{
	irq_handler_reg(IRQ_KBD, &int_handler);
}
