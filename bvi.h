/*  BVI.H
 *
 *  NOTE: Edit this file with tabstop=4 !
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <sys/stat.h>
#include <setjmp.h>

#include "patchlevel.h"
#include "config.h"

#if HAVE_NCURSES_H
#include <ncurses.h>
#else
#include <curses.h>
#endif

/* defines for filemode */
#define	ERROR				-1
#define REGULAR				0
#define NEW					1
#define DIRECTORY			2
#define CHARACTER_SPECIAL	3
#define BLOCK_SPECIAL		4
#define PARTIAL             5

/* regular expressions */
#define END     0
#define ONE     1
#define STAR    2

/* undo modes */
#define U_EDIT		1	/* undo o r R */
#define U_TRUNC		2	/* undo D */
#define U_INSERT	4	/* undo i */
#define U_DELETE	8	/* undo x */
#define U_BACK		16	/* undo X */
#define U_APPEND	32	/* undo P A */
#define U_TILDE		64	/* ~ */

#define S_GLOBAL	0x100

/* logic modes */
#define LSHIFT  1
#define RSHIFT  2
#define LROTATE 3
#define RROTATE 4
#define AND 5
#define OR  6
#define XOR 7
#define NEG 8
#define NOT 9

#define HEX			0
#define ASCII		1
#define FORWARD		0
#define BACKWARD	1
#define CR			'\r'
#define NL			'\n'
#define BS			8
#define	ESC			27
#define SEARCH		0

#define CMDLNG(a,b)     (len <= a && len >= b)

#ifndef NULL
#	define NULL		((void *)0)
#endif

#ifndef TRUE
#	define TRUE		1
#	define FALSE	0
#endif

#define PTR		char *
#define DELIM	'/'

/* Define escape key */
#define KEY_ESC		27

#define MAXCMD	255
#define BUFFER	1024

#define BLK_COUNT 32		/* number of data blocks */
#define MARK_COUNT 64		/* number of markers */

#define SKIP_WHITE  while(*cmd!='\0'&&isspace(*cmd))cmd++;

#ifdef DEBUG
extern FILE *debug_fp;
#endif

#ifndef HAVE_STRERROR
extern char *sys_errlist[];
#endif

struct MARKERS_ {
	long address;
	char marker;		/* Usually we use '+' character, but can be another */
};

/* ---------------------------------
 * CORE structure - each for
 * different buffers
 * --------------------------------- */

struct CORE {
	struct {
		int COLUMNS_DATA;
		int COLUMNS_HEX;
		int COLUMNS_ADDRESS;
		/*
		   struct colors {} */
	} params;
	struct {
		PTR mem;
		PTR maxpos;
	} editor;
	struct {
		int maxy;
		int maxx;
	} screen;
	/*
	   struct MARKERS;
	 */
};

typedef struct CORE core_t;

/* -----------------------------------
 * Current state of the current buffer
 * ----------------------------------- */

#define BVI_MODE_CMD	1
#define BVI_MODE_EDIT	2
#define BVI_MODE_VISUAL 3
#define BVI_MODE_REPL	4

struct STATE {
	/* Command, Edit, Visual or REPL modes */
	int mode;
	/* Current positions */
	PTR pagepos;
	PTR curpos;
	PTR mempos;
	int x;
	int y;
	int loc;
	int screen;
	int scrolly;
	int toggle_selection;
	struct {
		long start;
		long end;
	} selection;
};

typedef struct STATE state_t;

extern char *version;
extern char addr_form[];
extern char pattern[];
extern char rep_buf[];
extern int x, y;
extern int filemode;
extern int edits, new;
extern int addr_flag;
extern int ignore_case, magic;
extern int screen, status;
extern PTR maxpos;
extern PTR undo_start;
extern PTR current_start;
extern PTR curpos;
extern PTR current;
extern PTR start_addr;
extern PTR end_addr;
extern char *name, cmdstr[];
extern off_t filesize, memsize;
extern PTR markbuf[];
extern PTR last_motion;
extern off_t undo_count;
extern off_t yanked;
extern off_t undosize;
extern char *copyright, *notfound;
extern char *terminal;
extern char *undo_buf;
extern char *yank_buf;
extern int repl_count;
extern char string[];
extern char *shell;
extern char *poi;
extern int smode;
extern int again;
extern int block_flag;
extern off_t block_begin, block_end, block_size;

#ifndef S_ISDIR			/* POSIX 1003.1 file type tests. */
#define	S_ISDIR(m)	((m & 0170000) == 0040000)	/* directory */
#define	S_ISCHR(m)	((m & 0170000) == 0020000)	/* char special */
#define	S_ISBLK(m)	((m & 0170000) == 0060000)	/* block special */
#define	S_ISREG(m)	((m & 0170000) == 0100000)	/* regular file */
#define	S_ISFIFO(m)	((m & 0170000) == 0010000)	/* fifo */
#endif

/* ================= Debug utilities ================ */

void bvi_error(int mode, char* fmt, ...);
void bvi_info(int mode, char* fmt, ...);
void bvi_debug(int mode, char* fmt, ...);

/* ================= Exports ================ */

off_t alloc_buf(off_t, char **), yd_addr(void);
off_t range(int);
void do_dot(void), do_exit(void), do_shell(void), do_undo(void);
void do_tilde(off_t), trunc_cur(void);
void do_back(off_t, PTR), do_ins_chg(PTR, char *, int);
void do_mark(int, PTR), badcmd(char *), movebyte(void);
void do_over(PTR, off_t, PTR), do_put(PTR, off_t, PTR);
void jmpproc(int), printline(PTR, int);
void wmsg(char *);
int addfile(char *);
int bregexec(PTR, char *);
int chk_comm(int);
int doecmd(char *, int);
int do_append(int, char *), do_logic(int, char *);
int do_logic_block(int, char *, int);
int do_delete(off_t, PTR);
int doset(char *);
int do_substitution(int, char *, PTR, PTR);
int hexchar(void);
int outmsg(char *);
PTR searching(int, char *, PTR, PTR, int);
PTR wordsearch(PTR, char);
PTR backsearch(PTR, char);
PTR fsearch(PTR, PTR, char *);
PTR rsearch(PTR, PTR, char *);
PTR end_word(PTR);
PTR calc_addr(char **, PTR);
PTR do_ft(int, int);
char *patcpy(char *, char *, char);
void setpage(PTR), smsg(char *);

void usage(void), bvi_init(char *), statpos(void), setcur(void);
void showparms(int), toggle(void), scrolldown(int), scrollup(int);
void fileinfo(char *);
void clearstr(void), clear_marks(void);

void quit(void), sysemsg(char *), do_z(int), stuffin(char *);
off_t edit(int), load(char *);
off_t calc_size(char *);
int ascii_comp(char *, char *), hex_comp(char *, char *);
int cur_forw(int), cur_back(void);
int lineout(void), save(char *, PTR, PTR, int);
int at_least(char *, char *, int);
int vgetc(void), xpos(void), enlarge(off_t);
int getcmdstr(char *, int), read_rc(char *);
int wait_return(int);
char *substr(const char *, size_t, size_t);
int read_history(char *filename);
void record_cmd(char* cmdline);

/* ========= Event handlers ======== */

int handler__goto_HEX();
int handler__goto_ASCII();
int handler__toggle();
int handler__tilda();
int handler__goto_home();
int handler__M();
int handler__L();
int handler__goto_left();
int handler__goto_right();
int handler__goto_up();
int handler__goto_EOL();
int handler__goto_down();
int handler__line();
int handler__toolwin_toggle();
int handler__cmdstring();
int handler__previous_page();
int handler__scrolldown();
int handler__scrollup();
int handler__linescroll_down();
int handler__nextpage();
int handler__fileinfo();
int handler__screen_redraw();
int handler__linescroll_up();
int handler__luarepl();
int handler__toggle_selection();
int handler__append_mode();
int handler__backsearch();
int handler__setpage();
int handler__doft1();
int handler__doft2();
int handler__doft3_F();
int handler__doft3_f();
int handler__doft3_t();
int handler__doft3_T();
int handler__goto1();
int handler__goto2();
int handler__search_string1();
int handler__search_string2();
int handler__search_string3();
int handler__search_string4();
int handler__search_next();
int handler__mark();
int handler__goto_mark();
int handler__trunc();
int handler__overwrite();
int handler__paste();
int handler__redo();
int handler__undo();
int handler__visual();
int handler__wordsearch();
int handler__yank();
int handler__doz();
int handler__exit();
int handler__stuffin();
int handler__insert();
int handler__s();
int handler__append2();
int handler__insert2();
int handler__paste2();
int handler__c_or_d();
int handler__x();
int handler__x2();

