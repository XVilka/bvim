/* Bvim - BVi IMproved, binary analysis framework
 *
 * Copyright 1996-2004 by Gerhard Buergmann <gerhard@puon.at>
 * Copyright 2011 by Anton Kochkov <anton.kochkov@gmail.com>
 *
 * This file is part of Bvim.
 *
 * Bvim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bvim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bvim.  If not, see <http://www.gnu.org/licenses/>.
 */ 

#include <stdlib.h>
#include <string.h>
#include "data.h"
#include "signature.h"

extern core_t core;
extern state_t state;

// TODO: Implement signatures pack and signatures as linked lists
// TODO: Add lua/plugin interface for access to signatures base
// TODO: Link signatures and C-structs for autosearch and 
// extracting data

/* ---------------------------------------------------------------
 *                 Internal functions
 * --------------------------------------------------------------- 
 */

/* Load column from "magic" file */
static struct magic* extract_magic_record(char* line)
{
	int line_size = 0;
	struct magic *mg;

	mg = (struct magic*)malloc(sizeof(mg));
	line_size = strlen(line);

	return mg;
}

/* Load named signatures pack */
// TODO: Load from file
// TODO: Load from SQLite
static int signatures_load(char* path)
{
	return 0;
}

/* Unload named signatures pack */
static int signatures_unload(char *name)
{
	return 0;
}

// TODO: Add fuzzy search support
/* Return offset from block start, or offsets list */
offset_list signature_search(signature_t sg, unsigned long block_id)
{
	return 0;
}
