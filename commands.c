/* COMM.C - Routines to parse and execute "command line" commands, such as
 * search or colon commands.
 *
 * NOTE: Edit this file with tabstop=4 !
 *
 * Copyright 1996-2003 by Gerhard Buergmann gerhard@puon.at
 * Copyright 2011 by Anton Kochkov anton.kochkov@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * See file COPYING for information on distribution conditions.
 */

#include "bvi.h"
#include "keys.h"
#include "blocks.h"
#include "math.h"
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
static struct stat buf;
static int c_argc = 0;
static char *c_argv[9];

// TODO: move messages to messages.h

char *nowrtmsg = "No write@since last change (:%s! overrides)";
char *morefiles = "more files@to edit";
char *ambigous = "Ambigous|Too many file names";
char *ambvalue = "Ambigous|Too many values";
char *extra = "Extra chars|Extra characters at end of command";
char *noaddr = "No address allowed@on this command";
char *noval = "No value@for binary operation";
int repl_count = -1;
int addr_flag;

PTR start_addr;
PTR end_addr;

extern core_t core;
extern state_t state;

extern char *name;		/* actual filename */
extern char **files;		/* used for "next" and "rewind" */
extern int numfiles, curfile;
extern int errno;

static char oldbuf[CMDSZ];		/** for :!! command **/

/* =================== Commands storage ===================== */

static struct command_array cmdmap;

int CmdAdd(struct command item)
{
	if (cmdmap.items == cmdmap.allocated) {
		if (cmdmap.allocated == 0)
			cmdmap.allocated = 3;
		else
			cmdmap.allocated += 4;

		void *_tmp =
		    realloc(cmdmap.arr,
			    (cmdmap.allocated * sizeof(struct command)));

		if (!_tmp) {
			fprintf(stderr, "ERROR: Couldn't realloc memory!\n");
			return (-1);
		}
		cmdmap.arr = (struct command *)_tmp;
	}
	cmdmap.arr[cmdmap.items] = item;
	cmdmap.items++;

	return cmdmap.items;
}

int CmdDel(char *name)
{
	int i = 0;
	while (i < cmdmap.items) {
		if (strcmp(name, cmdmap.arr[i].name) == 0) {
			cmdmap.arr[i].enabled = 0;
			return 0;
		}
		i++;
	}
	return -1;
}

int CmdDefaults()
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
		{ 0, NULL, NULL, 0, 0, { NULL }, 0, 0 }
	};
	while (cmds_def[i].id != 0) {
		CmdAdd(cmds_def[i]);
		i++;
	}
	return 0;
}


void commands__Init()
{
	cmdmap.arr = NULL;
	cmdmap.items = 0;
	cmdmap.allocated = 0;
	CmdDefaults();
}

void commands__Destroy()
{
	free(cmdmap.arr);
}

int commands__Cmd_Add(struct command *new_cmd)
{
	CmdAdd(*new_cmd);
	return 0;
}

int commands__Cmd_Del(char* name)
{
	CmdDel(name);
	return 0;
}

/* =================== Command handlers ===================== */

char luacmdbuf[CMDSZ];

static char* tmp_cmd;

// :help
int command__help(char flags, int c_argc, char **c_argv) {
	int i = 0;
	if (c_argc == 0) {
		ui__ErrorMsg("What?");
	} else if (c_argc == 1) {
		while (i < cmdmap.items) {
			if (strncmp(c_argv[0], cmdmap.arr[i].name, strlen(cmdmap.arr[i].name))) {
				ui__ErrorMsg("Command not exist!");
			} else {
				ui__StatusMsg(cmdmap.arr[i].description);
				return 0;
			}
			i++;
		}
	}
	return -1;
}

// :map
int command__map(char flags, int c_argc, char **c_argv) {
	int n = 0;

	struct key *key_tmp;

	if (c_argc == 0) {
		ui__ErrorMsg("Error: empty mapping definition!");
	} else if (c_argc == 1) {
		if (strncmp(c_argv[0], "all", 3)) {
			ui__ErrorMsg("Map what?");
		} else {
			keys__KeyMaps_Show();
		}
	} else {
		luacmdbuf[0] = '\0';
		for (n = 1; n < c_argc; n++) {
			strcat(luacmdbuf, " ");
			strcat(luacmdbuf, c_argv[n]);
		}
		key_tmp = keys__KeyString_Parse(c_argv[0]);
		if (!keys__Key_Map(key_tmp))
			ui__ErrorMsg("Error: can't set new key mapping!");
		return 0;
	}
	return -1;
}

// :unmap
int command__unmap(char flags, int c_argc, char **c_argv) {
	struct key *key_tmp;

	if (c_argc != 1) {
		ui__ErrorMsg("Error: empty mapping definition!");
	} else {
		key_tmp = keys__KeyString_Parse(c_argv[0]);
		if (!keys__Key_Unmap(key_tmp))
			ui__ErrorMsg("Error: can't remove key mapping!");
		return 0;
	}
	return -1;
}


// :set
int command__set(char flags, int c_argc, char **c_argv) {
	int n  = 0;
	if (chk_comm(NO_ADDR))
		return -1;
	if (c_argc == 0) {
		doset(NULL);
	} else {
		for (n = 0; n < c_argc; n++) {
			if (doset(c_argv[n]))
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
int command__block(char flags, int c_argc, char **c_argv) {
	char size[256];
	struct block_item tmp_blk;
	
	// Need to be removed/cleared
	char tmptmp[256];
	tmptmp[0] = '\0';
	struct block_item *blkblk;

	size[0] = '\0';

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
					blocks__Add(tmp_blk);
					// Adding block to highlights
					ui__BlockHighlightAdd(&tmp_blk);
					// this we need to remove too
					blkblk = blocks__GetByID(tmp_blk.id);
					sprintf(tmptmp, "Added block: start %ld end %ld", blkblk->pos_start, blkblk->pos_end);
					
					ui__Screen_Repaint();

					// And this need to be destroyed too
					ui__ErrorMsg(tmptmp);
				} else {
					ui__ErrorMsg("Wrong block start and end values!");
					return -1;
				}
			} else {
				ui__ErrorMsg("Wrong :block command format!");
				return -1;
			}
		/* :block del */
		} else if (!strncmp(c_argv[0], "del", 3)) {
		/* :block info */
		} else if (!strncmp(c_argv[0], "info", 4)) {
		/* :block list */
		} else if (!strncmp(c_argv[0], "list", 4)) {
		/* :block save */
		} else if (!strncmp(c_argv[0], "save", 4)) {
		/* :block yank */
		} else if (!strncmp(c_argv[0], "yank", 4)) {
		/* :block fold */
		} else if (!strncmp(c_argv[0], "fold", 4)) {
			if (c_argc == 1) {
				blkblk = blocks__GetByID((unsigned int)atoi(c_argv[1])); // block id
				if (blkblk != NULL) {
					blkblk->folding = 1;
					ui__Screen_Repaint();
				}
			}
		} else {
			ui__ErrorMsg("Wrong :block command format!");
			return -1;
		}
	}
	return 0;
}

// :lua
int command__lua(char flags, int c_argc, char **c_argv) {
	int n = 0;

	if (c_argc == 0) {
		ui__ErrorMsg("Error: empty lua command!");
	} else {
		luacmdbuf[0] = '\0';
		for (n = 0; n < c_argc; n++) {
			strcat(luacmdbuf, " ");
			strcat(luacmdbuf, c_argv[n]);
		}
		bvi_run_lua_string(luacmdbuf);
	}
	return 0;
}

// :args
int command__args(char flags, int c_argc, char **c_argv) {
	int n = 0;

	if (chk_comm(NO_ADDR | NO_ARG))
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
	ui__StatusMsg(string);
	return 0;
}

// :source
int command__source(char flags, int c_argc, char **c_argv) {
	if (chk_comm(NO_ADDR | ONE_FILE))
		return -1;
	if (read_rc(c_argv[0]))
		ui__SystemErrorMsg(c_argv[0]);
	refresh();
	return 0;
}

// :run
int command__run(char flags, int c_argc, char **c_argv) {
	if (c_argc == 0) {
		ui__ErrorMsg("Error: empty plugin name!");
	} else {
		bvi_run_lua_script(c_argv[0]);
	}
	return 0;
}

// :cd
int command__cd(char flags, int c_argc, char **c_argv) {
	if (chk_comm(NO_ADDR | ONE_FILE))
		return -1;
	if (!(flags & FLAG_FORCE)) {
		if (edits) {
			if (P(P_AW)) {
				save(name, core.editor.mem, maxpos, WRITE);
				edits = 0;
			} else {
				sprintf(string, nowrtmsg, "cd");
				ui__ErrorMsg(string);
				return -1;
			}
		}
	}
	if (c_argc == 0) {
		sprintf(string, "%c", DELIM);
		c_argv[0] = string;
	}
	if (chdir(c_argv[0]))
		ui__SystemErrorMsg(c_argv[0]);
	return 0;
}

// :edit
int command__edit(char flags, int c_argc, char **c_argv) {
	/*
	 * The command ":e#" gets expanded to something like ":efile", so
	 * detect that case here.
	 */
	if (*tmp_cmd == 'e' && c_argc == 0) {
		if (tmp_cmd[1] == '!')
			(void)doecmd(&tmp_cmd[2], TRUE);
		else
			(void)doecmd(&tmp_cmd[1], FALSE);
		return 0;
	}
	if (chk_comm(NO_ADDR | MAX_ONE_FILE))
		return -1;
	doecmd(c_argv[0], flags & FLAG_FORCE);
	return 0;
}

// :file
int command__file(char flags, int c_argc, char **c_argv) {
	if (chk_comm(NO_ADDR | MAX_ONE_FILE))
		return -1;
	if (c_argc != 0)
		name = strdup(c_argv[0]);
	fileinfo(name);
	return 0;
}

// :read
int command__read(char flags, int c_argc, char **c_argv) {
	if (chk_comm(NO_ADDR | MAX_ONE_FILE))
		return -1;
	/* read reads a file at EOF */
	if (c_argc == 0)
		c_argv[0] = name;
	if (!addfile(c_argv[0])) {
		edits = U_TRUNC;
		undosize = filesize;
		ui__Screen_Repaint();
	}
	return 0;
}

// :xit
int command__xit(char flags, int c_argc, char **c_argv) {
	if (chk_comm(NO_ADDR | NO_ARG))
		return -1;
	do_exit();
	return 0;
}

// :next
int command__next(char flags, int c_argc, char **c_argv) {
	if (chk_comm(NO_ADDR | NO_ARG))
		return -1;
	if (!(flags & FLAG_FORCE)) {
		if (edits) {
			if (P(P_AW)) {
				save(name, core.editor.mem, maxpos, WRITE);
				edits = 0;
			} else {
				sprintf(string, nowrtmsg, "next");
				ui__ErrorMsg(string);
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
		ui__ErrorMsg("No more files@to edit!");
	return 0;
}

// :rewind
int command__rewind(char flags, int c_argc, char **c_argv) {
	if (chk_comm(NO_ADDR | NO_ARG))
		return -1;
	if (numfiles <= 1)	/* nothing to rewind */
		return -1;
	if (!(flags & FLAG_FORCE)) {
		if (edits) {
			if (P(P_AW)) {
				save(name, core.editor.mem, maxpos, WRITE);
				edits = 0;
			} else {
				sprintf(string, nowrtmsg, "rewind");
				ui__ErrorMsg(string);
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
int command__append(char flags, int c_argc, char **c_argv) {
	if (chk_comm(NO_ADDR | MAX_ONE_ARG))
		return -1;
	do_ins_chg(start_addr, c_argc == 1 ? c_argv[0] : "a", U_APPEND);
	return 0;
}

// :change
int command__change(char flags, int c_argc, char **c_argv) {
	if (chk_comm(MAX_ONE_ARG))
		return -1;
	if (!addr_flag)
		start_addr = current;
	do_ins_chg(start_addr, c_argc == 1 ? c_argv[0] : "a", U_EDIT);
	return 0;
}

// :mark
int command__mark(char flags, int c_argc, char **c_argv) {
	if (c_argc == 0) {
		ui__ErrorMsg("Mark what?");
		return -1;
	}
	if (c_argc > 1 || (strlen(c_argv[0]) > 1)) {
		ui__ErrorMsg(extra);
		return -1;
	}
	if (!addr_flag)
		start_addr = current;
	do_mark(c_argv[0][0], start_addr);
	return 0;
}

// :yank
int command__yank(char flags, int c_argc, char **c_argv) {
	if ((yanked = yd_addr()) == 0L)
		return -1;
	if ((yanked = alloc_buf(yanked, &yank_buf)) == 0L)
		return -1;
	memcpy(yank_buf, start_addr, yanked);
	sprintf(string, "%lu bytes", (long)yanked);
	ui__StatusMsg(string);
	return 0;
}

// :overwrite
int command__overwrite(char flags, int c_argc, char **c_argv) {
	if (chk_comm(NO_ARG))
		return -1;
	if (!addr_flag)
		start_addr = current;
	do_over(start_addr, yanked, yank_buf);
	return 0;
}

// :undo
int command__undo(char flags, int c_argc, char **c_argv) {
	if (chk_comm(NO_ADDR | NO_ARG))
		return -1;
	do_undo();
	return 0;
}

// :version
int command__version(char flags, int c_argc, char **c_argv) {
	if (chk_comm(NO_ADDR | NO_ARG))
		return -1;
	sprintf(string, "bvi version %s %s", VERSION, copyright);
	ui__StatusMsg(string);
	return 0;
}

// :shell
int command__shell(char flags, int c_argc, char **c_argv) {
	if (chk_comm(NO_ADDR | NO_ARG))
		return -1;
	do_shell();
	clear();
	ui__Screen_Repaint();
	clearstr();
	return 0;
}

// :quit
int command__quit(char flags, int c_argc, char **c_argv) {
	if (chk_comm(NO_ADDR | NO_ARG))
		return -1;
	if (!(flags & FLAG_FORCE)) {
		if (edits) {
			sprintf(string, nowrtmsg, "quit");
			ui__ErrorMsg(string);
			return -1;
		} else {
			if ((curfile + 1) < numfiles) {
				sprintf(string, "%d %s", numfiles - curfile - 1, morefiles);
				ui__ErrorMsg(string);
				return -1;
			} else
			quit();
			return 0;
		}
	} else
		quit();
	return 0;
}

// TODO: Implement math expressions calculations here

// :sleft
int command__sleft(char flags, int c_argc, char **c_argv) {
	if (c_argc == 1) {
		do_logic(LSHIFT, c_argv[0]);
	} else if (c_argc == 0) {
		do_logic(LSHIFT, "1");
	} else if (c_argc == 2) {
		do_logic_block(LSHIFT, c_argv[0], atoi(c_argv[1]));
	} else {
		ui__ErrorMsg(ambvalue);
		return -1;
	}
	return 0;
}

// :sright
int command__sright(char flags, int c_argc, char **c_argv) {
	if (c_argc == 1) {
		do_logic(RSHIFT, c_argv[0]);
	} else if (c_argc == 0) {
		do_logic(RSHIFT, "1");
	} else if (c_argc == 2) {
		do_logic_block(RSHIFT, c_argv[0], atoi(c_argv[1]));
	} else {
		ui__ErrorMsg(ambvalue);
		return -1;
	}
	return 0;
}

// :rleft
int command__rleft(char flags, int c_argc, char **c_argv) {
	if (c_argc == 1) {
		do_logic(LROTATE, c_argv[0]);
	} else if (c_argc == 0) {
		do_logic(LROTATE, "1");
	} else if (c_argc == 2) {
		do_logic_block(LROTATE, c_argv[0], atoi(c_argv[1]));
	} else {
		ui__ErrorMsg(ambvalue);
		return -1;
	}
	return 0;
}

// :rright
int command__rright(char flags, int c_argc, char **c_argv) {
	if (c_argc == 1) {
		do_logic(RROTATE, c_argv[0]);
	} else if (c_argc == 0) {
		do_logic(RROTATE, "1");
	} else if (c_argc == 2) {
		do_logic_block(RROTATE, c_argv[0], atoi(c_argv[1]));
	} else {
		ui__ErrorMsg(ambvalue);
		return -1;
	}
	return 0;
}

// :and
int command__and(char flags, int c_argc, char **c_argv) {
	if (c_argc == 1) {
		do_logic(AND, c_argv[0]);
	} else if (c_argc == 0) {
		ui__ErrorMsg(noval);
		return -1;
	} else if (c_argc == 2) {
		do_logic_block(AND, c_argv[0], atoi(c_argv[1]));
	} else {
		ui__ErrorMsg(ambvalue);
		return -1;
	}
	return 0;
}

// :or
int command__or(char flags, int c_argc, char **c_argv) {
	if (c_argc == 1) {
		do_logic(OR, c_argv[0]);
	} else if (c_argc == 0) {
		ui__ErrorMsg(noval);
		return -1;
	} else if (c_argc == 2) {
		do_logic_block(OR, c_argv[0], atoi(c_argv[1]));
	} else {
		ui__ErrorMsg(ambvalue);
		return -1;
	}
	return 0;
}

// :xor
int command__xor(char flags, int c_argc, char **c_argv) {
	if (c_argc == 1) {
		do_logic(XOR, c_argv[0]);
	} else if (c_argc == 0) {
		ui__ErrorMsg(noval);
		return -1;
	} else if (c_argc == 2) {
		do_logic_block(XOR, c_argv[0], atoi(c_argv[1]));
	} else {
		ui__ErrorMsg(ambvalue);
		return -1;
	}
	return 0;
}

// :neg
int command__neg(char flags, int c_argc, char **c_argv) {
	if (c_argc != 0) {
		ui__ErrorMsg(extra);
		return -1;
	}
	do_logic(NEG, "255");
	return 0;
}

// :not
int command__not(char flags, int c_argc, char **c_argv) {
	if (c_argc != 0) {
		ui__ErrorMsg(extra);
		return -1;
	}
	do_logic(NOT, "255");
	return 0;
}
	

/* =============== End of command handlers ================== */

/*
 * docmdline() - handle a colon command
 *
 * Handles a colon command received interactively by getcmdln() or from
 * the environment variable "BVIINIT" (or eventually .bvirc).
 */
void docmdline(char* cmdline)
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
		ui__ErrorMsg("Command too long");
		return;
	}
	strcpy(buff, cmdline);
	cmd = buff;

	/* With no address given, we start at the beginning of the file and
	 * go to the end of the file (not line like in vi).
	 */
	addr_flag = 0;
	start_addr = core.editor.mem;
	end_addr = maxpos - 1;
	SKIP_WHITE if (*cmd == '%') {
		cmd++;
		addr_flag = 2;
	} else {
		if ((start_addr = calc_addr(&cmd, core.editor.mem)) == NULL) {
			return;
		}
		if (*cmd == ',') {
			cmd++;
			addr_flag = 1;
			SKIP_WHITE
			    if ((end_addr =
				 calc_addr(&cmd, maxpos - 1)) == NULL) {
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
		ui__ErrorMsg("Addr1 > addr2|First address exceeds second");
		return;
	}
	if ((end_addr + 1) > maxpos) {
		ui__ErrorMsg("Not that many bytes@in buffer");
		return;
	}
	if (start_addr < core.editor.mem) {
		sprintf(string, "Negative address@- first byte is %ld",
			P(P_OF));
		ui__ErrorMsg(string);
		return;
	}

	SKIP_WHITE
	/**** End of address range calculation ****/
	
	if (*cmd == '\0') {
		setpage(end_addr);
		return;
	}
	strcpy(cmdbuf, cmd);	/* save the unmodified command */
	record_cmd(cmd);

	if (*cmd == '!') {
		if (*(cmdbuf + 1) == '\0') {
			ui__ErrorMsg
			    ("Incomplete shell escape command@- use 'shell' to get a shell");
			return;
		}
		if (*(cmdbuf + 1) == '!') {
			if (oldbuf[0] != '\0') {
				strcpy(cmdbuf + 1, oldbuf);
				ui__StatusMsg(oldbuf);
			} else {
				ui__ErrorMsg
				    ("No previous command@to substitute for !");
				return;
			}
		} else
			sprintf(oldbuf, "\n%s\n", cmdbuf + 1);

		if (P(P_AW)) {
			save(name, core.editor.mem, maxpos, WRITE);
			edits = 0;
		}
		if (edits)
			ui__StatusMsg("[No write]|[No write since last change]");
		savetty();
		endwin();
		shresult = system(cmdbuf + 1);
		if (shresult == SIGQUIT)
			fprintf(stderr, "[Shell execution error]");
		fprintf(stderr, "[Hit return to continue]");
		getchar();
		doupdate();
		ui__Screen_Repaint();
		clearstr();
		return;
	}

	n = 0;
	while (*cmd >= 'a' && *cmd <= 'z') {
		cmdname[n++] = *cmd++;
		if (n == MAXNAME) {
			sprintf(string, "What?|%s: Not an editor command 1",
				cmdbuf);
			ui__ErrorMsg(string);
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
		repl_count =
		    do_substitution(*cmd, cmd + 1, start_addr, end_addr);
		if (repl_count == -1) {
			ui__ErrorMsg
			    ("No previous substitute re|No previous substitute regular expression");
			return;	/* No prev subst */
		}
		ui__Screen_Repaint();
		if (!repl_count) {
			ui__ErrorMsg("Fail|Substitute pattern matching failed");
		} else if (repl_count > 1) {
			sprintf(string, "%d subs|%d substitutions", repl_count,
				repl_count);
			ui__StatusMsg(string);
		}
		return;
	} else if (!strncmp("global", cmdname, len) && CMDLNG(6, 1)) {
		current = start_addr - 1;
		repl_count = 0;
		addch('\n');
		while ((current = searching(*cmd, cmd + 1, current,
					    end_addr,
					    FALSE | S_GLOBAL)) != 0L) {
			addch('\n');
			ui__Line_Print(current, core.screen.maxy - 1);
			repl_count++;
			if (repl_count == LINES - 2) {
				if (wait_return(FALSE))
					return;
				repl_count = 0;
			}
		}
		if (repl_count) {
			wait_return(TRUE);
		} else {
			ui__Screen_Repaint();
			ui__ErrorMsg(BVI_ERROR_PATNOTFOUND);
		}
		return;
	} else if (cmdname[0] == 't') {
		if (!addr_flag)
			start_addr = current;
		cmd = cmdname + 1L;
		SKIP_WHITE do_mark(*cmd, start_addr);
		return;
	}

	if ((!strncmp("write", cmdname, len) && CMDLNG(5, 1))
	    || !strcmp("wq", cmdname)) {
		if (*cmd == '>') {
			if (*(++cmd) == '>') {
				cmd++;
				saveflag = APPEND;
			} else {
				ui__ErrorMsg("Write forms are 'w' and 'w>>'");
				return;
			}
		} else if (*cmd == '!') {
			ui__ErrorMsg("Not yet implemented");
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
			ui__ErrorMsg(ambigous);
			return;
		}
		if (c_argc == 1) {
			/* change '%' to Filename */
			while ((p = strchr(c_argv[0], '%')) != NULL
			       && *(p - 1) != '\\') {
				if (name == NULL) {
					ui__ErrorMsg
					    ("No filename@to substitute for %");
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
				ok = save(name, start_addr, end_addr, saveflag);
			else
				ok = save(c_argv[0], start_addr, end_addr,
					  saveflag);
		} else {
			if (c_argc == 0) {
				if (P(P_RO)) {
					sprintf(string,
						"\"%s\" File is read only",
						name);
					ui__ErrorMsg(string);
					return;
				} else
					ok = save(name, start_addr, end_addr,
						  saveflag);
			} else {
				if (!stat(c_argv[0], &buf)) {
					if (saveflag == WRITE) {
						sprintf(string,
							"File exists@- use \"%s! %s\" to overwrite",
							cmdname, c_argv[0]);
						ui__ErrorMsg(string);
						return;
					} else {	/* APPEND */
					/* We can only append to a regular file! */
						if (S_ISREG(buf.st_mode)) {
							if (filemode == PARTIAL)
								filemode =
								    REGULAR;
						} else if (S_ISBLK(buf.st_mode)) {
							ui__ErrorMsg
							    ("Cannot append to a block special file");
							return;
						}
						ok = save(c_argv[0], start_addr,
							  end_addr, saveflag);
					}
				} else {
					if (saveflag == APPEND) {
						ui__ErrorMsg("No such file");
						return;
					} else {	/* WRITE */
					/* If we write the block of a partial file to a new file, it will
					 * become a regular file!
					 */
						if (filemode == PARTIAL)
							filemode = REGULAR;
						ok = save(c_argv[0], start_addr,
							  end_addr, saveflag);
					}
				}
			}
		}
		if (!strcmp("wq", cmdname)) {
			if (ok)
				quit();
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
	while (j < cmdmap.items) {
		if (!strncmp(cmdmap.arr[j].name, cmdname, len) && CMDLNG(cmdmap.arr[j].size1, cmdmap.arr[j].size2) &&
			(cmdmap.arr[j].enabled == 1)) {
			if ((cmdmap.arr[j].handler_type ==
			     BVI_HANDLER_INTERNAL) & (cmdmap.arr[j].handler.
						      func != NULL)) {
				(*(cmdmap.arr[j].handler.func)) (flags, c_argc, c_argv);
				cmd_recognized = 1;
				break;
			} else
			    if ((cmdmap.arr[j].handler_type ==
				 BVI_HANDLER_LUA) & (cmdmap.arr[j].handler.
						     lua_cmd != NULL)) {
				bvi_run_lua_string(cmdmap.arr[j].handler.
						   lua_cmd);
				cmd_recognized = 1;
				break;
			}
		}
		j++;
	}
	if (cmd_recognized == 0) {
		sprintf(string, "What?|%s: Not an editor command", cmd);
		if P
			(P_MM) {
			if (!strncmp("delete", cmdname, len) && CMDLNG(6, 1)) {
				if ((undo_count = yd_addr()) == 0L)
					return;
				do_delete(undo_count, start_addr);
				sprintf(string, "%lu bytes", (long)undo_count);
				ui__StatusMsg(string);
			} else if (!strncmp("insert", cmdname, len)
				   && CMDLNG(6, 1)) {
				if (chk_comm(MAX_ONE_ARG))
					return;
				if (!addr_flag)
					start_addr = current;
				do_ins_chg(start_addr - 1,
					   c_argc == 1 ? c_argv[0] : "a",
					   U_INSERT);
			} else if (!strncmp("put", cmdname, len)
				   && CMDLNG(3, 2)) {
				if (chk_comm(NO_ARG))
					return;
				if (!addr_flag)
					start_addr = current;
				do_put(start_addr, yanked, yank_buf);
			} else {
				ui__ErrorMsg(string);
			}
		} else {
			if (!strncmp("delete", cmdname, len) && CMDLNG(6, 1)) {
				movebyte();
			} else if (!strncmp("insert", cmdname, len)
				   && CMDLNG(6, 1)) {
				movebyte();
			} else if (!strncmp("put", cmdname, len)
				   && CMDLNG(3, 2)) {
				movebyte();
			} else {
				ui__ErrorMsg(string);
			}
		}
	}
}

/* calculate address range for :yank and :delete command */
off_t yd_addr()
{
	off_t count = 0;

	if (c_argc == 0) {
		switch (addr_flag) {
		case 0:
			start_addr = current;
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
			start_addr = current;
		case 1:
			end_addr = start_addr + count - 1;
			break;
		case 2:
			start_addr = end_addr;
			end_addr = start_addr + count - 1;
			break;
		}
	} else {
		ui__ErrorMsg(ambvalue);
		return 0;
	}
	return count;
}

void do_exit()
{
	if (edits) {
		if (!save(name, core.editor.mem, maxpos - 1L, WRITE))
			return;
	}
	if ((curfile + 1) < numfiles) {
		sprintf(string, "%d %s", numfiles - curfile - 1, morefiles);
		ui__ErrorMsg(string);
	} else {
#ifdef HAVE_LUA_H
		bvi_lua_finish();
#endif
		keys__Destroy();
		commands__Destroy();
		quit();
	}
}

int doecmd(char* arg, int flags)
{
	char *tmp;

	if (*arg == '\0')
		arg = NULL;
	if (!(flags & FLAG_FORCE) && edits) {
		sprintf(string, nowrtmsg, "edit");
		ui__ErrorMsg(string);
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
				ui__ErrorMsg
				    ("No alternate filename@to substitute for #");
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
		ui__ErrorMsg("No file|No current filename");
		return FALSE;
	}

	edits = 0;
	filesize = load(name);
	if (arg == NULL) {
		setpage(current < maxpos ? current : maxpos - 1L);
	}
	return TRUE;
}

/* If flag == TRUE we do a ui__Screen_Repaint */
int wait_return(int flag)
{
	int c;

	signal(SIGINT, jmpproc);
	clearstr();
	attrset(A_REVERSE);
	mvaddstr(core.screen.maxy, 0, "[Hit return to continue]");
	attrset(A_NORMAL);
	refresh();
	c = getch();
	if (flag) {
		clear();
		ui__Screen_Repaint();
	}
	clearstr();
	signal(SIGINT, SIG_IGN);
	if (c != CR && c != NL && c != ' ' && c != ':' && c != KEY_ENTER)
		return 1;
	return 0;
}

/* check command arguments for integrity */
int chk_comm(int flag)
{
	if ((flag & NO_ADDR) && (addr_flag > 0)) {
		ui__ErrorMsg(noaddr);
		return 1;
	}
	if ((flag & NO_ARG) && (c_argc > 0)) {
		ui__ErrorMsg(extra);
		return 1;
	}
	if ((flag & MAX_ONE_ARG) && (c_argc > 1)) {
		ui__ErrorMsg(ambvalue);
		return 1;
	}
	if ((flag & ONE_FILE) && (c_argc == 0)) {
		ui__ErrorMsg("Missing filename");
		return 1;
	}
	if ((flag & MAX_ONE_FILE) && (c_argc > 1)) {
		ui__ErrorMsg(ambigous);
		return 1;
	}
	return 0;
}
