/* BVI  -  Binary Visual Editor
 *
 * 1996-02-28  V 1.0.0
 * 1999-01-27  V 1.1.0
 * 1999-04-22  V 1.1.1 
 * 1999-07-01  V 1.2.0 beta
 * 1999-10-22  V 1.2.0 final
 * 2000-05-10  V 1.3.0 alpha
 * 2000-10-24  V 1.3.0 final
 * 2002-01-03  V 1.3.1
 * 2004-01-04  V 1.3.2
 *
 * NOTE: Edit this file with tabstop=4 !
 *
 * Copyright 1996-2004 by Gerhard Buergmann gerhard@puon.at
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

#include <sys/types.h>

#include "bvi.h"
#include "set.h"
#include "ui.h"
#include "keys.h"

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#ifdef HAVE_LUA_H
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "bscript.h"
#endif

#include "plugins.h"

char *copyright = "Copyright (C) 1996-2004 by Gerhard Buergmann";

jmp_buf env;			/* context for `longjmp' function   */

core_t core;
state_t state;

int x, xx, y;
int status;
off_t size;

/* Tools window */
/* WINDOW *tools_win = NULL; */


PTR mem = NULL;
PTR curpos;
PTR maxpos;
/* PTR pagepos; */
PTR spos;
char *name = NULL;
char *shell;
char string[MAXCMD];
char cmdstr[MAXCMD + 1] = "";
FILE *Ausgabe_Datei;
int edits = 0;

off_t filesize, memsize, undosize;
long precount = -1;		/* number preceding command */

int block_flag = 0;
off_t block_begin, block_end, block_size;

char **files;			/* list of input files */
int numfiles;			/* number of input files */
int curfile;			/* number of the current file */

int arrnum = 0;
char numarr[64];		/* string for collecting number */
char rep_buf[BUFFER];

PTR current;
PTR last_motion;
PTR current_start;
PTR undo_start;
off_t undo_count;
off_t yanked = 0L;
char *yank_buf = NULL;
char *undo_buf = NULL;
PTR markbuf[26];

char addr_form[15];

char *nobytes = "No bytes@in the buffer";

static char progname[8];
static char line[MAXCMD];
static int mark;
static int wrstat = 1;

extern struct MARKERS_ markers[MARK_COUNT];

/* ======================= EVENT HANDLERS ====================== */

// TODO: eliminate global variables:
int lflag;


// case '^':
int handler__goto_HEX() {
	x = core.params.COLUMNS_ADDRESS;
	state.loc = HEX;
	return 0;
}

/* case '0':
 * x = COLUMNS_ADDRESS + COLUMNS_HEX;
 * state.loc = ASCII;
 */

//case '$':
int handler__goto_ASCII() {
	x = core.params.COLUMNS_ADDRESS - 1 + core.params.COLUMNS_HEX + core.params.COLUMNS_DATA;
	state.loc = ASCII;
	return 0;
}

//case '\t':
int handler__toggle() {
	toggle();
	return 0;
}

//case '~':
int handler__tilda() {
	if (precount < 1)
		precount = 1;
	sprintf(rep_buf, "%ld~", precount);
	do_tilde(precount);
	lflag++;
	return 0;
}

//case KEY_HOME: /* go to the HOME */
//case 'H':
int handler__goto_home() {
	if (precount > 0) {
		y = --precount;
		if (y > core.screen.maxy - 1) {
			scrolldown(y - core.screen.maxy + 1);
			y = core.screen.maxy - 1;
		}
	} else
		y = 0;
	if (state.loc == HEX)
		x = core.params.COLUMNS_ADDRESS;
	else
		x = core.params.COLUMNS_ADDRESS + core.params.COLUMNS_HEX;
	return 0;
}

//case 'M':
int handler__M() {
	y = core.screen.maxy / 2;
	if ((PTR) (state.pagepos + state.screen) > maxpos)
		y = (int)(maxpos - state.pagepos) / core.params.COLUMNS_DATA / 2;
	if (state.loc == HEX)
		x = core.params.COLUMNS_ADDRESS;
	else
		x = core.params.COLUMNS_ADDRESS + core.params.COLUMNS_HEX;
	return 0;
}

//case KEY_LL:
//case 'L':
int handler__L() {
	int n = 0;
	if (precount < 1)
		precount = 1;
	n = core.screen.maxy - 1;
	if ((PTR) ((state.pagepos + state.screen)) > maxpos)
		n = (int)(maxpos - state.pagepos) / core.params.COLUMNS_DATA;
	if (precount < n)
		y = n + 1 - precount;
	if (state.loc == HEX)
		x = core.params.COLUMNS_ADDRESS;
	else
		x = core.params.COLUMNS_ADDRESS + core.params.COLUMNS_HEX;
	return 0;
}

//case BVICTRL('H'):
//case KEY_BACKSPACE:
//case KEY_LEFT:
//case 'h':
int handler__goto_left() {
	do {
		if (x > (core.params.COLUMNS_ADDRESS + 2)
			&& x < (core.params.COLUMNS_HEX + core.params.COLUMNS_ADDRESS + 1))
				x -= 3;
		else if (x > (core.params.COLUMNS_HEX + core.params.COLUMNS_ADDRESS - 2))
			x--;
	} while (--precount > 0);
	if (x < core.params.COLUMNS_ADDRESS + core.params.COLUMNS_HEX)
		state.loc = HEX;
	else
		state.loc = ASCII;
	return 0;
}

//case ' ':
//case KEY_RIGHT:
//case 'l':
int handler__goto_right() {
	do {
		/*
		 if (x < (COLUMNS_HEX + 6))  x += 3;
		*/
		if (x < (core.params.COLUMNS_HEX + core.params.COLUMNS_ADDRESS - 2))
			x += 3;
		else if (x > (core.params.COLUMNS_HEX + 3)
			&& x < (core.params.COLUMNS_HEX + core.params.COLUMNS_ADDRESS - 1 + core.params.COLUMNS_DATA))
				x++;
	} while (--precount > 0);
	if (x < core.params.COLUMNS_ADDRESS + core.params.COLUMNS_HEX)
		state.loc = HEX;
	else
		state.loc = ASCII;
	return 0;
}

//case '-':
//case KEY_UP:
//case 'k':
int handler__goto_up() {
	do {
		if (y > 0)
			y--;
		else
			scrollup(1);
	} while (--precount > 0);
	return 0;
}

//case '+':
//case CR:
int handler__goto_EOL() {
	if (state.loc == HEX)
		x = core.params.COLUMNS_ADDRESS;
	else
		x = core.params.COLUMNS_ADDRESS + core.params.COLUMNS_HEX;
	return 0;
}

//case 'j':
//case BVICTRL('J'):
//case BVICTRL('N'):
//case KEY_DOWN:
int handler__goto_down() {
	do {
		if ((PTR) ((state.pagepos + (y + 1) * core.params.COLUMNS_DATA)) >  maxpos)
			break;
		if (y < (core.screen.maxy - 1))
			y++;
		else
			scrolldown(1);
	} while (--precount > 0);
	return 0;
}

//case '|':
int handler__line() {
	if (precount < 1)
		return -1;
	if (state.loc == ASCII)
		x = core.params.COLUMNS_ADDRESS - 1 + core.params.COLUMNS_HEX + precount;
	else
		x = 5 + 3 * precount;
	if (x > core.params.COLUMNS_ADDRESS - 1 + core.params.COLUMNS_HEX + core.params.COLUMNS_DATA) {
		x = core.params.COLUMNS_ADDRESS - 1 + core.params.COLUMNS_HEX + core.params.COLUMNS_DATA;
		state.loc = ASCII;
	}
	return 0;
}

//case 'S':
int handler__toolwin_toggle() {
	if (!ui__ToolWin_Exist()) {
		ui__ToolWin_Show(10);
	} else {
		ui__ToolWin_Hide();
	}
	return 0;
}

//case ':':
int handler__cmdstring() {
	clearstr();
	addch(':');
	refresh();
	/* TODO: Add <Tab> autocompletion */
	getcmdstr(cmdstr, 1);
	if (strlen(cmdstr))
		docmdline(cmdstr);
	return 0;
}

//case BVICTRL('B'):
//case KEY_PPAGE:
int handler__previous_page() {
	if (mem <= (PTR) (state.pagepos - state.screen))
		state.pagepos -= state.screen;
	else
		state.pagepos = mem;
		ui__Screen_Repaint();
	return 0;
}

//case BVICTRL('D'):
int handler__scrolldown() {
	if (precount > 1)
		state.scrolly = precount;
	scrolldown(state.scrolly);
	return 0;
}

//case BVICTRL('U'):
int handler__scrollup() {
	if (precount > 1)
		state.scrolly = precount;
	scrollup(state.scrolly);
	return 0;
}

//case BVICTRL('E'):
int handler__linescroll_down() {
	if (y > 0)
		y--;
	scrolldown(1);
	return 0;
}

//case BVICTRL('F'):
//case KEY_NPAGE:
int handler__nextpage() {
	if (maxpos >= (PTR) (state.pagepos + state.screen)) {
		state.pagepos += state.screen;
		current += state.screen;
		if (current - mem >= filesize) {
			current = mem + filesize;
			setpage((PTR) (mem + filesize - 1L));
		}
		ui__Screen_Repaint();
	}
	return 0;
}

//case BVICTRL('G'):
int handler__fileinfo() {
	fileinfo(name);
	wrstat = 0;
	return 0;
}

//case BVICTRL('L'):	/*** REDRAW SCREEN ***/
int handler__screen_redraw() {
	ui__Screen_New();
	return 0;
}

//case BVICTRL('Y'):
int handler__linescroll_up() {
	if (y < core.screen.maxy)
		y++;
	scrollup(1);
	return 0;
}

//case 'A':
int handler__append_mode() {
	smsg("APPEND MODE");
	current = (PTR) (mem + filesize - 1L);
	setpage(current++);
	cur_forw(0);
	setcur();
	undosize = filesize;
	undo_count = edit('A');
	return 0;
}

//case 'B':
//case 'b':
int handler__backsearch() {
	setpage(backsearch(current, 'b'));
	return 0;
}

//case 'e':
int handler__setpage() {
	setpage(end_word(current));
	return 0;
}

//case ',':
int handler__doft1() {
	do_ft(-1, 0);
	return 0;
}

//case ';':
int handler__doft2() {
	do_ft(0, 0);
	return 0;
}

//case 'F':
//case 'f':
//case 't':
//case 'T':
int handler__doft3() {
	// TODO: split handlers for each key
	do_ft('f', 0);
	return 0;
}

//case 'G':
int handler__goto1() {
	last_motion = current;
	if (precount > -1) {
		if ((precount < P(P_OF)) || (precount - P(P_OF)) > (filesize - 1L)) {
			beep();
		} else {
			setpage((PTR)(mem + precount - P(P_OF)));
		}
	} else {
		setpage((PTR) (mem + filesize - 1L));
	}
	return 0;
}

//case 'g':
int handler__goto2() {
	off_t inaddr;
	
	last_motion = current;
	msg("Goto Hex Address: ");
	refresh();
	getcmdstr(cmdstr, 19);
	if (cmdstr[0] == '^') {
		inaddr = P(P_OF);
	} else if (cmdstr[0] == '$') {
		inaddr = filesize + P(P_OF) - 1L;
	} else {
		unsigned long ltmp;
		sscanf(cmdstr, "%lx", &ltmp);
		inaddr = (off_t) ltmp;
	}
	if (inaddr < P(P_OF))
		return -1;
	inaddr -= P(P_OF);
	if (inaddr < filesize) {
		setpage(mem + inaddr);
	} else {
		if (filesize == 0L)
			return -1;
		sprintf(string, "Max. address of current file : %06lX", (long)(filesize - 1L + P(P_OF)));
		ui__ErrorMsg(string);
	}
	return 0;
}

//case '?':
//case '/':
//case '#':
//case '\\':
int handler__search_string() {
	// TODO: split handlers for each key	
	char ch = '/';

	clearstr();
	addch(ch);
	refresh();
	if (getcmdstr(line, 1))
		return -1;
	last_motion = current;
	searching(ch, line, current, maxpos - 1, P(P_WS));
	return 0;
}

//case 'n':
//case 'N':
int handler__search_next() {
	last_motion = current;
	searching('n', "", current, maxpos - 1, P(P_WS));
	return 0;
}

//case 'm':
int handler__mark() {
	do_mark(vgetc(), current);
	return 0;
}

//case '\'':
//case '`':
int handler__goto_mark() {
	//if ((ch == '`' && state.loc == ASCII) || (ch == '\'' && state.loc == HEX))
	//	toggle();
	mark = vgetc();
	if (mark == '`' || mark == '\'') {
		setpage(last_motion);
		last_motion = current;
	} else {
		if (mark < 'a' || mark > 'z') {
			beep();
			return -1;
		} else if (markbuf[mark - 'a'] == NULL) {
			beep();
			return -1;
		}
		setpage(markbuf[mark - 'a']);
	}
	return 0;
}

//case 'D':
int handler__trunc() {
	if (precount < 1)
		precount = 1;
	sprintf(rep_buf, "%ldD", precount);
	trunc_cur();
	return 0;
}

//case 'o':
int handler__overwrite() {
	if (precount < 1)
		precount = 1;
	sprintf(rep_buf, "%ldo", precount);
	do_over(current, yanked, yank_buf);
	return 0;
}

//case 'P':
int handler__paste() {
	if (precount < 1)
		precount = 1;
	if ((undo_count = alloc_buf(yanked, &undo_buf)) == 0L)
		return -1;
	sprintf(rep_buf, "%ldP", precount);
	if (do_append(yanked, yank_buf))
		return -1;
	/* we save it not for undo but for the dot command
	 * memcpy(undo_buf, yank_buf, yanked);
	 */
	ui__Screen_Repaint();
	return 0;
}

//case 'r':
//case 'R':
int handler__redo() {
	if (filesize == 0L)
		return -1;
	if (precount < 1)
		precount = 1;
	sprintf(rep_buf, "%ld%c", precount, 'r');
	undo_count = edit('r');
	lflag++;
	return 0;
}

//case 'u':
int handler__undo() {
	do_undo();
	return 0;
}

//case 'W':
//case 'w':
int handler__wordsearch() {
	state.loc = ASCII;
	setpage(wordsearch(current, 'w'));
	return 0;
}

//case 'y':
int handler__yank() {
	long count;

	count = range('y');
	if (count > 0) {
		if ((yanked = alloc_buf(count, &yank_buf)) == 0L) {
			return -1;
		}
		memcpy(yank_buf, current, yanked);
	} else if (count < 0) {
		if ((yanked = alloc_buf(-count, &yank_buf)) == 0L) {
			return -1;
		}
		memcpy(yank_buf, current + count, yanked);
	} else {
		return -1;
	}
	/* sprintf(string, "%ld bytes yanked", labs(count));
	 * 	msg(string);
	 */
	return 0;
}

//case 'z':
int handler__doz() {
	do_z(vgetc());
	return 0;
}

//case 'Z':
int handler__exit() {
	if (vgetc() == 'Z')
		do_exit();
	else
		beep();
	return 0;
}

//case '.':
int handler__stuffin() {
	if (!strlen(rep_buf)) {
		beep();
	} else {
		stuffin(rep_buf);
	}
	return 0;
}

/* =================== END OF EVENT HANDLERS =================== */
extern int from_file;
static FILE *ffp;
static char fbuf[256];

/* reads the init file (.bvirc) */
int read_rc(fn)
char *fn;
{
	if ((ffp = fopen(fn, "r")) == NULL)
		return -1;
	from_file = 1;
	while (fgets(fbuf, 255, ffp) != NULL) {
		strtok(fbuf, "\n\r");
		docmdline(fbuf);
	}
	fclose(ffp);
	from_file = 0;
	return 0;
}

/* reads the history file (.bvihistory) */
int read_history(fn)
char *fn;
{
	if ((ffp = fopen(fn, "r")) == NULL)
		return -1;
	from_file = 1;
	while (fgets(fbuf, 255, ffp) != NULL) {
		strtok(fbuf, "\n\r");
	}
	fclose(ffp);
	from_file = 0;
	return 0;
}

void usage()
{
	fprintf(stderr, "Usage: %s [-R] [-c cmd | +cmd] [-f script]\n\
       [-b begin] [-e end] [-s size] file ...\n", progname);
	exit(1);
}

int main(argc, argv)
int argc;
char *argv[];
{
	int ch;
	int n = 1;
	int script = -1;
	int i;
	char *poi;

	keys__Init();

#ifdef HAVE_LOCALE_H
	setlocale(LC_ALL, "");
#endif
#ifdef HAVE_LUA_H
	bvi_lua_init();
#endif

	for (i = 0; i < MARK_COUNT; i++)
		markers[i].address = 0;

	poi = strrchr(argv[0], DELIM);

	if (poi)
		strncpy(progname, ++poi, 7);
	else
		strncpy(progname, argv[0], 7);
	strtok(progname, ".");

	if (!strcasecmp(progname, "bview")) {
		params[P_RO].flags |= P_CHANGED;
		P(P_RO) = TRUE;
	} else if (!strcasecmp(progname, "bvedit")) {
		/* This should be the beginners version */
	}

	while (n < argc) {
		switch (argv[n][0]) {
		case '-':
			if (argv[n][1] == 'R') {
				params[P_RO].flags |= P_CHANGED;
				P(P_RO) = TRUE;
			} else if (argv[n][1] == 'c') {
				if (argv[n + 1] == NULL) {
					usage();
				} else {
					strcpy(cmdstr, argv[++n]);
				}
			} else if (argv[n][1] == 'f') {
				if (argv[n + 1] == NULL
				    || argv[n + 1][0] == '-') {
					usage();
				} else {
					script = ++n;
				}
			} else if (argv[n][1] == 'b') {
				if (argv[n + 1] == NULL
				    || argv[n + 1][0] == '-') {
					usage();
				} else {
					block_begin = calc_size(argv[++n]);
					block_flag |= 1;
				}
			} else if (argv[n][1] == 'e') {
				if (argv[n + 1] == NULL
				    || argv[n + 1][0] == '-') {
					usage();
				} else {
					block_end = calc_size(argv[++n]);
					block_flag |= 2;
				}
			} else if (argv[n][1] == 's') {
				if (argv[n + 1] == NULL
				    || argv[n + 1][0] == '-') {
					usage();
				} else {
					block_size = calc_size(argv[++n]);
					block_flag |= 4;
				}
			} else if (argv[n][1] == 'w') {
				if (argv[n][2] == '\0') {
					usage();
				} else {
					params[P_LI].flags |= P_CHANGED;
					P(P_LI) = atoi(argv[n] + 2);
				}
			} else
				usage();
			n++;
			break;
		case '+':	/* +cmd */
			if (argv[n][1] == '\0') {
				strcpy(cmdstr, "$");
			} else {
				strcpy(cmdstr, &argv[n][1]);
			}
			n++;
			break;
		default:	/* must be a file name */
			name = strdup(argv[n]);
			files = &(argv[n]);
			numfiles = argc - n;
			n = argc;
			break;
		}
	}
	switch (block_flag) {
	case 2:
		block_begin = 0;
	case 1 | 2:
		block_size = block_end - block_begin + 1;
		break;
	case 4:
		block_begin = 0;
	case 1 | 4:
		block_end = block_begin + block_size - 1;
		break;
	case 2 | 4:
		block_begin = block_end + 1 - block_size;
		break;
	case 1 | 2 | 4:
		if (block_end - block_begin != block_size + 1) {
			fprintf(stderr, "Ambigous block data\n");
			exit(1);
		}
		break;
	}

	if (block_flag && !numfiles) {
		fprintf(stderr, "Cannot read a range of a nonexisting file\n");
		exit(1);
	}
	if (numfiles > 1)
		fprintf(stderr, "%d files to edit\n", numfiles);
	curfile = 0;
	
	/* ====== UI initialization ====== */
	ui__Init();

	signal(SIGINT, SIG_IGN);
	filesize = load(name);

	bvi_init(argv[0]);
	params[P_TT].svalue = terminal;
	if (block_flag && (P(P_MM) == TRUE)) {
		P(P_MM) = FALSE;
		params[P_TT].flags |= P_CHANGED;
	}
	if (script > -1)
		read_rc(argv[script]);
	if (*cmdstr != '\0')
		docmdline(cmdstr);

	/* main loop */
	do {
		setjmp(env);
		current = (PTR) (state.pagepos + y * core.params.COLUMNS_DATA + xpos());
		if (wrstat)
			statpos();
		wrstat = 1;
		setcur();
		ch = vgetc();
		while (ch >= '0' && ch <= '9') {
			numarr[arrnum++] = ch;
			ch = vgetc();
		}
		numarr[arrnum] = '\0';
		if (arrnum != 0)
			precount = strtol(numarr, (char **)NULL, 10);
		else
			precount = -1;
		lflag = arrnum = 0;
	
		/* TODO: move all checks in keys.c */

		keys__Key_Pressed(ch);
		/*
		if (pkey.handler_type == BVI_HANDLER_INTERNAL) {
			if (pkey.handler.func != NULL)
				(*(pkey.handler.func))();
		} else if (pkey.handler_type == BVI_HANDLER_LUA) {
			if (pkey.handler.lua_cmd != NULL)
				bvi_run_lua_string(pkey.handler.lua_cmd);
		}
		*/

/*
		default:
			if P(P_MM) {
				if (precount < 1)
					precount = 1;
				switch (ch) {
				case 'I':
					sprintf(rep_buf, "%ldI", precount);
					current = mem;
					setpage(mem);
					ui__Screen_Repaint();
					undo_count = edit('i');
					lflag++;
					break; // undo does not work correctly !!!
				case 's':
					sprintf(rep_buf, "%lds", precount);
					if (do_delete
					    ((off_t) precount, current))
						break;
					precount = 1;
					undo_count = edit('i');
					lflag++;
					break;
				case 'a':
					if (cur_forw(1))
						break;
					current++;
				case 'i':
					sprintf(rep_buf, "%ld%c", precount, ch);
					undo_count = edit(ch);
					lflag++;
					break;
				case 'p':
					sprintf(rep_buf, "%ldp", precount);
					do_put(current, yanked, yank_buf);
					break;
				case 'c':
				case 'd':
					count = range(ch);
					if (count > 0)
						do_delete((off_t) count,
							  current);
					else if (count < 0)
						do_back(-(off_t) count,
							current);
					if (ch == 'c') {
						precount = 1;
						undo_count = edit('i');
						lflag++;
//
//					} else if (count) {
//						sprintf(string, "%ld bytes deleted", labs(count));
//						msg(string);
//
					}
					break;
				case 'x':
					sprintf(rep_buf, "%ldx", precount);
					do_delete((off_t) precount, current);
					break;
				case 'X':
					sprintf(rep_buf, "%ldX", precount);
					do_back((off_t) precount, current);
					break;
				default:
					flushinp();
					beep();
				}
			} else {
				switch (ch) {
				case 'x':
					if (precount < 1)
						precount = 1;
					if ((off_t) precount + current ==
					    maxpos) {
						sprintf(rep_buf, "%ldx",
							precount);
						do_delete((off_t) precount,
							  current);
					} else {
						movebyte();
						flushinp();
						beep();
					}
					break;
				case 'd':
				case 'i':
				case 'I':
				case 'p':
				case 'X':
					movebyte();
				default:
					flushinp();
					beep();
				}
			}
		}
*/
		if (lflag)
			ui__lineout();
	} while (1);
}

off_t calc_size(arg)
char *arg;
{
	long val;
	char *poi;

	if (*arg == '0') {
		val = strtol(arg, &poi, 16);
	} else {
		val = strtol(arg, &poi, 10);
	}
	switch (*poi) {
	case 'k':
	case 'K':
		val *= 1024;
		break;
	case 'm':
	case 'M':
		val *= 1048576;
		break;
	case '\0':
		break;
	default:
		usage();
	}
	return (off_t) val;
}

void trunc_cur()
{
	undosize = filesize;
	undo_count = (off_t) (maxpos - current);
	undo_start = current;
	filesize = state.pagepos - mem + y * core.params.COLUMNS_DATA + xpos();
	maxpos = (PTR) (mem + filesize);
	if (filesize == 0L) {
		ui__ErrorMsg(nobytes);
	} else
		cur_back();
	edits = U_TRUNC;
	ui__Screen_Repaint();
}

int do_append(count, buf)
int count;
char *buf;
{
	if (filesize + count > memsize) {
		if (enlarge(count + 100L))
			return 1;
	}
	memcpy(mem + filesize, buf, count);
	undo_start = mem + filesize - 1L;
	setpage(undo_start + count);
	edits = U_APPEND;
	undosize = filesize;
	filesize += count;
	maxpos += count;
	return 0;
}

void do_tilde(count)
off_t count;
{
	if (filesize == 0L)
		return;
	undo_start = current;
	if (current + count > maxpos) {
		beep();
		return;
	}
	if ((undo_count = alloc_buf(count, &undo_buf)) == 0L)
		return;
	memcpy(undo_buf, current, undo_count);
	while (count--) {
		if (isupper((int)(*current)))
			*current = tolower((int)(*current));
		else if (islower((int)(*current)))
			*current = toupper((int)(*current));
		current++;
		cur_forw(0);
	}
	edits = U_TILDE;
	setcur();
}

void do_undo()
{
	off_t n, tempsize;
	char temp;
	PTR set_cursor;
	PTR s;
	PTR d;

	if (undo_count == 0L) {
		ui__ErrorMsg("Nothing to undo");
		return;
	}
	set_cursor = undo_start;
	switch (edits) {
	case U_EDIT:
	case U_TILDE:
		n = undo_count;
		s = undo_buf;
		d = undo_start;
		while (n--) {
			temp = *d;
			*d = *s;
			*s = temp;
			s++;
			d++;
		}
		break;
	case U_APPEND:
	case U_TRUNC:
		tempsize = filesize;
		filesize = undosize;
		undosize = tempsize;
		maxpos = (PTR) (mem + filesize);
		if (filesize)
			set_cursor = maxpos - 1L;
		else
			set_cursor = maxpos;
		break;
	case U_INSERT:
		filesize -= undo_count;
		maxpos -= undo_count;
		memcpy(undo_buf, undo_start, undo_count);
		memmove(undo_start, undo_start + undo_count,
			maxpos - undo_start);
		edits = U_DELETE;
		break;
	case U_BACK:
	case U_DELETE:
		filesize += undo_count;
		maxpos += undo_count;
		memmove(undo_start + undo_count, undo_start,
			maxpos - undo_start);
		memcpy(undo_start, undo_buf, undo_count);
		edits = U_INSERT;
		break;
	}
	setpage(set_cursor);
	if (edits == U_TRUNC && undosize > filesize)
		cur_back();
	ui__Screen_Repaint();
}

void do_over(loc, n, buf)
PTR loc;
off_t n;
PTR buf;
{
	if (n < 1L) {
		ui__ErrorMsg(nobytes);
		return;
	}
	if (loc + n > maxpos) {
		beep();
		return;
	}
	if ((undo_count = alloc_buf(n, &undo_buf)) == 0L)
		return;
	undo_start = loc;
	memcpy(undo_buf, loc, n);
	memcpy(loc, buf, n);
	edits = U_EDIT;
	setpage(loc + n - 1);
	ui__Screen_Repaint();
}

void do_put(loc, n, buf)
PTR loc;
off_t n;
PTR buf;
{
	if (n < 1L) {
		ui__ErrorMsg(nobytes);
		return;
	}
	if (loc > maxpos) {
		beep();
		return;
	}
	if (filesize + n > memsize) {
		if (enlarge(n + 1024))
			return;
	}
	if ((undo_count = alloc_buf(n, &undo_buf)) == 0L)
		return;
	undo_start = loc + 1;
	edits = U_INSERT;
	filesize += n;
	maxpos += n;
	memmove(undo_start + n, undo_start, maxpos - loc);
	memcpy(undo_start, buf, n);
	setpage(loc + n);
	ui__Screen_Repaint();
}

/* argument sig not used, because only SIGINT will be catched */
void jmpproc(sig)
int sig;
{
	if (P(P_EB))
		beep();
	ui__Screen_Repaint();
	clearstr();
	signal(SIGINT, SIG_IGN);
	longjmp(env, 0);
}

off_t range(ch)
int ch;
{
	int ch1;
	long count;

	ch1 = vgetc();
	while (ch1 >= '0' && ch1 <= '9') {
		numarr[arrnum++] = ch1;
		ch1 = vgetc();
	}
	numarr[arrnum] = '\0';
	if (arrnum != 0)
		count = strtol(numarr, (char **)NULL, 10);
	else
		count = 1;
	arrnum = 0;
	sprintf(rep_buf, "%ld%c%s%c", precount, ch, numarr, ch1);
	switch (ch1) {
	case '/':	/**** Search String ****/
	case '\\':
		strcat(rep_buf, "\n");
		clearstr();
		addch(ch1);
		refresh();
		if (getcmdstr(line, 1))
			break;
		end_addr = searching(ch1, line, current, maxpos - 1, FALSE);
		if (!end_addr) {
			beep();
			return 0;
		}
		return (end_addr - current);
	case '?':
	case '#':
		strcat(rep_buf, "\n");
		clearstr();
		addch(ch1);
		refresh();
		if (getcmdstr(line, 1))
			break;
		start_addr = searching(ch1, line, current, maxpos - 1, FALSE);
		if (!start_addr) {
			beep();
			return 0;
		}
		return (start_addr - current);
	case 'f':
	case 't':
		precount = count;
		end_addr = do_ft(ch1, 1);
		if (!end_addr) {
			beep();
			return 0;
		}
		return (end_addr + 1 - current);
	case 'F':
	case 'T':
		precount = count;
		start_addr = do_ft(ch1, 1);
		if (!start_addr) {
			beep();
			return 0;
		}
		return (start_addr - current);
	case '$':
		trunc_cur();
		return 0;
	case 'G':
		if (count == -1) {
			trunc_cur();
			return 0;
		} else if ((count < P(P_OF)) || (count
						 - (off_t) P(P_OF)) >
			   (filesize - 1L)) {
			beep();
			return 0;
		} else {
			if (mem + count < current) {
				return (mem + count - current);
			} else {
				return (count - (current - mem));
			}
		}
	case ' ':
		return precount;
	case '`':
	case '\'':
		mark = vgetc();
		if (mark < 'a' || mark > 'z') {
			beep();
			return 0;
		}
		end_addr = markbuf[mark - 'a'];
		if (end_addr == NULL) {
			beep();
			return 0;
		}
		if (end_addr < current) {
			return (end_addr - current);
		} else {
			return (end_addr - current + 1);
		}
	}
	beep();
	return 0;
}
