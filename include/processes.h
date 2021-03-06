/*
 *
 * BertOS - Processes assignment header
 * src/include/processes.h
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

#pragma once

/*
 * This basic structure stores the name of a process,
 * the id generated by add_process, it's state
 * and the function pointer used to call it.
 */
struct process {
	char *name;
	int id;
	char state;
	int (*func)(void);
};

int add_process(char *name, int (*func)(void));
int suspend_process(int id);
int resume_process(int id);
int kill_process(int id);
void do_round(void);
