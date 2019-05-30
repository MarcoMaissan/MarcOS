/*
 *
 * BertOS - String operations
 * src/boot/string.c
 *
 * Copyright (C) 2019 Bastiaan Teeuwen <bastiaan@mkcl.nl>
 * Copyright (C) 2019 Robin Kruit <0936014@hr.nl>
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

#include <sys/string.h>

void *memchr(const void *src, int c, size_t n)
{
	const unsigned char *s = src;

	while (n--)
		if (*s++ == c)
			return (void *) s - 1;

	return NULL;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;

	while (n--)
		*d++ = *s++;

	return dest;
}

void *memset(void *dest, int c, size_t n)
{
	unsigned char *d = dest;

	while (n--)
		*d++ = c;

	return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
	char *d = dest;
	const char *s = src;

	if (d == s)
		return dest;

	if (s > d) {
		while (n--)
			*d++ = *s++;
	} else {
		d += n - 1;
		s += n - 1;

		while (n--)
			*d-- = *s--;
	}

	return dest;
}

int strcmp(const char *src1, const char *src2)
{
	while (*src1 == *src2 && *src1) {
		src1++;
		src2++;
	}

	return *(unsigned char *) src1 - *(unsigned char *) src2;
}

int strncmp(const char *src1, const char *src2, size_t n)
{
	n--;

	while (*src1 == *src2 && *src1 && n--) {
		src1++;
		src2++;
	}

	return *(unsigned char *) src1 - *(unsigned char *) src2;
}

char *strcpy(char *dest, const char *src)
{
	char *d = dest;
	const char *s = src;

	while ((*d++ = *s++));

	return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	size_t i;

	for (i = 0; i < n && src[i] != '\0'; i++)
		dest[i] = src[i];

	while (i < n)
		dest[i++] = '\0';

	return dest;
}

size_t strlen(const char *str)
{
	const char *s = str;

	while (*s)
		s++;

	return (s - str);
}

size_t strnlen(const char *str, size_t n)
{
	size_t l;

	for (l = 0; *str && l < n; l++, str++);

	return l;
}

char *strtok(char *src, const char *delim)
{
	static char *d = NULL;
	const char *tok = delim;

	if (src) {
		d = src;
	} else {
		if (!d)
			return NULL;

		src = d;
	}

	while (*tok && *src) {
		if (*src == *tok) {
			src++;
			tok = delim;

			continue;
		}

		tok++;
	}

	if (!*src)
		return (d = NULL);

	for (d = src; *d; d++) {
		for (tok = delim; *tok; tok++) {
			if (*d != *tok)
				continue;

			*d++ = '\0';
			return src;
		}
	}

	d = NULL;
	return src;
}
