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

#include "bvim.h"
#include "keys.h"
#include "blocks.h"
#include "bmath.h"
#include "search.h"
#include "set.h"
#include "ui.h"
#include "commands.h"

#include "messages.h"

#ifdef HAVE_LUA_H
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "bscript.h"
#endif

#ifdef HAVE_UNISTD_H
#	include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
#	include <fcntl.h>
#endif

#define WRITE (O_WRONLY|O_CREAT|O_TRUNC)
#define APPEND (O_WRONLY|O_APPEND)

#include "plugins.h"

#define	CMDSZ	256		/* size of the command buffer */
#define MAXNAME	10		/* size of a : command name */

#define NO_ADDR			1
#define NO_ARG			8
#define ONE_ARG			16
#define	MAX_ONE_ARG		32
#define ONE_FILE		64
#define	MAX_ONE_FILE	128

static char *altfile = NULL;	/* alternate file */
static struct stat bbuf;
static int c_argc = 0;
static char *c_argv[9];

// TODO: move messages to messages.h

int repl_count = -1;
int addr_flag;

PTR start_addr;
PTR end_addr;

//extern core_t core;
//extern state_t state;

extern char *name;		/* actual filename */
extern char **files;		/* used for "next" and "rewind" */
extern int numfiles, curfile;
extern int errno;

static char oldbuf[CMDSZ];		/** for :!! command **/

/* -------------------------------------------------------------
 *                Internal functions
 * -------------------------------------------------------------
 */

int CmdAdd(core_t *core, command_t *item)
{
	if (core->cmdmap.items == core->cmdmap.allocated) {
		if (core->cmdmap.allocated == 0)
			core->cmdmap.allocated = 3;
		else
			core->cmdmap.allocated += 4;

		void *_tmp =
		    realloc(core->cmdmap.arr,
			    (core->cmdmap.allocated * sizeof(command_t)));

		if (!_tmp) {
			fprintf(stderr, "ERROR [CmdAdd]: Couldn't realloc memory!\n");
			return (-1);
		}
		core->cmdmap.arr = (struct command *)_tmp;
	}
	core->cmdmap.arr[core->cmdmap.items] = *item;
	core->cmdmap.items++;

	return core->cmdmap.items;
}

int CmdDel(core_t *core, char *name)
{
	int i = 0;
	while (i < core->cmdmap.items) {
		if (strcmp(name, core->cmdmap.arr[i].name) == 0) {
			core->cmdmap.arr[i].enabled = 0;
			return 0;
		}
		i++;
	}
	return -1;
}

int CmdDefaults(core_t *core)
{
	int i = 0;
	struct command cmds_def[] = {
		{ 1, "help", "show usage for every command", 1, BVI_HANDLER_INTERNAL, { .func = command__help }, 4, 2},
		{ 2, "map", "", 1, BVI_HANDLER_INTERNAL, { .func = command__map }, 3, 2},
		{ 3, "unmap", "", 1, BVI_HANDLER_INTERNAL, { .func = command__unmap }, 5, 2},
		{ 4, "set", "", 1, BVI_HANDLER_INTERNAL, { .func = command__set }, 3, 2},
		{ 5, "block", "", 1, BVI_HANDLER_INTERNAL, { .func = command__block }, 5, 2},
		{ 6, "lua", "", 1, BVI_HANDLER_INTERNAL, { .func = command__lua }, 3, 2},
		{ 7, "args", "", 1, BVI_HANDLER_INTERNAL, { .func = command__args }, 4, 2},
		{ 8, "source", "", 1, BVI_HANDLER_INTERNAL, { .func = command__source }, 6, 2},
		{ 9, "run", "", 1, BVI_HANDLER_INTERNAL, { .func = command__run }, 3, 2},
		{ 10, "cd", "", 1, BVI_HANDLER_INTERNAL, { .func = command__cd }, 2, 2},
		{ 11, "edit", "", 1, BVI_HANDLER_INTERNAL, { .func = command__edit }, 4, 1},
		{ 12, "file", "", 1, BVI_HANDLER_INTERNAL, { .func = command__file }, 4, 1},
		{ 13, "read", "", 1, BVI_HANDLER_INTERNAL, { .func = command__read }, 4, 1},
		{ 14, "xit", "", 1, BVI_HANDLER_INTERNAL, { .func = command__xit }, 3, 1},
		{ 15, "next", "", 1, BVI_HANDLER_INTERNAL, { .func = command__next }, 4, 1},
		{ 16, "rewind", "", 1, BVI_HANDLER_INTERNAL, { .func = command__rewind }, 6, 3},
		{ 17, "append", "", 1, BVI_HANDLER_INTERNAL, { .func = command__append }, 6, 1},
		{ 18, "change", "", 1, BVI_HANDLER_INTERNAL, { .func = command__change }, 6, 1},
		{ 19, "mark", "", 1, BVI_HANDLER_INTERNAL, { .func = command__mark }, 4, 2},
		{ 20, "yank", "", 1, BVI_HANDLER_INTERNAL, { .func = command__yank }, 4, 1},
		{ 21, "overwrite", "", 1, BVI_HANDLER_INTERNAL, { .func = command__overwrite }, 9, 1},
		{ 22, "undo", "", 1, BVI_HANDLER_INTERNAL, { .func = command__undo }, 4, 1},
		{ 23, "version", "", 1, BVI_HANDLER_INTERNAL, { .func = command__version }, 7, 2},
		{ 24, "shell", "", 1, BVI_HANDLER_INTERNAL, { .func = command__shell }, 5, 2},
		{ 25, "quit", "", 1, BVI_HANDLER_INTERNAL, { .func = command__quit }, 4, 1},
		{ 26, "sleft", "", 1, BVI_HANDLER_INTERNAL, { .func = command__sleft }, 5, 2},
		{ 27, "sright", "", 1, BVI_HANDLER_INTERNAL, { .func = command__sright }, 6, 2},
		{ 28, "rleft", "", 1, BVI_HANDLER_INTERNAL, { .func = command__rleft }, 5, 2},
		{ 29, "rright", "", 1, BVI_HANDLER_INTERNAL, { .func = command__rright }, 6, 2},
		{ 30, "and", "", 1, BVI_HANDLER_INTERNAL, { .func = command__and }, 3, 2},
		{ 31, "or", "", 1, BVI_HANDLER_INTERNAL, { .func = command__or }, 2, 2},
		{ 32, "xor", "", 1, BVI_HANDLER_INTERNAL, { .func = command__xor }, 3, 2},
		{ 33, "neg", "", 1, BVI_HANDLER_INTERNAL, { .func = command__neg }, 3, 2},
		{ 34, "not", "", 1, BVI_HANDLER_INTERNAL, { .func = command__not }, 3, 2},
		{ 35, "fuz", "fuzzy search command", 1, BVI_HANDLER_INTERNAL, { .func = command__fuz }, 3, 2},
		{ 0, NULL, NULL, 0, 0, { NULL }, 0, 0 }
	};
	while (cmds_def[i].id != 0) {
		CmdAdd(core, &cmds_def[i]);
		i++;
	}
	return 0;
}

/* ------------------------------------------------------
 *                  Exported functions
 * ------------------------------------------------------
 */

void commands__Init(core_t *core)
{
	core->cmdmap.arr = NULL;
	core->cmdmap.items = 0;
	core->cmdmap.allocated = 0;
	CmdDefaults(core);
}

void commands__Destroy(core_t *core)
{
	free(core->cmdmap.arr);
}

int commands__Cmd_Add(core_t *core, command_t *new_cmd)
{
	CmdAdd(core, new_cmd);
	return 0;
}

int commands__Cmd_Del(core_t *core, char* name)
{
	CmdDel(core, name);
	return 0;
}

/* =================== Command handlers ===================== */

char luacmdbuf[CMDSZ];

static char* tmp_cmd;

// :help
int command__help(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	int i = 0;
	if (c_argc == 0) {
		bvim_error(core, buf, "What?");
	} else if (c_argc == 1) {
		while (i < core->cmdmap.items) {
			if (strncmp(c_argv[0], core->cmdmap.arr[i].name, strlen(core->cmdmap.arr[i].name))) {
				bvim_error(core, buf, "Command not exist!");
			} else {
				bvim_info(core, buf, core->cmdmap.arr[i].description);
				return 0;
			}
			i++;
		}
	}
	return -1;
}

// :map
int command__map(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	int n = 0;
	bkey_t *key_tmp;

	if (c_argc == 0) {
		bvim_error(core, buf, "Error: empty mapping definition!");
	} else if (c_argc == 1) {
		if (strncmp(c_argv[0], "all", 3)) {
			bvim_error(core, buf, "Map what?");
		} else {
			keys__KeyMaps_Show(core);
		}
	} else {
		luacmdbuf[0] = '\0';
		for (n = 1; n < c_argc; n++) {
			strcat(luacmdbuf, " ");
			strcat(luacmdbuf, c_argv[n]);
		}
		key_tmp = keys__KeyString_Parse(core, c_argv[0]);
		if (!keys__Key_Map(core, key_tmp))
			bvim_error(core, buf, "Error: can't set new key mapping!");
		return 0;
	}
	return -1;
}

// :unmap
int command__unmap(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	bkey_t *key_tmp;

	if (c_argc != 1) {
		bvim_error(core, buf, "Error: empty mapping definition!");
	} else {
		key_tmp = keys__KeyString_Parse(core, c_argv[0]);
		if (!keys__Key_Unmap(core, key_tmp))
			bvim_error(core, buf, "Error: can't remove key mapping!");
		return 0;
	}
	return -1;
}


// :set
int command__set(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	int n  = 0;
	if (chk_comm(core, buf, NO_ADDR))
		return -1;
	if (c_argc == 0) {
		doset(core, NULL);
	} else {
		for (n = 0; n < c_argc; n++) {
			if (doset(core, c_argv[n]))
				return 0;
		}
	}
	return -1;
}

/* COMMAND: ":block"
 *
 * Syntax:
 *	:block add <start> <end>
 *	or
 *	:block add <start> +size
 *	where size can be 456, 15K, 20M
 *	:block del <id>
 *	or
 *	:block del <start>
 *	or
 *	:block info <id>
 *	or
 *	:block list
 *	or
 *	:block save <id> <filename>
 *	or
 *	:block yank <id>
 */
int command__block(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	struct block_item tmp_blk;
	struct block_item *blkblk;

	if (c_argc == 0) {
		return -1;
	} else {
		/* :block add */
		if (!strncmp(c_argv[0], "add", 3)) {
			if (c_argc == 5) {
				/* Filling info into tmp_blk structure */
				tmp_blk.id = (unsigned int)atoi(c_argv[1]); // block id
				if (atoi(c_argv[2]) < atoi(c_argv[3])) {
					tmp_blk.pos_start = math__eval(MATH_ARITH, c_argv[2]);
					tmp_blk.pos_end = math__eval(MATH_ARITH, c_argv[3]);
					if ((atoi(c_argv[4]) < 0) | (atoi(c_argv[4]) > 6))
						tmp_blk.palette = C_HX;
					else
						tmp_blk.palette = atoi(c_argv[4]);
					tmp_blk.hl_toggle = 1;
					tmp_blk.folding = 0;
					/* Allocating new block and inserting it into blocks list */
					blocks__Add(buf, tmp_blk);
					// Adding block to highlights
					ui__BlockHighlightAdd(&tmp_blk);
					// this we need to remove too
					blkblk = blocks__GetByID(buf, tmp_blk.id);
					bvim_info(core, buf, "Added block: start %ld end %ld", blkblk->pos_start, blkblk->pos_end);
					
					ui__Screen_Repaint(core, buf);
				} else {
					bvim_error(core, buf, "Wrong block start and end values!");
					return -1;
				}
			} else {
				bvim_error(core, buf, "Wrong :block command format!");
				return -1;
			}
		/* :block del */
		} else if (!strncmp(c_argv[0], "del", 3)) {
		/* :block info */
		} else if (!strncmp(c_argv[0], "info", 4)) {
			if (c_argc == 2) {
				blkblk = blocks__GetByID(buf, (unsigned int)atoi(c_argv[1])); // block id
				if (blkblk != NULL) {
					bvim_info(core, buf, "Block #%d [%ld, %ld], %s", blkblk->id, blkblk->pos_start, blkblk->pos_end, blkblk->annotation);
				} else {
					bvim_error(core, buf, "Can't find block with %d id!", (unsigned int)atoi(c_argv[1]));
					return -1;
				}
			} else {
				bvim_error(core, buf, "Wrong :block command format!");
				return -1;
			}
		/* :block list */
		} else if (!strncmp(c_argv[0], "list", 4)) {
		/* :block save */
		} else if (!strncmp(c_argv[0], "save", 4)) {
		/* :block yank */
		} else if (!strncmp(c_argv[0], "yank", 4)) {
		/* :block fold */
		} else if (!strncmp(c_argv[0], "fold", 4)) {
			if (c_argc == 1) {
				blkblk = blocks__GetByID(buf, (unsigned int)atoi(c_argv[1])); // block id
				if (blkblk != NULL) {
					blkblk->folding = 1;
					ui__Screen_Repaint(core, buf);
				}
			}
		/* :block annotation */
		} else if (!strncmp(c_argv[0], "note", 4)) {
			if (c_argc == 3) {
				blkblk = blocks__GetByID(buf, (unsigned int)atoi(c_argv[1])); // block id
				if (blkblk != NULL) {
					blkblk->annotation = c_argv[2]; // FIXME: Use valid string copy function !
				} else {
					bvim_error(core, buf, "Can't find block with %d id!", (unsigned int)atoi(c_argv[1]));
					return -1;
				}
			} else {
				bvim_error(core, buf, "Wrong :block command format!");
				return -1;
			}
		} else {
			bvim_error(core, buf, "Wrong :block command format!");
			return -1;
		}
	}
	return 0;
}

// :lua
int command__lua(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	int n = 0;

	if (c_argc == 0) {
		bvim_error(core, buf, "Error: empty lua command!");
	} else {
		luacmdbuf[0] = '\0';
		for (n = 0; n < c_argc; n++) {
			strcat(luacmdbuf, " ");
			strcat(luacmdbuf, c_argv[n]);
		}
		bvim_run_lua_string(core, buf, luacmdbuf);
	}
	return 0;
}

// :args
int command__args(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	int n = 0;

	if (chk_comm(core, buf, NO_ADDR | NO_ARG))
		return -1;
	string[0] = '\0';
	for (n = 0; n < numfiles; n++) {
		if (n == curfile)
			strcat(string, "[");
		strcat(string, files[n]);
		if (n == curfile)
			strcat(string, "]");
		strcat(string, " ");
	}
	bvim_info(core, buf, string);
	return 0;
}

// :source
int command__source(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (chk_comm(core, buf, NO_ADDR | ONE_FILE))
		return -1;
	if (read_rc(core, c_argv[0]))
		bvim_error(core, buf, c_argv[0]);
	refresh();
	return 0;
}

// :run
int command__run(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (c_argc == 0) {
		bvim_error(core, buf, "Error: empty plugin name!");
	} else {
		bvim_run_lua_script(core, buf, c_argv[0]);
	}
	return 0;
}

// :cd
int command__cd(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (chk_comm(core, buf, NO_ADDR | ONE_FILE))
		return -1;
	if (!(flags & FLAG_FORCE)) {
		if (edits) {
			if (P(P_AW)) {
				save(core, buf, name, WRITE);
				edits = 0;
			} else {
				bvim_error(core, buf, BVI_ERROR_NOWRITE, "cd");
				return -1;
			}
		}
	}
	if (c_argc == 0) {
		sprintf(string, "%c", DELIM);
		c_argv[0] = string;
	}
	if (chdir(c_argv[0]))
		bvim_error(core, buf, c_argv[0]);
	return 0;
}

// :edit
int command__edit(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	/*
	 * The command ":e#" gets expanded to something like ":efile", so
	 * detect that case here.
	 */
	if (*tmp_cmd == 'e' && c_argc == 0) {
		if (tmp_cmd[1] == '!')
			(void)doecmd(core, buf, &tmp_cmd[2], TRUE);
		else
			(void)doecmd(core, buf, &tmp_cmd[1], FALSE);
		return 0;
	}
	if (chk_comm(core, buf, NO_ADDR | MAX_ONE_FILE))
		return -1;
	doecmd(core, buf, c_argv[0], flags & FLAG_FORCE);
	return 0;
}

// :file
int command__file(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (chk_comm(core, buf, NO_ADDR | MAX_ONE_FILE))
		return -1;
	if (c_argc != 0)
		name = strdup(c_argv[0]);
	fileinfo(core, buf, name);
	return 0;
}

// :read
int command__read(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (chk_comm(core, buf, NO_ADDR | MAX_ONE_FILE))
		return -1;
	/* read reads a file at EOF */
	if (c_argc == 0)
		c_argv[0] = name;
	if (!addfile(core, buf, c_argv[0])) {
		edits = U_TRUNC;
		undosize = filesize;
		ui__Screen_Repaint(core, buf);
	}
	return 0;
}

// :xit
int command__xit(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (chk_comm(core, buf, NO_ADDR | NO_ARG))
		return -1;
	do_exit(core);
	return 0;
}

// :next
int command__next(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (chk_comm(core, buf, NO_ADDR | NO_ARG))
		return -1;
	if (!(flags & FLAG_FORCE)) {
		if (edits) {
			if (P(P_AW)) {
				save(core, buf, name, WRITE);
				edits = 0;
			} else {
				bvim_error(core, buf, BVI_ERROR_NOWRITE, "next");
				return -1;
			}
		}
	}
	if ((curfile + 1) < numfiles) {
		if (flags & FLAG_FORCE)
			stuffin(":e! ");
		else
			stuffin(":e ");
		stuffin(files[++curfile]);
		stuffin("\n");
	} else
		bvim_error(core, buf, "No more files@to edit!");
	return 0;
}

// :rewind
int command__rewind(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (chk_comm(core, buf, NO_ADDR | NO_ARG))
		return -1;
	if (numfiles <= 1)	/* nothing to rewind */
		return -1;
	if (!(flags & FLAG_FORCE)) {
		if (edits) {
			if (P(P_AW)) {
				save(core, buf, name, WRITE);
				edits = 0;
			} else {
				bvim_error(core, buf, BVI_ERROR_NOWRITE, "rewind");
				return -1;
			}
		}
	}
	curfile = 0;
	if (flags & FLAG_FORCE)
		stuffin(":e! ");
	else
		stuffin(":e ");
	stuffin(files[0]);
	stuffin("\n");
	return 0;
}

// :append
int command__append(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (chk_comm(core, buf, NO_ADDR | MAX_ONE_ARG))
		return -1;
	do_ins_chg(core, buf, start_addr, c_argc == 1 ? c_argv[0] : "a", U_APPEND);
	return 0;
}

// :change
int command__change(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (chk_comm(core, buf, MAX_ONE_ARG))
		return -1;
	if (!addr_flag)
		start_addr = buf->state.current;
	do_ins_chg(core, buf, start_addr, c_argc == 1 ? c_argv[0] : "a", U_EDIT);
	return 0;
}

// :mark
int command__mark(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (c_argc == 0) {
		bvim_error(core, buf, "Mark what?");
		return -1;
	}
	if (c_argc > 1 || (strlen(c_argv[0]) > 1)) {
		bvim_error(core, buf, BVI_ERROR_EXTRACHARS);
		return -1;
	}
	if (!addr_flag)
		start_addr = buf->state.current;
	do_mark(buf, c_argv[0][0], start_addr);
	return 0;
}

// :yank
int command__yank(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if ((yanked = yd_addr(core, buf)) == 0L)
		return -1;
	if ((yanked = alloc_buf(core, buf, yanked, &yank_buf)) == 0L)
		return -1;
	memcpy(yank_buf, start_addr, yanked);
	bvim_info(core, buf, "%lu bytes", (long)yanked);
	return 0;
}

// :overwrite
int command__overwrite(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (chk_comm(core, buf, NO_ARG))
		return -1;
	if (!addr_flag)
		start_addr = buf->state.current;
	do_over(core, buf, start_addr, yanked, yank_buf);
	return 0;
}

// :undo
int command__undo(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (chk_comm(core, buf, NO_ADDR | NO_ARG))
		return -1;
	do_undo(core, buf);
	return 0;
}

// :version
int command__version(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (chk_comm(core, buf, NO_ADDR | NO_ARG))
		return -1;
	bvim_info(core, buf, "bvim version %s %s", VERSION, copyright);
	return 0;
}

// :shell
int command__shell(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (chk_comm(core, buf, NO_ADDR | NO_ARG))
		return -1;
	do_shell(core, buf);
	clear();
	ui__Screen_Repaint(core, buf);
	ui__clearstr(core, buf);
	return 0;
}

// :quit
int command__quit(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (chk_comm(core, buf, NO_ADDR | NO_ARG))
		return -1;
	if (!(flags & FLAG_FORCE)) {
		if (edits) {
			bvim_error(core, buf, BVI_ERROR_NOWRITE, "quit");
			return -1;
		} else {
			if ((curfile + 1) < numfiles) {
				bvim_error(core,buf, "%d %s", numfiles - curfile - 1, BVI_ERROR_MOREFILES);
				return -1;
			} else
			quit(core, buf);
			return 0;
		}
	} else
		quit(core, buf);
	return 0;
}

// TODO: Implement math expressions calculations here

// :sleft
int command__sleft(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (c_argc == 1) {
		math__logic(core, buf, LSHIFT, c_argv[0]);
	} else if (c_argc == 0) {
		math__logic(core, buf, LSHIFT, "1");
	} else if (c_argc == 2) {
		math__logic_block(core, buf, LSHIFT, c_argv[0], atoi(c_argv[1]));
	} else {
		bvim_error(core, buf, BVI_ERROR_AMBVALUE);
		return -1;
	}
	return 0;
}

// :sright
int command__sright(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (c_argc == 1) {
		math__logic(core, buf, RSHIFT, c_argv[0]);
	} else if (c_argc == 0) {
		math__logic(core, buf, RSHIFT, "1");
	} else if (c_argc == 2) {
		math__logic_block(core, buf, RSHIFT, c_argv[0], atoi(c_argv[1]));
	} else {
		bvim_error(core, buf, BVI_ERROR_AMBVALUE);
		return -1;
	}
	return 0;
}

// :rleft
int command__rleft(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (c_argc == 1) {
		math__logic(core, buf, LROTATE, c_argv[0]);
	} else if (c_argc == 0) {
		math__logic(core, buf, LROTATE, "1");
	} else if (c_argc == 2) {
		math__logic_block(core, buf, LROTATE, c_argv[0], atoi(c_argv[1]));
	} else {
		bvim_error(core, buf, BVI_ERROR_AMBVALUE);
		return -1;
	}
	return 0;
}

// :rright
int command__rright(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (c_argc == 1) {
		math__logic(core, buf, RROTATE, c_argv[0]);
	} else if (c_argc == 0) {
		math__logic(core, buf, RROTATE, "1");
	} else if (c_argc == 2) {
		math__logic_block(core, buf, RROTATE, c_argv[0], atoi(c_argv[1]));
	} else {
		bvim_error(core, buf, BVI_ERROR_AMBVALUE);
		return -1;
	}
	return 0;
}

// :and
int command__and(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (c_argc == 1) {
		math__logic(core, buf, AND, c_argv[0]);
	} else if (c_argc == 0) {
		bvim_error(core, buf, BVI_ERROR_NOVAL);
		return -1;
	} else if (c_argc == 2) {
		math__logic_block(core, buf, AND, c_argv[0], atoi(c_argv[1]));
	} else {
		bvim_error(core, buf, BVI_ERROR_AMBVALUE);
		return -1;
	}
	return 0;
}

// :or
int command__or(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (c_argc == 1) {
		math__logic(core, buf, OR, c_argv[0]);
	} else if (c_argc == 0) {
		bvim_error(core, buf, BVI_ERROR_NOVAL);
		return -1;
	} else if (c_argc == 2) {
		math__logic_block(core, buf, OR, c_argv[0], atoi(c_argv[1]));
	} else {
		bvim_error(core, buf, BVI_ERROR_AMBVALUE);
		return -1;
	}
	return 0;
}

// :xor
int command__xor(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (c_argc == 1) {
		math__logic(core, buf, XOR, c_argv[0]);
	} else if (c_argc == 0) {
		bvim_error(core, buf, BVI_ERROR_NOVAL);
		return -1;
	} else if (c_argc == 2) {
		math__logic_block(core, buf, XOR, c_argv[0], atoi(c_argv[1]));
	} else {
		bvim_error(core, buf, BVI_ERROR_AMBVALUE);
		return -1;
	}
	return 0;
}

// :neg
int command__neg(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (c_argc != 0) {
		bvim_error(core, buf, BVI_ERROR_EXTRACHARS);
		return -1;
	}
	math__logic(core, buf, NEG, "255");
	return 0;
}

// :not
int command__not(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	if (c_argc != 0) {
		bvim_error(core, buf, BVI_ERROR_EXTRACHARS);
		return -1;
	}
	math__logic(core, buf, NOT, "255");
	return 0;
}

// TODO: Add blocks support
// TODO: Show matches in tools window, with scrolling
int command__fuz(core_t *core, buf_t *buf, char flags, int c_argc, char **c_argv) {
	struct found fnd;
	int distance;
	if (c_argc == 1) {
		distance = 1;
		fnd = fuzzy_search(core, buf, filesize, c_argv[0], strlen(c_argv[0]), FUZZY_BITAP_HAMMINGTON_DISTANCE, distance);
		bvim_info(core, buf, "Found %d matches", fnd.cnt);
	} else if (c_argc == 2) {
		distance = atoi(c_argv[0]);
		fnd = fuzzy_search(core, buf, filesize, c_argv[1], strlen(c_argv[1]), FUZZY_BITAP_HAMMINGTON_DISTANCE, distance);
		bvim_info(core, buf, "Found %d matches", fnd.cnt);
	} else {
		bvim_error(core, buf, BVI_ERROR_EXTRACHARS);
		return -1;
	}
	return 0;
}

/* =============== End of command handlers ================== */

/*
 * docmdline() - handle a colon command
 *
 * Handles a colon command received interactively by getcmdln() or from
 * the environment variable "BVIINIT" (or eventually .bvimrc).
 */
void docmdline(core_t *core, buf_t *buf, char* cmdline)
{
	char buff[CMDSZ];
	char cmdbuf[CMDSZ];
	char cmdname[MAXNAME];
	char *p;
	size_t len;
	int n, ok, j = 0;
	char flags = 0;
	int saveflag;
	int shresult;
	char *cmd;
	int cmd_recognized = 0;

	if (cmdline == NULL)
		return;
	if (*cmdline == '"')
		return;			/** comment **/
	if (strlen(cmdline) > CMDSZ - 2) {
		bvim_error(core, buf, "Command too long");
		return;
	}
	strcpy(buff, cmdline);
	cmd = buff;

	/* With no address given, we start at the beginning of the file and
	 * go to the end of the file (not line like in vi).
	 */
	addr_flag = 0;
	start_addr = buf->mem;
	end_addr = buf->maxpos - 1;
	SKIP_WHITE if (*cmd == '%') {
		cmd++;
		addr_flag = 2;
	} else {
		if ((start_addr = calc_addr(core, buf, &cmd, buf->mem)) == NULL) {
			return;
		}
		if (*cmd == ',') {
			cmd++;
			addr_flag = 1;
			SKIP_WHITE
			    if ((end_addr =
				 calc_addr(core, buf, &cmd, buf->maxpos - 1)) == NULL) {
				return;
			}
		} else {
			if (addr_flag) {
				/* if we have only one address */
				end_addr = start_addr;
			}
		}
	}
	SKIP_WHITE if (start_addr > (end_addr + 1)) {
		bvim_error(core, buf, "Addr1 > addr2|First address exceeds second");
		return;
	}
	if ((end_addr + 1) > buf->maxpos) {
		bvim_error(core, buf, "Not that many bytes@in buffer");
		return;
	}
	if (start_addr < buf->mem) {
		bvim_error(core, buf, "Negative address@- first byte is %ld", P(P_OF));
		return;
	}

	SKIP_WHITE
	/**** End of address range calculation ****/
	
	if (*cmd == '\0') {
		setpage(core, buf, end_addr);
		return;
	}
	strcpy(cmdbuf, cmd);	/* save the unmodified command */
	record_cmd(core, cmd);

	if (*cmd == '!') {
		if (*(cmdbuf + 1) == '\0') {
			bvim_error(core, buf, "Incomplete shell escape command@- use 'shell' to get a shell");
			return;
		}
		if (*(cmdbuf + 1) == '!') {
			if (oldbuf[0] != '\0') {
				strcpy(cmdbuf + 1, oldbuf);
				bvim_info(core, buf, oldbuf);
			} else {
				bvim_error(core, buf, "No previous command@to substitute for !");
				return;
			}
		} else
			sprintf(oldbuf, "\n%s\n", cmdbuf + 1);

		if (P(P_AW)) {
			save(core, buf, name, WRITE);
			edits = 0;
		}
		if (edits)
			bvim_info(core, buf, "[No write]|[No write since last change]");
		savetty();
		endwin();
		shresult = system(cmdbuf + 1);
		if (shresult == SIGQUIT)
			fprintf(stderr, "[Shell execution error]");
		fprintf(stderr, "[Hit return to continue]");
		getchar();
		doupdate();
		ui__Screen_Repaint(core, buf);
		ui__clearstr(core, buf);
		return;
	}

	n = 0;
	while (*cmd >= 'a' && *cmd <= 'z') {
		cmdname[n++] = *cmd++;
		if (n == MAXNAME) {
			bvim_error(core, buf, "What?|%s: Not an editor command 1", cmdbuf);
			return;
		}
	}
	cmdname[n] = '\0';
	if (*cmd == '!') {
		flags |= FLAG_FORCE;
		cmd++;
	}
	len = strlen(cmdname);
	SKIP_WHITE if (!strncmp("substitute", cmdname, len) && CMDLNG(10, 1)) {
		repl_count = do_substitution(core, buf, *cmd, cmd + 1, start_addr, end_addr);
		if (repl_count == -1) {
			bvim_error(core, buf, "No previous substitute re|No previous substitute regular expression");
			return;	/* No prev subst */
		}
		ui__Screen_Repaint(core, buf);
		if (!repl_count) {
			bvim_error(core, buf, "Fail|Substitute pattern matching failed");
		} else if (repl_count > 1) {
			bvim_info(core, buf, "%d subs|%d substitutions", repl_count, repl_count);
		}
		return;
	} else if (!strncmp("global", cmdname, len) && CMDLNG(6, 1)) {
		buf->state.current = start_addr - 1;
		repl_count = 0;
		addch('\n');
		while ((buf->state.current = searching(core, buf, *cmd, cmd + 1, buf->state.current, end_addr, FALSE | S_GLOBAL)) != 0L) {
			addch('\n');
			ui__Line_Print(core, buf, buf->state.current, core->screen.maxy - 1);
			repl_count++;
			if (repl_count == LINES - 2) {
				if (wait_return(core, buf, FALSE))
					return;
				repl_count = 0;
			}
		}
		if (repl_count) {
			wait_return(core, buf, TRUE);
		} else {
			ui__Screen_Repaint(core, buf);
			bvim_error(core, buf, BVI_ERROR_PATNOTFOUND);
		}
		return;
	} else if (cmdname[0] == 't') {
		if (!addr_flag)
			start_addr = buf->state.current;
		cmd = cmdname + 1L;
		SKIP_WHITE do_mark(buf, *cmd, start_addr);
		return;
	}

	if ((!strncmp("write", cmdname, len) && CMDLNG(5, 1))
	    || !strcmp("wq", cmdname)) {
		if (*cmd == '>') {
			if (*(++cmd) == '>') {
				cmd++;
				saveflag = APPEND;
			} else {
				bvim_error(core, buf, "Write forms are 'w' and 'w>>'");
				return;
			}
		} else if (*cmd == '!') {
			bvim_error(core, buf, "Not yet implemented");
			return;
		} else {
			saveflag = WRITE;
		}
		SKIP_WHITE c_argc = 0;
		if ((c_argv[c_argc] = strtok(cmd, " \t")) != NULL)
			c_argc++;
		while ((c_argv[c_argc] = strtok(NULL, " \t")) != NULL)
			c_argc++;

		if (c_argc > 1) {
			bvim_error(core, buf, BVI_ERROR_AMBIGOUS);
			return;
		}
		if (c_argc == 1) {
			/* change '%' to Filename */
			while ((p = strchr(c_argv[0], '%')) != NULL
			       && *(p - 1) != '\\') {
				if (name == NULL) {
					bvim_error(core, buf, "No filename@to substitute for %");
					return;
				}
				*p = '\0';
				strcpy(oldbuf, c_argv[0]);
				strcat(oldbuf, name);
				strcat(oldbuf, p + 1);
				c_argv[0] = oldbuf;
			}
		}
		if (name == NULL && c_argc != 0)
			name = strdup(c_argv[0]);
		if (flags & FLAG_FORCE) {
			if (c_argc == 0)
				ok = save(core, buf, name, saveflag);
			else
				ok = save(core, buf, c_argv[0], saveflag);
		} else {
			if (c_argc == 0) {
				if (P(P_RO)) {
					bvim_error(core, buf, "\"%s\" File is read only", name);
					return;
				} else
					ok = save(core, buf, name, saveflag);
			} else {
				if (!stat(c_argv[0], &bbuf)) {
					if (saveflag == WRITE) {
						bvim_error(core, buf, "File exists@- use \"%s! %s\" to overwrite", cmdname, c_argv[0]);
						return;
					} else {	/* APPEND */
					/* We can only append to a regular file! */
						if (S_ISREG(bbuf.st_mode)) {
							if (filemode == PARTIAL)
								filemode =
								    REGULAR;
						} else if (S_ISBLK(bbuf.st_mode)) {
							bvim_error(core, buf, "Cannot append to a block special file");
							return;
						}
						ok = save(core, buf, c_argv[0], saveflag);
					}
				} else {
					if (saveflag == APPEND) {
						bvim_error(core, buf, "No such file");
						return;
					} else {	/* WRITE */
					/* If we write the block of a partial file to a new file, it will
					 * become a regular file!
					 */
						if (filemode == PARTIAL)
							filemode = REGULAR;
						ok = save(core, buf, c_argv[0], saveflag);
					}
				}
			}
		}
		if (!strcmp("wq", cmdname)) {
			if (ok)
				quit(core, buf);
		}
		return;
	}

	c_argc = 0;
	cmd = strtok(cmdbuf, " \t");

	/* Extract command handlers from here */

	while ((c_argv[c_argc] = strtok(NULL, " \t")) != NULL)
		c_argc++;

	tmp_cmd = cmd; // save original command for some parsing purposes
	j = 0;
	while (j < core->cmdmap.items) {
		if (!strncmp(core->cmdmap.arr[j].name, cmdname, len) && CMDLNG(core->cmdmap.arr[j].size1, core->cmdmap.arr[j].size2) &&
			(core->cmdmap.arr[j].enabled == 1)) {
			if ((core->cmdmap.arr[j].handler_type == BVI_HANDLER_INTERNAL) & (core->cmdmap.arr[j].handler.func != NULL)) {
				(*(core->cmdmap.arr[j].handler.func)) (core, buf, flags, c_argc, c_argv);
				cmd_recognized = 1;
				break;
			} else {
			    if ((core->cmdmap.arr[j].handler_type == BVI_HANDLER_LUA) & (core->cmdmap.arr[j].handler.lua_cmd != NULL)) {
					bvim_run_lua_string(core, buf, core->cmdmap.arr[j].handler.lua_cmd);
					cmd_recognized = 1;
					break;
				}
			}
		}
		j++;
	}
	if (cmd_recognized == 0) {
		sprintf(string, "What?|%s: Not an editor command", cmd);
		if P
			(P_MM) {
			if (!strncmp("delete", cmdname, len) && CMDLNG(6, 1)) {
				if ((undo_count = yd_addr(core, buf)) == 0L)
					return;
				do_delete(core, buf, undo_count, start_addr);
				bvim_info(core, buf, "%lu bytes", (long)undo_count);
			} else if (!strncmp("insert", cmdname, len)
				   && CMDLNG(6, 1)) {
				if (chk_comm(core, buf, MAX_ONE_ARG))
					return;
				if (!addr_flag)
					start_addr = buf->state.current;
				do_ins_chg(core, buf, start_addr - 1,
					   c_argc == 1 ? c_argv[0] : "a", U_INSERT);
			} else if (!strncmp("put", cmdname, len)
				   && CMDLNG(3, 2)) {
				if (chk_comm(core, buf, NO_ARG))
					return;
				if (!addr_flag)
					start_addr = buf->state.current;
				do_put(core, buf, start_addr, yanked, yank_buf);
			} else {
				bvim_error(core, buf, string);
			}
		} else {
			if (!strncmp("delete", cmdname, len) && CMDLNG(6, 1)) {
				movebyte(core, buf);
			} else if (!strncmp("insert", cmdname, len)
				   && CMDLNG(6, 1)) {
				movebyte(core, buf);
			} else if (!strncmp("put", cmdname, len)
				   && CMDLNG(3, 2)) {
				movebyte(core, buf);
			} else {
				bvim_error(core, buf, string);
			}
		}
	}
}

/* calculate address range for :yank and :delete command */
off_t yd_addr(core_t *core, buf_t *buf)
{
	off_t count = 0;

	if (c_argc == 0) {
		switch (addr_flag) {
		case 0:
			start_addr = buf->state.current;
		case 1:
			count = 1;
			break;
		case 2:
			count = end_addr - start_addr + 1;
			break;
		}
	} else if (c_argc == 1) {
		count = atoi(c_argv[0]);
		switch (addr_flag) {
		case 0:
			start_addr = buf->state.current;
		case 1:
			end_addr = start_addr + count - 1;
			break;
		case 2:
			start_addr = end_addr;
			end_addr = start_addr + count - 1;
			break;
		}
	} else {
		bvim_error(core, buf, BVI_ERROR_AMBVALUE);
		return 0;
	}
	return count;
}

void do_exit(core_t *core)
{
// FIXME: implement function, which check all buffers
	if (edits) {
		if (!save(core, core->curbuf, name, WRITE))
			return;
	}
	if ((curfile + 1) < numfiles) {
		bvim_error(core, core->curbuf, "%d %s", numfiles - curfile - 1, BVI_ERROR_MOREFILES);
	} else {
#ifdef HAVE_LUA_H
		bvim_lua_finish(core);
#endif
		quit(core, core->curbuf);
	}
}

int doecmd(core_t *core, buf_t *buf, char* arg, int flags)
{
	char *tmp;

	if (*arg == '\0')
		arg = NULL;
	if (!(flags & FLAG_FORCE) && edits) {
		bvim_error(core, buf, BVI_ERROR_NOWRITE, "edit");
		/*
		   if (altfile)
		   free(altfile);
		   altfile = strdup(arg);
		 */
		return FALSE;
	}
	if (arg != NULL) {
		if (name != NULL && !strcmp(arg, name)) {
			if (!edits || (edits && !(flags & FLAG_FORCE)))
				return TRUE;
		}
		if (name != NULL && !strcmp(arg, "#")) {
			if (altfile == NULL) {
				bvim_error(core, buf, "No alternate filename@to substitute for #");
				return FALSE;
			}
			tmp = name;
			name = altfile;
			altfile = tmp;
		} else {
			if (altfile) {
				free(altfile);
			}
			altfile = name;
			name = strdup(arg);
		}
	}
	if (name == NULL) {
		bvim_error(core, buf, "No file|No current filename");
		return FALSE;
	}

	edits = 0;
	filesize = load(core, buf, name);
	if (arg == NULL) {
		setpage(core, buf, buf->state.current < buf->maxpos ? buf->state.current : buf->maxpos - 1L);
	}
	return TRUE;
}

/* If flag == TRUE we do a ui__Screen_Repaint */
int wait_return(core_t *core, buf_t *buf,  int flag)
{
	int c;

	signal(SIGINT, jmpproc);
	ui__clearstr(core, buf);
	attrset(A_REVERSE);
	mvaddstr(core->screen.maxy, 0, "[Hit return to continue]");
	attrset(A_NORMAL);
	refresh();
	c = getch();
	if (flag) {
		clear();
		ui__Screen_Repaint(core, buf);
	}
	ui__clearstr(core, buf);
	signal(SIGINT, SIG_IGN);
	if (c != CR && c != NL && c != ' ' && c != ':' && c != KEY_ENTER)
		return 1;
	return 0;
}

/* check command arguments for integrity */
int chk_comm(core_t *core, buf_t* buf, int flag)
{
	if ((flag & NO_ADDR) && (addr_flag > 0)) {
		bvim_error(core, buf, BVI_ERROR_NOADDR);
		return 1;
	}
	if ((flag & NO_ARG) && (c_argc > 0)) {
		bvim_error(core, buf, BVI_ERROR_EXTRACHARS);
		return 1;
	}
	if ((flag & MAX_ONE_ARG) && (c_argc > 1)) {
		bvim_error(core, buf, BVI_ERROR_AMBVALUE);
		return 1;
	}
	if ((flag & ONE_FILE) && (c_argc == 0)) {
		bvim_error(core, buf, "Missing filename");
		return 1;
	}
	if ((flag & MAX_ONE_FILE) && (c_argc > 1)) {
		bvim_error(core, buf, BVI_ERROR_AMBIGOUS);
		return 1;
	}
	return 0;
}

