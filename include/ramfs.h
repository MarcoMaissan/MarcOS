/*
 *
 * BertOS - File System assignment header
 * src/include/ramfs.h
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

#include <stddef.h>
typedef int ssize_t; /* ugh */

#define NAME_MAX	32	/* Max. length of a file name */

struct file {
	char		name[NAME_MAX + 1];	/* File name */
	char		*data;			/* File data pointer */
	size_t		size;			/* File data size in bytes */
	struct file	*prev, *next;
};

struct file *ramfs_seek(const char *name);
struct file *ramfs_readdir(void);

ssize_t ramfs_read(struct file *fp, char **data);
ssize_t ramfs_write(struct file *fp, const char *data, size_t n);

int ramfs_create(const char *name);
void ramfs_remove(struct file *fp);
