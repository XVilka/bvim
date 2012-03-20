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

/* Internal abstractions */

int InitBlocksList();
block_link BlockNew();
void BlockFree();
void BlockInsertNext();
block_link BlockDeleteNext();
block_link BlockNext();
struct block_item BlockGet();

/* External interface */

int blocks__Iterator(buf_t *buf, int (*(func))(), int result);
int blocks__Add(buf_t *buf, struct block_item b);
int blocks__Init(buf_t *buf);
int blocks__Destroy(buf_t *buf);
int blocks__DelByID(buf_t *buf, int id);
int blocks__DelByName(buf_t *buf, char* name);
struct block_item* blocks__GetByID(buf_t *buf, unsigned int id);
struct block_item* blocks__GetByName(buf_t *buf, char* name);

