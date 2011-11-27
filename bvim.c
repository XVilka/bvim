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

#include <sys/types.h>

#include "bvim.h"
#include "blocks.h"
#include "set.h"
#include "ui.h"
#include "keys.h"
#include "commands.h"
#include "messages.h"

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

char *copyright = 
"\nCopyright (C) 1996-2004 by Gerhard Buergmann\n\
Copyright (C) 2011 by Anton Kochkov";

jmp_buf env;			/* context for `longjmp' function   */

core_t core;
state_t state;
api_t api;

int x, xx, y;
int status;
off_t size;

PTR curpos;
PTR maxpos;
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

PTR last_motion;
PTR current_start;
PTR undo_start;
off_t undo_count;
off_t yanked = 0L;
char *yank_buf = NULL;
char *undo_buf = NULL;
PTR markbuf[26];

char addr_form[15];

static char progname[8];
static char line[MAXCMD];
static int mark;
static int wrstat = 1;

extern struct MARKERS_ markers[MARK_COUNT];

/* ======================= EVENT HANDLERS ====================== */

// TODO: eliminate global variables:
int lflag;

// case '^':
int handler__goto_HEX()
{
	x = core.params.COLUMNS_ADDRESS;
	state.loc = HEX;
	return 0;
}

/* case '0':
 * x = COLUMNS_ADDRESS + COLUMNS_HEX;
 * state.loc = ASCII;
 */

//case '$':
int handler__goto_ASCII()
{
	x = core.params.COLUMNS_ADDRESS - 1 + core.params.COLUMNS_HEX +
	    core.params.COLUMNS_DATA;
	state.loc = ASCII;
	return 0;
}

//case '\t':
int handler__toggle()
{
	toggle();
	return 0;
}

//case '~':
int handler__tilda()
{
	if (precount < 1)
		precount = 1;
	sprintf(rep_buf, "%ld~", precount);
	do_tilde(precount);
	lflag++;
	return 0;
}

//case KEY_HOME: /* go to the HOME */
//case 'H':
int handler__goto_home()
{
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
int handler__M()
{
	y = core.screen.maxy / 2;
	if ((PTR) (state.pagepos + state.screen) > maxpos)
		y = (int)(maxpos -
			  state.pagepos) / core.params.COLUMNS_DATA / 2;
	if (state.loc == HEX)
		x = core.params.COLUMNS_ADDRESS;
	else
		x = core.params.COLUMNS_ADDRESS + core.params.COLUMNS_HEX;
	return 0;
}

//case KEY_LL:
//case 'L':
int handler__L()
{
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
int handler__goto_left()
{
	do {
		if (x > (core.params.COLUMNS_ADDRESS + 2)
		    && x <
		    (core.params.COLUMNS_HEX + core.params.COLUMNS_ADDRESS + 1))
			x -= 3;
		else if (x >
			 (core.params.COLUMNS_HEX +
			  core.params.COLUMNS_ADDRESS - 2))
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
int handler__goto_right()
{
	do {
		/*
		   if (x < (COLUMNS_HEX + 6))  x += 3;
		 */
		if (x <
		    (core.params.COLUMNS_HEX + core.params.COLUMNS_ADDRESS - 2))
			x += 3;
		else if (x > (core.params.COLUMNS_HEX + 3)
			 && x <
			 (core.params.COLUMNS_HEX +
			  core.params.COLUMNS_ADDRESS - 1 +
			  core.params.COLUMNS_DATA))
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
int handler__goto_up()
{
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
int handler__goto_EOL()
{
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
int handler__goto_down()
{
	do {
		if ((PTR) ((state.pagepos + (y + 1) * core.params.COLUMNS_DATA))
		    > maxpos)
			break;
		if (y < (core.screen.maxy - 1))
			y++;
		else
			scrolldown(1);
	} while (--precount > 0);
	return 0;
}

//case '|':
int handler__line()
{
	if (precount < 1)
		return -1;
	if (state.loc == ASCII)
		x = core.params.COLUMNS_ADDRESS - 1 + core.params.COLUMNS_HEX +
		    precount;
	else
		x = 5 + 3 * precount;
	if (x >
	    core.params.COLUMNS_ADDRESS - 1 + core.params.COLUMNS_HEX +
	    core.params.COLUMNS_DATA) {
		x = core.params.COLUMNS_ADDRESS - 1 + core.params.COLUMNS_HEX +
		    core.params.COLUMNS_DATA;
		state.loc = ASCII;
	}
	return 0;
}

//case 'S':
int handler__toolwin_toggle()
{
	if (!ui__ToolWin_Exist()) {
		ui__ToolWin_Show(10);
	} else {
		ui__ToolWin_Hide();
	}
	return 0;
}

//case ':':
int handler__cmdstring()
{
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
int handler__previous_page()
{
	if (core.editor.mem <= (PTR) (state.pagepos - state.screen))
		state.pagepos -= state.screen;
	else
		state.pagepos = core.editor.mem;
	ui__Screen_Repaint();
	return 0;
}

//case BVICTRL('D'):
int handler__scrolldown()
{
	if (precount > 1)
		state.scrolly = precount;
	scrolldown(state.scrolly);
	return 0;
}

//case BVICTRL('U'):
int handler__scrollup()
{
	if (precount > 1)
		state.scrolly = precount;
	scrollup(state.scrolly);
	return 0;
}

//case BVICTRL('E'):
int handler__linescroll_down()
{
	if (y > 0)
		y--;
	scrolldown(1);
	return 0;
}

//case BVICTRL('F'):
//case KEY_NPAGE:
int handler__nextpage()
{
	if (maxpos >= (PTR) (state.pagepos + state.screen)) {
		state.pagepos += state.screen;
		state.current += state.screen;
		if (state.current - core.editor.mem >= filesize) {
			state.current = core.editor.mem + filesize;
			setpage((PTR) (core.editor.mem + filesize - 1L));
		}
		ui__Screen_Repaint();
	}
	return 0;
}

// "Ctrl-G"
int handler__fileinfo()
{
	fileinfo(name);
	wrstat = 0;
	return 0;
}

// Redraw screen
int handler__screen_redraw()
{
	ui__Screen_New();
	return 0;
}

// "Ctrl-Y" key
int handler__linescroll_up()
{
	if (y < core.screen.maxy)
		y++;
	scrollup(1);
	return 0;
}

// "Ctrl-R" key - Open Lua REPL window
int handler__luarepl()
{
	ui__REPL_Main();
	return 0;
}

// "Ctrl-S" key - Toggle selection with help of cursor
int handler__toggle_selection()
{
	if (state.toggle_selection == 0) {
		state.toggle_selection = 1;
	} else {
		state.toggle_selection = 0;
	}
	return 0;
}

// "A" key
int handler__append_mode()
{
	smsg("APPEND MODE");
	state.current = (PTR) (core.editor.mem + filesize - 1L);
	setpage(state.current++);
	cur_forw(0);
	setcur();
	undosize = filesize;
	undo_count = edit('A');
	return 0;
}

// "B" or "b" keys
int handler__backsearch()
{
	setpage(backsearch(state.current, 'b'));
	return 0;
}

// "e" key
int handler__setpage()
{
	setpage(end_word(state.current));
	return 0;
}

// "," key
int handler__doft1()
{
	do_ft(-1, 0);
	return 0;
}

// ";" key
int handler__doft2()
{
	do_ft(0, 0);
	return 0;
}

//case 'F':
int handler__doft3_F()
{
	do_ft('F', 0);
	return 0;
}

// "f" key
int handler__doft3_f()
{
	do_ft('f', 0);
	return 0;
}

// "t" key
int handler__doft3_t()
{
	do_ft('t', 0);
	return 0;
}

// "T" key
int handler__doft3_T()
{
	do_ft('T', 0);
	return 0;
}

//case 'G':
int handler__goto1()
{
	last_motion = state.current;
	if (precount > -1) {
		if ((precount < P(P_OF))
		    || (precount - P(P_OF)) > (filesize - 1L)) {
			beep();
		} else {
			setpage((PTR) (core.editor.mem + precount - P(P_OF)));
		}
	} else {
		setpage((PTR) (core.editor.mem + filesize - 1L));
	}
	return 0;
}

//case 'g':
int handler__goto2()
{
	off_t inaddr;

	last_motion = state.current;
	ui__StatusMsg("Goto Hex Address: ");
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
		setpage(core.editor.mem + inaddr);
	} else {
		if (filesize == 0L)
			return -1;
		sprintf(string, "Max. address of current file : %06lX",
			(long)(filesize - 1L + P(P_OF)));
		ui__ErrorMsg(string);
	}
	return 0;
}

//case '?':
int handler__search_string1()
{
	char ch = '?';

	clearstr();
	addch(ch);
	refresh();
	if (getcmdstr(line, 1))
		return -1;
	last_motion = state.current;
	searching(ch, line, state.current, maxpos - 1, P(P_WS));
	return 0;
}

// case '/':
int handler__search_string2()
{
	char ch = '/';

	clearstr();
	addch(ch);
	refresh();
	if (getcmdstr(line, 1))
		return -1;
	last_motion = state.current;
	searching(ch, line, state.current, maxpos - 1, P(P_WS));
	return 0;
}

// case '#':
int handler__search_string3()
{
	char ch = '#';

	clearstr();
	addch(ch);
	refresh();
	if (getcmdstr(line, 1))
		return -1;
	last_motion = state.current;
	searching(ch, line, state.current, maxpos - 1, P(P_WS));
	return 0;
}

// case '\\':
int handler__search_string4()
{
	char ch = '\\';

	clearstr();
	addch(ch);
	refresh();
	if (getcmdstr(line, 1))
		return -1;
	last_motion = state.current;
	searching(ch, line, state.current, maxpos - 1, P(P_WS));
	return 0;
}


//case 'n':
//case 'N':
int handler__search_next()
{
	last_motion = state.current;
	searching('n', "", state.current, maxpos - 1, P(P_WS));
	return 0;
}

//case 'm':
int handler__mark()
{
	do_mark(vgetc(), state.current);
	return 0;
}

//case '\'':
//case '`':
int handler__goto_mark()
{
	//if ((ch == '`' && state.loc == ASCII) || (ch == '\'' && state.loc == HEX))
	//      toggle();
	mark = vgetc();
	if (mark == '`' || mark == '\'') {
		setpage(last_motion);
		last_motion = state.current;
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
int handler__trunc()
{
	if (precount < 1)
		precount = 1;
	sprintf(rep_buf, "%ldD", precount);
	trunc_cur();
	return 0;
}

//case 'o':
int handler__overwrite()
{
	if (precount < 1)
		precount = 1;
	sprintf(rep_buf, "%ldo", precount);
	do_over(state.current, yanked, yank_buf);
	return 0;
}

/* Paste previously yanked buffer/byte */
/* "P" key */
int handler__paste()
{
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

/* Redo handler */
/* "r" and "R" keys */
int handler__redo()
{
	if (filesize == 0L)
		return -1;
	if (precount < 1)
		precount = 1;
	sprintf(rep_buf, "%ld%c", precount, 'r');
	undo_count = edit('r');
	lflag++;
	return 0;
}

/* Undo handler */
/* "u" and "U" keys */
int handler__undo()
{
	do_undo();
	return 0;
}

/* VISUAL mode selection/toggle handler */
/* "v" key */
int handler__visual()
{
	struct block_item tmpblk;

	if ((state.mode != BVI_MODE_VISUAL ) & (state.mode != BVI_MODE_REPL)) {
		state.mode = BVI_MODE_VISUAL;
		// start selection
		state.selection.start = get_cursor_position();
		bvim_info(state.mode, "Started selection from %ld", state.selection.start);
		return 0;
	}
	if (state.mode == BVI_MODE_VISUAL) {
		state.mode = BVI_MODE_EDIT;
		// end selection
		state.selection.end = get_cursor_position();
		bvim_info(state.mode, "Selected block [%ld, %ld]", state.selection.start, state.selection.end);
		tmpblk.pos_start = state.selection.start;
		tmpblk.pos_end = state.selection.end;
		tmpblk.palette = 1; // TODO: something more nice
		tmpblk.hl_toggle = 1;
		tmpblk.id = BVI_VISUAL_SELECTION_ID; // TODO: do something!
		blocks__Add(tmpblk);
		ui__BlockHighlightAdd(&tmpblk);
		ui__Screen_Repaint();
		return 0;
	}
	return 0;
}

//case 'W':
//case 'w':
int handler__wordsearch()
{
	state.loc = ASCII;
	setpage(wordsearch(state.current, 'w'));
	return 0;
}

/* Yanking buffer or byte */
/* "y" key */
int handler__yank()
{
	long count;

	count = range('y');
	if (count > 0) {
		if ((yanked = alloc_buf(count, &yank_buf)) == 0L) {
			return -1;
		}
		memcpy(yank_buf, state.current, yanked);
	} else if (count < 0) {
		if ((yanked = alloc_buf(-count, &yank_buf)) == 0L) {
			return -1;
		}
		memcpy(yank_buf, state.current + count, yanked);
	} else {
		return -1;
	}
	/* sprintf(string, "%ld bytes yanked", labs(count));
	 *      msg(string);
	 */
	return 0;
}

//case 'z':
int handler__doz()
{
	do_z(vgetc());
	return 0;
}

/* Exit program */
/* "Z" key */
int handler__exit()
{
	if (vgetc() == 'Z')
		do_exit();
	else
		beep();
	return 0;
}

//case '.':
int handler__stuffin()
{
	if (!strlen(rep_buf)) {
		beep();
	} else {
		stuffin(rep_buf);
	}
	return 0;
}

/* ---------------------- Disabled handlers ------------------------ */

//			if P(P_MM) {

//				if (precount < 1)
//					precount = 1;
//
//				case 'I':
int handler__insert()
{
	sprintf(rep_buf, "%ldI", precount);
	state.current = core.editor.mem;
	setpage(core.editor.mem);
	ui__Screen_Repaint();
	undo_count = edit('i');
	lflag++;
	return 0;
}	
// undo does not work correctly !!!

//case 's':
int handler__s()
{
	sprintf(rep_buf, "%lds", precount);
	if (do_delete((off_t) precount, state.current))
		return 0;
	precount = 1;
	undo_count = edit('i');
	lflag++;
	return 0;
}

//case 'a':
int handler__append2()
{
	if (cur_forw(1))
		return 0;
	state.current++;
	return 0;
}

//case 'i':
int handler__insert2()
{
	char ch = 'i';

	sprintf(rep_buf, "%ld%c", precount, ch);
	undo_count = edit(ch);
	lflag++;
	return 0;
}

//case 'p':
int handler__paste2()
{
	sprintf(rep_buf, "%ldp", precount);
	do_put(state.current, yanked, yank_buf);
	return 0;
}	

//case 'c':
//case 'd':
int handler__c_or_d()
{
	char ch = 'c';
	int count = range(ch);

	if (count > 0)
		do_delete((off_t) count, state.current);
	else if (count < 0)
		do_back(-(off_t) count, state.current);
	if (ch == 'c') {
		precount = 1;
		undo_count = edit('i');
		lflag++;
	//
	//} else if (count) {
	//	sprintf(string, "%ld bytes deleted", labs(count));
	//	msg(string);
	//
	}
	return 0;
}

//case 'x':
int handler__x()
{
	sprintf(rep_buf, "%ldx", precount);
	do_delete((off_t) precount, state.current);
	return 0;
}

//case 'X':
int handler__x2()
{
	sprintf(rep_buf, "%ldX", precount);
	do_back((off_t) precount, state.current);
	return 0;
}

/* =================== END OF EVENT HANDLERS =================== */

extern int from_file;
static FILE *ffp;
static char fbuf[256];

/* reads the init file (.bvimrc) */
int read_rc(char* fn)
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

/* reads the history file (.bvimhistory) */
int read_history(char* fn)
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

/* Record commands history */
void record_cmd(char* cmdline)
{
// TODO: record plain commands in one file, block commands content - in files with unique name
// this means, that it will be full-featured history with infinite undo/redo
	char rcpath[256];
	FILE *hifile = NULL;

	rcpath[0] = '\0';
	strcat(rcpath, getenv("HOME"));
	strcat(rcpath, "/.bvimhistory");
	hifile = fopen(rcpath, "a");
	if (!strcmp(cmdline, "q")) {
		fprintf(hifile, "### Session quit ###\n");
	} else {
		fprintf(hifile, "%s\n", cmdline);
	}
	fclose(hifile);
}

/* ---------------------------------------------------------------
 *  Debugging and errors handling
 * ---------------------------------------------------------------
 */

void bvim_error(int mode, char* fmt, ...)
{
	char msg[1024];
	va_list ap;

	msg[0] = '\0';

	va_start(ap, fmt);
	vsprintf(msg, fmt, ap);
	va_end(ap);

	ui__ErrorMsg(msg);
}

void bvim_info(int mode, char* fmt, ...)
{
	char msg[1024];
	va_list ap;

	msg[0] = '\0';

	va_start(ap, fmt);
	vsprintf(msg, fmt, ap);
	va_end(ap);

	ui__StatusMsg(msg);
}

void bvim_debug(int mode, char* fmt, ...)
{
	char msg[1024];
	va_list ap;

	msg[0] = '\0';

	va_start(ap, fmt);
	vsprintf(msg, fmt, ap);
	va_end(ap);

	fprintf(stderr, "%s", msg);

}

/* ---------------------------------------------------------------
 * Main function
 * ---------------------------------------------------------------
 */

void usage()
{
	fprintf(stderr, "Usage: %s [-R] [-c cmd | +cmd] [-f script]\n\
       [-b begin] [-e end] [-s size] file ...\n", progname);
	exit(1);
}

int main(int argc, char* argv[])
{
	int ch;
	int n = 1;
	int script = -1;
	int i;
	char *poi;

	/* Hotkeys and keymap initialization */
	keys__Init();

#ifdef HAVE_LOCALE_H
	setlocale(LC_ALL, "");
#endif

#ifdef HAVE_LUA_H
	/* Lua subsystem initialization */
	bvim_lua_init();
#endif

	/* Commands parser initialization */
	commands__Init();

	/* Plugins infrastructure initialization */
	plugins__Init();

	for (i = 0; i < MARK_COUNT; i++)
		markers[i].address = 0;

	state.mode = BVI_MODE_EDIT; // default mode from start

	poi = strrchr(argv[0], DELIM);

	if (poi)
		strncpy(progname, ++poi, 7);
	else
		strncpy(progname, argv[0], 7);
	strtok(progname, ".");

	if (!strcasecmp(progname, "bvimew")) {
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

	/* UI initialization */
	ui__Init();

	signal(SIGINT, SIG_IGN);
	filesize = load(name);

	bvim_init(argv[0]);
	params[P_TT].svalue = terminal;
	if (block_flag && (P(P_MM) == TRUE)) {
		P(P_MM) = FALSE;
		params[P_TT].flags |= P_CHANGED;
	}
	if (script > -1)
		read_rc(argv[script]);
	if (*cmdstr != '\0')
		docmdline(cmdstr);

	// Main loop
	do {
		setjmp(env);
		state.current = (PTR) (state.pagepos + y * core.params.COLUMNS_DATA + xpos());
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

		keys__Key_Pressed(ch);
		
		// TODO: There are operations disabled by default
/*
		default:
			if P(P_MM) {
				if (precount < 1)
					precount = 1;
				switch (ch) {
				case 'I':
					sprintf(rep_buf, "%ldI", precount);
					state.current = core.editor.mem;
					setpage(core.editor.mem);
					ui__Screen_Repaint();
					undo_count = edit('i');
					lflag++;
					break; // undo does not work correctly !!!
				case 's':
					sprintf(rep_buf, "%lds", precount);
					if (do_delete
					    ((off_t) precount, state.current))
						break;
					precount = 1;
					undo_count = edit('i');
					lflag++;
					break;
				case 'a':
					if (cur_forw(1))
						break;
					state.current++;
				case 'i':
					sprintf(rep_buf, "%ld%c", precount, ch);
					undo_count = edit(ch);
					lflag++;
					break;
				case 'p':
					sprintf(rep_buf, "%ldp", precount);
					do_put(state.current, yanked, yank_buf);
					break;
				case 'c':
				case 'd':
					count = range(ch);
					if (count > 0)
						do_delete((off_t) count,
							  state.current);
					else if (count < 0)
						do_back(-(off_t) count,
							state.current);
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
					do_delete((off_t) precount, state.current);
					break;
				case 'X':
					sprintf(rep_buf, "%ldX", precount);
					do_back((off_t) precount, state.current);
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
					if ((off_t) precount + state.current ==
					    maxpos) {
						sprintf(rep_buf, "%ldx",
							precount);
						do_delete((off_t) precount,
							  state.current);
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

off_t calc_size(char* arg)
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
	undo_count = (off_t) (maxpos - state.current);
	undo_start = state.current;
	filesize = state.pagepos - core.editor.mem + y * core.params.COLUMNS_DATA + xpos();
	maxpos = (PTR) (core.editor.mem + filesize);
	if (filesize == 0L) {
		bvim_error(state.mode, BVI_ERROR_NOBYTES);
	} else
		cur_back();
	edits = U_TRUNC;
	ui__Screen_Repaint();
}

int do_append(int count, char* buf)
{
	if (filesize + count > memsize) {
		if (enlarge(count + 100L))
			return 1;
	}
	memcpy(core.editor.mem + filesize, buf, count);
	undo_start = core.editor.mem + filesize - 1L;
	setpage(undo_start + count);
	edits = U_APPEND;
	undosize = filesize;
	filesize += count;
	maxpos += count;
	return 0;
}

void do_tilde(off_t count)
{
	if (filesize == 0L)
		return;
	undo_start = state.current;
	if (state.current + count > maxpos) {
		beep();
		return;
	}
	if ((undo_count = alloc_buf(count, &undo_buf)) == 0L)
		return;
	memcpy(undo_buf, state.current, undo_count);
	while (count--) {
		if (isupper((int)(*(state.current))))
			*(state.current) = tolower((int)(*(state.current)));
		else if (islower((int)(*(state.current))))
			*(state.current) = toupper((int)(*(state.current)));
		state.current++;
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
		maxpos = (PTR) (core.editor.mem + filesize);
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

void do_over(PTR loc, off_t n, PTR buf)
{
	if (n < 1L) {
		bvim_error(state.mode, BVI_ERROR_NOBYTES);
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

void do_put(PTR loc, off_t n, PTR buf)
{
	if (n < 1L) {
		bvim_error(state.mode, BVI_ERROR_NOBYTES);
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
void jmpproc(int sig)
{
	if (P(P_EB))
		beep();
	ui__Screen_Repaint();
	clearstr();
	signal(SIGINT, SIG_IGN);
	longjmp(env, 0);
}

off_t range(int ch)
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
		end_addr = searching(ch1, line, state.current, maxpos - 1, FALSE);
		if (!end_addr) {
			beep();
			return 0;
		}
		return (end_addr - state.current);
	case '?':
	case '#':
		strcat(rep_buf, "\n");
		clearstr();
		addch(ch1);
		refresh();
		if (getcmdstr(line, 1))
			break;
		start_addr = searching(ch1, line, state.current, maxpos - 1, FALSE);
		if (!start_addr) {
			beep();
			return 0;
		}
		return (start_addr - state.current);
	case 'f':
	case 't':
		precount = count;
		end_addr = do_ft(ch1, 1);
		if (!end_addr) {
			beep();
			return 0;
		}
		return (end_addr + 1 - state.current);
	case 'F':
	case 'T':
		precount = count;
		start_addr = do_ft(ch1, 1);
		if (!start_addr) {
			beep();
			return 0;
		}
		return (start_addr - state.current);
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
			if (core.editor.mem + count < state.current) {
				return (core.editor.mem + count - state.current);
			} else {
				return (count - (state.current - core.editor.mem));
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
		if (end_addr < state.current) {
			return (end_addr - state.current);
		} else {
			return (end_addr - state.current + 1);
		}
	}
	beep();
	return 0;
}

// QUIT from the BVI
void quit()
{
	/* Destroy all structures/lists */
	keys__Destroy();
	commands__Destroy();
	blocks__Destroy();
	plugins__Destroy();

	/* Destroy all curses stuff, restore terminal state */
	move(core.screen.maxy, 0);
	ui__Destroy();
	printf("bvim version %s %s\n", VERSION, copyright);

	/* just exit */
	exit(0);
}

