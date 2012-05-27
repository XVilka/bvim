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
#include "bvim.h"
#include "blocks.h"
#include "bscript.h"

extern core_t core;

/* =============== Blocks list/buffer abstractions =============== */

/* Preallocate some memory for future blocks allocation */
int InitBlocksList(buf_t *buf, int N)
{
	int i = 0;
	buf->blocks = (block_link)malloc((N + 1)*(sizeof *(buf->blocks)));
	for (i = 0; i < N + 1; i++) {
		buf->blocks[i].next = &(buf->blocks[i + 1]);
	}
	buf->blocks[N].next = NULL;
	return 0;
}

int BlockAdd(buf_t *buf, struct block_item i)
{
	block_link t = NULL;
	t = (block_link)malloc(sizeof(*t));
	if (t != NULL) {
		t->item = i;
		if (buf->blocks != NULL) {
			t->next = buf->blocks->next;
			buf->blocks->next = t;
		} else {
			buf->blocks = t;
			buf->blocks->next = NULL;
		}
	} else {
		return -1;
	}
	return 0;
}

void BlockFree(buf_t *buf, block_link x)
{
	BlockInsertNext(buf->blocks, x);
}

void BlockInsertNext(block_link x, block_link t)
{
	t->next = x->next;
	x->next = t;
}

block_link BlockDeleteNext(block_link x)
{
	block_link t =  NULL;
	if (x != NULL) {
		t = x->next;
		x->next = t->next;
	}
	return t;
}

block_link BlockNext(block_link x)
{
	return x->next;
}

struct block_item BlockGet(block_link x)
{
	return x->item;
}

/* ============= Blocks interface/handlers ================ */

/* Iterator of any functions on blocks list,
 * where result - expected result for function
 * All blocks are unique */
int blocks__Iterator(buf_t *buf, int (*(func))(), int result)
{
	block_link t;
	t = buf->blocks;
	while (t != NULL)
	{
		if (t->item.id != 0) {
			if ((*(func))(&(t->item)) == result) {
				return 0;
			}
		}
		t = t->next;
	}
	return -1;
}

int blocks__Add(buf_t *buf, struct block_item b) {
	
	BlockAdd(buf, b);

	return 0;
}

int blocks__DelByID(buf_t *buf, int id) {
	return 0;
}

int blocks__DelByName(buf_t *buf, char* name) {
	return 0;
}

struct block_item* blocks__GetByID(buf_t *buf, unsigned int id) {
	block_link t;
	
	t = buf->blocks;
	
	while (t != NULL)
	{
		if (t->item.id == id) return &(t->item);
		t = t->next;
	}
	return NULL;
}

struct block_item* blocks__GetByName(buf_t *buf, char* name) {
	block_link t;

	t = buf->blocks;

	while (t != NULL)
	{
		if (!strcmp(t->item.name, name)) return &(t->item);
		t = t->next;
	}
	return NULL;
}

/* Do not use this in exported API! */

int blocks__Init(buf_t *buf)
{
	//InitBlocksList(buf, 10);
	return 0;
}

/* Do not use this in exported API! */

int blocks__Destroy(buf_t *buf)
{
	block_link t;
	t = buf->blocks;
	while (t != NULL)
	{
		free(t);
		t = t->next;
	}
	return 0;
}

