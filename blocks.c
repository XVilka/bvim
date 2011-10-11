#include <stdlib.h>
#include "bvi.h"
#include "blocks.h"
#include "bscript.h"

/* =============== Blocks list/buffer abstractions =============== */

block_link blocks;

/* Preallocate some memory for future blocks allocation */
int InitBlocksList(int N)
{
	int i = 0;
	blocks = (block_link)malloc((N + 1)*(sizeof *blocks));
	for (i = 0; i < N + 1; i++) {
		blocks[i].next = &blocks[i + 1];
	}
	blocks[N].next = NULL;
	return 0;
}

int BlockAdd(struct block_item i)
{
	block_link t = NULL;
	t = (block_link)malloc(sizeof(block_link));
	if (t != NULL) {
		t->item = i;
		if (blocks != NULL) {
			t->next = blocks->next;
			blocks->next = t;
		} else {
			t->next = NULL;
			blocks = t;
		}
	} else {
		return -1;
	}
	return 0;
}

void BlockFree(block_link x)
{
	BlockInsertNext(blocks, x);
}

void BlockInsertNext(block_link x, block_link t)
{
	t->next = x->next;
	x->next = t;
}

block_link BlockDeleteNext(block_link x)
{
	block_link t;
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
int blocks__Iterator(int (*(func))(), int result)
{
	block_link t;
	t = blocks;
	while (t != NULL)
	{
		if ((*(func))(&(t->item)) == result) {
			return 0;
		}
		t = t->next;
	}
	return -1;
}

int blocks__Add(struct block_item b) {
	
	BlockAdd(b);

	return 0;
}

int blocks__DelByID(int id) {
	return 0;
}

int blocks__DelByName(char* name) {
	return 0;
}

struct block_item* blocks__GetByID(int id) {
	return NULL;
}

struct block_item* blocks__GetByName(char* name) {
	return NULL;
}

int blocks__Init()
{
	InitBlocksList(10);
	return 0;
}

int blocks__Destroy()
{
	block_link t;
	t = blocks;
	while (t != NULL)
	{
		BlockFree(t);
		t = t->next;
	}
	return 0;
}

