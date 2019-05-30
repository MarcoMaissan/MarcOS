/*
 *
 * BertOS - File System assignment
 * src/ramfs.c
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

#include <ramfs.h>

static struct file *head;

/* Seek for files by 'name' */
struct file *ramfs_seek(const char *name)
{
	/* TODO */
}

/* Return the head of the file list */
struct file *ramfs_readdir(void)
{
	return head;
}

/*
 * Read from a file into 'data' (which is automatically (re)allocated)
 * Return:
 *  # of bytes read
 * -1 when out of memory
 */
ssize_t ramfs_read(struct file *fp, char **data)
{
	/* TODO */
}

/*
 * Write 'n' bytes to a file from 'data'
 * Return:
 *  # of bytes written
 * -1 when out of memory
 */
ssize_t ramfs_write(struct file *fp, const char *data, size_t n)
{
	/* TODO */
}

/*
 * Create a file with 'name'
 * Return:
 *  0 on success
 * -1 when out of memory
 * -2 when file with 'name' already exists
 * -3 when an invalid 'name' is entered
 */
int ramfs_create(const char *name)
{
	/* TODO */
}

/* Remove a file and it's data */
void ramfs_remove(struct file *fp)
{
	/* TODO */
}
