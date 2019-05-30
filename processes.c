/*
 *
 * BertOS - Process assignment
 * src/processes.c
 *
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

#include <sys/print.h>

#include <processes.h>

/*
 * This function should fill a new instance of the process struct
 */
int add_process(char *name, int (*func)(void))
{
	/* TODO */
}

/*
 * This function should pause a process, causing the scheduler to skip it
 */
int suspend_process(int id)
{
	/* TODO */
}

/*
 * This function should resume a paused process
 */
int resume_process(int id)
{
	/* TODO */
}

/*
 * This function should remove a process from the table and pack the table
 */
int kill_process(int id)
{
	/* TODO */
}

/*
 * This function should check each process’ state and act appropiately.
 * Processes can be killed here, after which the process table should be packed.
 */
void do_round(void)
{
	/* TODO */
}
