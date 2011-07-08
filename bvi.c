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
 * Copyright 1996-2004 by Gerhard Buergmann
 * gerhard@puon.at
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

#ifdef HAVE_LOCALE_H
#	include <locale.h>
#endif


char	*copyright  = "Copyright (C) 1996-2004 by Gerhard Buergmann";

jmp_buf	env;        /* context for `longjmp' function   */

int		loc;
int		maxx, maxy, x, xx, y;
int		screen, status;
off_t	size;
PTR		mem = NULL;
PTR		curpos;
PTR		maxpos;
PTR		pagepos;
PTR		spos;
char	*name = NULL;
char	*shell;
char	string[MAXCMD];
char	cmdstr[MAXCMD + 1] = "";
FILE	*Ausgabe_Datei;
int		edits = 0;
int		AnzAdd, Anzahl, Anzahl3;
off_t	filesize, memsize, undosize;
long	precount = -1;	/* number preceding command */

int		block_flag = 0;
off_t	block_begin, block_end, block_size;


char	**files;		/* list of input files */
int		numfiles;		/* number of input files */
int		curfile;		/* number of the current file */

int		arrnum = 0;
char	numarr[64];		/* string for collecting number */
char	rep_buf[BUFFER];

PTR		current;
PTR		last_motion;
PTR		current_start;
PTR		undo_start;
off_t	undo_count;
off_t	yanked = 0L;
char	*yank_buf = NULL;
char	*undo_buf = NULL;
PTR		markbuf[26];

char	addr_form[15];

char	*nobytes = "No bytes@in the buffer";

static	char	progname[8];
static	char	line[MAXCMD];
static	int		mark;
static	int		scrolly;
static	int		wrstat = 1;


void
usage()
{
	fprintf(stderr, "Usage: %s [-R] [-c cmd | +cmd] [-f script]\n\
       [-b begin] [-e end] [-s size] file ...\n", progname);
	exit(1);
}


int
main(argc, argv)
	int argc;
	char *argv[];
{
	int		ch;
	int		lflag;
	long	count;
	int		n = 1;
	int		script = -1;
	off_t	inaddr;
	char	*poi;

#ifdef HAVE_LOCALE_H
	setlocale(LC_ALL, "");
#endif
	poi = strrchr(argv[0], DELIM);

	if (poi) strncpy(progname, ++poi, 7);
		else strncpy(progname, argv[0], 7);
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
				if (argv[n + 1] == NULL || argv[n + 1][0] == '-') {
					usage();
				} else {
					script = ++n;
				}
			} else if (argv[n][1] == 'b') {
				if (argv[n + 1] == NULL || argv[n + 1][0] == '-') {
					usage();
				} else {
					block_begin = calc_size(argv[++n]);
					block_flag |= 1;
				}
			} else if (argv[n][1] == 'e') {
				if (argv[n + 1] == NULL || argv[n + 1][0] == '-') {
					usage();
				} else {
					block_end = calc_size(argv[++n]);
					block_flag |= 2;
				}
			} else if (argv[n][1] == 's') {
				if (argv[n + 1] == NULL || argv[n + 1][0] == '-') {
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
			} else usage();
			n++;
			break;
		case '+':			/* +cmd */
			if (argv[n][1] == '\0') {
				strcpy(cmdstr, "$");
			} else {
				strcpy(cmdstr, &argv[n][1]);
			}
			n++;
			break;
		default:			/* must be a file name */
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
	case 1|2:
		block_size = block_end - block_begin + 1;
		break;
	case 4:
		block_begin = 0;
	case 1|4:
		block_end = block_begin + block_size - 1;
		break;
	case 2|4:
		block_begin = block_end + 1 - block_size;
		break;
	case 1|2|4:
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
	if (numfiles > 1) fprintf(stderr, "%d files to edit\n", numfiles);
	curfile = 0;

	/****** Initialisation of curses ******/
	initscr();
	attrset(A_NORMAL);

	maxy = LINES;
	if (params[P_LI].flags & P_CHANGED) maxy = P(P_LI);
	scrolly = maxy / 2;
	P(P_SS) = scrolly;
	P(P_LI) = maxy;
	maxy--;
	keypad(stdscr, TRUE);
	scrollok(stdscr, TRUE);
	nonl();
	cbreak();
	noecho();

	/*
	AnzAdd = 8;
	strcpy(addr_form,  "%06lX  ");
	*/
	AnzAdd = 10;
	strcpy(addr_form,  "%08lX  ");

	Anzahl = ((COLS - AnzAdd - 1) / 16) * 4;
	P(P_CM) = Anzahl;
	maxx = Anzahl * 4 + AnzAdd + 1;
	Anzahl3 = Anzahl * 3;
	status = Anzahl3 + Anzahl - 17;
	screen = Anzahl * (maxy - 1);

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
		current = (PTR)(pagepos + y * Anzahl + xpos());
		if (wrstat) statpos();
		wrstat = 1;
		setcur();
		ch = vgetc();
		while (ch >= '0' && ch <= '9') {
			numarr[arrnum++] = ch;
			ch = vgetc();
		}
		numarr[arrnum] = '\0';
		if (arrnum != 0) precount = strtol(numarr, (char **)NULL, 10);
			else precount = -1;
		lflag = arrnum = 0;

		switch (ch) {
		case '^':	x = AnzAdd;
					loc = HEX;
					break;
		/*
		case '0':	x = AnzAdd + Anzahl3;
					loc = ASCII;
					break;
		*/
		case '$':	x = AnzAdd - 1 + Anzahl3 + Anzahl;
					loc = ASCII;
					break;
		case '\t':  toggle();
					break;
		case '~':	if (precount < 1) precount = 1;
					sprintf(rep_buf, "%ld~", precount);
					do_tilde(precount);
					lflag++;
					break;
		case KEY_HOME:
		case 'H':	if (precount > 0) {
						y = --precount;
						if (y > maxy - 1) {
							scrolldown(y - maxy + 1);
							y = maxy - 1; }
					} else y = 0;
					if (loc == HEX) x = AnzAdd;
						else	x = AnzAdd + Anzahl3;
					break;
		case 'M':	y = maxy / 2;
					if ((PTR)(pagepos + screen) > maxpos)
						y = (int)(maxpos - pagepos) / Anzahl / 2;
					if (loc == HEX) x = AnzAdd;
						else	x = AnzAdd + Anzahl3;
					break;
		case KEY_LL:
		case 'L':   if (precount < 1) precount = 1;
					n = maxy - 1;
					if ((PTR)((pagepos + screen)) > maxpos)
						n = (int)(maxpos - pagepos) / Anzahl;
					if (precount < n) y = n + 1 - precount;
					if (loc == HEX) x = AnzAdd;
						else	x = AnzAdd + Anzahl3;
					break;
		case BVICTRL('H'):
		case KEY_BACKSPACE:
		case KEY_LEFT:
		case 'h':	do {
						if (x > (AnzAdd + 2) && x < (Anzahl3 + AnzAdd + 1))
								x -= 3;
							else
								if (x > (Anzahl3 + AnzAdd - 2))  x--;
					} while (--precount > 0);
					if (x < AnzAdd + Anzahl3) loc = HEX;
						else loc = ASCII;
					break;
		case ' ':
		case KEY_RIGHT:
		case 'l':	do {
						/*
						if (x < (Anzahl3 + 6))  x += 3;
						*/
						if (x < (Anzahl3 + AnzAdd - 2))  x += 3;
							else if (x > (Anzahl3 + 3)
								&& x < (Anzahl3 + AnzAdd - 1 + Anzahl))
									x++;
					} while (--precount > 0);
					if (x < AnzAdd + Anzahl3) loc = HEX;
						else loc = ASCII;
					break;
		case '-':
		case KEY_UP :
		case 'k':	do {
						if (y > 0) y--;
							else scrollup(1);
					} while(--precount > 0);
					break;
		case '+':
		case CR:	if (loc == HEX) x = AnzAdd;
						else		x = AnzAdd + Anzahl3;
		case 'j':
		case BVICTRL('J'):
		case BVICTRL('N'):
		case KEY_DOWN:
					do {
						if ((PTR)((pagepos + (y + 1) * Anzahl)) > maxpos) break;
						if (y < (maxy - 1))	y++;
						else scrolldown(1);
					} while(--precount > 0);
					break;
		case '|':   if (precount < 1) break;
					if (loc == ASCII) x = AnzAdd - 1 + Anzahl3 + precount;
						else x = 5 + 3 * precount;
					if (x > AnzAdd - 1 + Anzahl3 + Anzahl) {
						x = AnzAdd - 1 + Anzahl3 + Anzahl;
						loc = ASCII; }
					break;
		case ':' :	clearstr();
					addch(ch);
					refresh();
					getcmdstr(cmdstr, 1);
					if (strlen(cmdstr))
						docmdline(cmdstr);
					break;
		case BVICTRL('B'):
		case KEY_PPAGE: 	/**** Previous Page ****/
					if (mem <= (PTR)(pagepos - screen))	pagepos -= screen;
						else	pagepos = mem;
					repaint();
					break;
		case BVICTRL('D'):
					if (precount > 1) scrolly = precount;
					scrolldown(scrolly);
					break;
		case BVICTRL('U'):
					if (precount > 1) scrolly = precount;
					scrollup(scrolly);
					break;
		case BVICTRL('E'):
					if (y > 0) y--;
					scrolldown(1);
					break;
		case BVICTRL('F'):
		case KEY_NPAGE: 	/**** Next Page *****/
					if (maxpos >= (PTR)(pagepos + screen))  {
						pagepos += screen;
						current += screen;
						if (current - mem >= filesize) {
							current = mem + filesize;
							setpage((PTR)(mem + filesize - 1L));
						}
						repaint();
					}
					break;
		case BVICTRL('G'):
					fileinfo(name);
					wrstat = 0;
					break;
		case BVICTRL('L'):   	/*** REDRAW SCREEN ***/
					new_screen();
					break;
		case BVICTRL('Y'):
					if (y < maxy) y++;
					scrollup(1);
					break;
		case 'A':   smsg("APPEND MODE");
					current = (PTR)(mem + filesize - 1L);
					setpage(current++);
					cur_forw(0);
					setcur();
					undosize = filesize;
					undo_count = edit(ch);
					break;
		case 'B':
		case 'b':	setpage(backsearch(current, ch));
					break;
		case 'e':	setpage(end_word(current));
					break;
		case ',':	do_ft(-1, 0);
					break;
		case ';':	do_ft(0, 0);
					break;
		case 'F':	
		case 'f':	
		case 't':	
		case 'T':	do_ft(ch, 0);
					break;
		case 'G':   last_motion = current;
					if (precount > -1)	{
						if ((precount < P(P_OF)) ||
								(precount - P(P_OF)) > (filesize - 1L)) {
							beep();
						} else { 
							setpage((PTR)(mem + precount - P(P_OF)));
						}
					} else { 
						setpage((PTR)(mem + filesize - 1L));
					}
					break;
		case 'g':	last_motion = current;
					msg("Goto Hex Address: ");
					refresh();
					getcmdstr(cmdstr, 19);
					if (cmdstr[0] == '^') {
						inaddr = P(P_OF);
					} else if (cmdstr[0] == '$') {
						inaddr = filesize + P(P_OF) - 1L;
					} else {
						long ltmp;
						sscanf(cmdstr, "%lx", &ltmp);
						inaddr = (off_t)ltmp;
					}
					if (inaddr < P(P_OF)) break;
					inaddr -= P(P_OF);
					if (inaddr < filesize)  {
						setpage(mem + inaddr);
					} else {
						if (filesize == 0L) break;
						sprintf(string, "Max. address of current file : %06lX",
							(long)(filesize - 1L + P(P_OF)));
						emsg(string);
					}
					break;
		case '?':
		case '/':	/**** Search String ****/
		case '#':
		case '\\':  clearstr();
					addch(ch);
					refresh();
					if (getcmdstr(line, 1)) break;
					last_motion = current;
					searching(ch, line, current, maxpos - 1, P(P_WS));
					break;
		case 'n': 		/**** Search Next ****/
		case 'N':   last_motion = current;
					searching(ch, "", current, maxpos - 1, P(P_WS));
					break;
		case 'm':	do_mark(vgetc(), current);
					break;
		case '\'':
		case '`':   if ((ch == '`' && loc == ASCII) ||
									(ch == '\'' && loc == HEX))
						toggle();
					mark = vgetc();
					if (mark == '`' || mark == '\'') {
						setpage(last_motion);
						last_motion = current;
					} else {
						if (mark < 'a' || mark > 'z') {
							beep(); break;
						} else if (markbuf[mark - 'a'] == NULL) {
								beep(); break;
						}
						setpage(markbuf[mark - 'a']);
					}
					break;
		case 'D':	if (precount < 1) precount = 1;
					sprintf(rep_buf, "%ldD", precount);
					trunc_cur();
					break;
		case 'o':	/* overwrite: this is an overwriting put */
					if (precount < 1) precount = 1;
					sprintf(rep_buf, "%ldo", precount);
					do_over(current, yanked, yank_buf);
					break;
		case 'P':
					if (precount < 1) precount = 1;
					if ((undo_count = alloc_buf(yanked, &undo_buf)) == 0L)
						break;
					sprintf(rep_buf, "%ldP", precount);
					if (do_append(yanked, yank_buf)) break;
					/* we save it not for undo but for the dot command
					memcpy(undo_buf, yank_buf, yanked);
					*/
					repaint();
					break;
		case 'r':
		case 'R':   if (filesize == 0L) break;
					if (precount < 1) precount = 1;
					sprintf(rep_buf, "%ld%c", precount, ch);
					undo_count = edit(ch);
					lflag++;
					break;
		case 'u':	do_undo();
					break;
		case 'W':
		case 'w':	loc = ASCII;
					setpage(wordsearch(current, ch));
					break;
		case 'y':	count = range(ch);
					if (count > 0) {
						if ((yanked = alloc_buf(count, &yank_buf)) == 0L) {
							break;
						}
						memcpy(yank_buf, current, yanked);
					} else if (count < 0) {
						if ((yanked = alloc_buf(-count, &yank_buf)) == 0L) {
							break;
						}
						memcpy(yank_buf, current + count, yanked);
					} else {
						break;
					}
/*
					sprintf(string, "%ld bytes yanked", labs(count));
					msg(string);
*/
					break;
		case 'z':	do_z(vgetc());
					break;
		case 'Z':   if (vgetc() == 'Z') do_exit();
						else beep();
					break;
		case '.':
					if (!strlen(rep_buf)) {
						beep();
					} else {
						stuffin(rep_buf);
					}
					break;
		default :
			if P(P_MM) {
				if (precount < 1) precount = 1;
				switch (ch) {
				case 'I':
					sprintf(rep_buf, "%ldI", precount);
					current = mem;
					setpage(mem);
					repaint();
					undo_count = edit('i');
					lflag++;
					break;
/* undo does not work correctly !!! */
				case 's':
					sprintf(rep_buf, "%lds", precount);
					if (do_delete((off_t)precount, current)) break;
					precount = 1;
					undo_count = edit('i');
					lflag++;
					break;
				case 'a':
					if (cur_forw(1)) break;
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
						do_delete((off_t)count, current);
					else if (count < 0)
						do_back(-(off_t)count, current);
					if (ch == 'c') {
						precount = 1;
						undo_count = edit('i');
						lflag++;
/*
					} else if (count) {
						sprintf(string, "%ld bytes deleted", labs(count));
						msg(string);
*/
					}
					break;
				case 'x':
					sprintf(rep_buf, "%ldx", precount);
					do_delete((off_t)precount, current);
					break;
				case 'X':
					sprintf(rep_buf, "%ldX", precount);
					do_back((off_t)precount, current);
					break;
				default:
					flushinp();
					beep();
				}
			} else {
				switch (ch) {
				case 'x':
					if (precount < 1) precount = 1;
					if ((off_t)precount + current == maxpos) {
						sprintf(rep_buf, "%ldx", precount);
						do_delete((off_t)precount, current);
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
		if (lflag) lineout();
	} while (1);
}


off_t
calc_size(arg)
	char	*arg;
{
	long	val;
	char	*poi;

	if (*arg == '0') {
		val = strtol(arg, &poi, 16);
	} else {
		val = strtol(arg, &poi, 10);
	}
	switch (*poi) {
	case 'k':
	case 'K':	val *= 1024;
			 	break;
	case 'm':
	case 'M':	val *= 1048576;
			 	break;
	case '\0':	break;
	default:	usage();
	}
	return (off_t)val;
}


void
trunc_cur()
{
	undosize = filesize;
	undo_count = (off_t)(maxpos - current);
	undo_start = current;
	filesize = pagepos - mem + y * Anzahl + xpos();
	maxpos = (PTR)(mem + filesize);
	if (filesize == 0L) {
		emsg(nobytes);
	} else	cur_back();
	edits = U_TRUNC;
	repaint();
}


int
do_append(count, buf)
	int		count;
	char	*buf;
{
	if (filesize + count > memsize) {
		if (enlarge(count + 100L)) return 1;
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


void
do_tilde(count)
	off_t	count;
{
	if (filesize == 0L) return;
	undo_start = current;
	if (current + count > maxpos) {
		beep();
		return;
	}
	if ((undo_count = alloc_buf(count, &undo_buf)) == 0L)
		return;
	memcpy(undo_buf, current, undo_count);
	while (count--) {
		if (isupper((int)(*current))) *current = tolower((int)(*current));
			else if (islower((int)(*current)))
				*current = toupper((int)(*current));
		current++;
		cur_forw(0);
	}
	edits = U_TILDE;
	setcur();
}


void
do_undo()
{
	off_t	n, tempsize;
	char	temp;
	PTR		set_cursor;
	PTR		s;
	PTR		d;

	if (undo_count == 0L) {
		emsg("Nothing to undo");
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
				s++; d++;
			}
			break;
		case U_APPEND:
		case U_TRUNC:
			tempsize = filesize;
			filesize = undosize;
			undosize = tempsize;
			maxpos = (PTR)(mem + filesize);
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
		if (edits == U_TRUNC && undosize > filesize) cur_back();
		repaint();
}


void
do_over(loc, n, buf)
	PTR		loc;
	off_t	n;
	PTR		buf;
{
	if (n < 1L) {
		emsg(nobytes);
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
	repaint();
}


void
do_put(loc, n, buf)
	PTR		loc;
	off_t	n;
	PTR		buf;
{
	if (n < 1L) {
		emsg(nobytes);
		return;
	}
	if (loc > maxpos) {
		beep();
		return;
	}
	if (filesize + n > memsize) {
		if (enlarge(n + 1024)) return;
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
	repaint();
}


/* argument sig not used, because only SIGINT will be catched */
void
jmpproc(sig)
	int	sig;
{
	if (P(P_EB)) beep();
	repaint();
	clearstr();
    signal(SIGINT, SIG_IGN);
    longjmp(env, 0);
}


off_t
range(ch)
	int		ch;
{
	int		ch1;
	long	count;

	ch1 = vgetc();
	while (ch1 >= '0' && ch1 <= '9') {
		numarr[arrnum++] = ch1;
		ch1 = vgetc();
	}
	numarr[arrnum] = '\0';
	if (arrnum != 0) count = strtol(numarr, (char **)NULL, 10);
		else count = 1;
	arrnum = 0;
	sprintf(rep_buf, "%ld%c%s%c", precount, ch, numarr, ch1);
	switch (ch1) {
	case '/':	/**** Search String ****/
	case '\\':
		strcat(rep_buf, "\n");
		clearstr();
		addch(ch1);
		refresh();
		if (getcmdstr(line, 1)) break;
		end_addr = searching(ch1, line, current, maxpos - 1, FALSE);
		if (!end_addr) {
			beep();
			return 0;
		}
		return(end_addr - current);
	case '?':
	case '#':
		strcat(rep_buf, "\n");
		clearstr();
		addch(ch1);
		refresh();
		if (getcmdstr(line, 1)) break;
		start_addr = searching(ch1, line, current, maxpos - 1, FALSE);
		if (!start_addr) {
			beep();
			return 0;
		}
		return(start_addr - current);
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
				- (off_t)P(P_OF)) > (filesize - 1L)) {
			beep();
			return 0;
		} else { 
			if (mem + count < current) {
				return(mem + count - current);
			} else {
				return(count - (current - mem));
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
			return(end_addr - current);
		} else {
			return(end_addr - current + 1);
		}
	}
	beep();
	return 0;
}
