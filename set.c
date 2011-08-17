/* SET.C - performing :set - command
 *
 * NOTE: Edit this file with tabstop=4 !
 *
 * 1996-02-29 created;
 * 1998-03-14 V 1.0.1
 * 1999-01-14 V 1.1.0
 * 1999-03-17 V 1.1.1
 * 1999-07-02 V 1.2.0 beta
 * 1999-08-14 V 1.2.0 final
 * 2000-07-15 V 1.3.0 final
 * 2001-10-10 V 1.3.1 
 * 2003-07-03 V 1.3.2
 *
 * Copyright 1996-2003 by Gerhard Buergmann 
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

#include "bvi.h"
#include "set.h"

extern struct BLOCK_ data_block[BLK_COUNT];

static int from_file = 0;
static FILE *ffp;
static char fbuf[256];
static char buf[64];

struct {
	short r;
	short g;
	short b;
} original_colors[8];

struct {
	short f;
	short b;
} original_colorpairs[8];

struct param params[] = {
	{"autowrite", "aw", FALSE, "", P_BOOL},
	{"columns", "cm", 16, "", P_NUM},
	{"errorbells", "eb", FALSE, "", P_BOOL},
	{"ignorecase", "ic", FALSE, "", P_BOOL},
	{"magic", "ma", TRUE, "", P_BOOL},
	{"memmove", "mm", FALSE, "", P_BOOL},
	{"offset", "of", 0, "", P_NUM},
	{"readonly", "ro", FALSE, "", P_BOOL},
	{"scroll", "scroll", 12, "", P_NUM},
	{"showmode", "mo", TRUE, "", P_BOOL},
	{"term", "term", 0, "", P_TEXT},
	{"terse", "terse", FALSE, "", P_BOOL},
	{"unixstyle", "us", FALSE, "", P_BOOL},
	{"window", "window", 25, "", P_NUM},
	{"wordlength", "wl", 4, "", P_NUM},
	{"wrapscan", "ws", TRUE, "", P_BOOL},
	{"", "", 0, "", 0,}	/* end marker */
};

struct color colors[] = {	/* RGB definitions and default value, if have no support of 256 colors */
	{"background", "bg", 50, 50, 50, COLOR_BLACK},
	{"addresses", "addr", 335, 506, 700, COLOR_BLUE},
	{"hex", "hex", 600, 600, 600, COLOR_MAGENTA},
	{"data", "data", 0, 800, 400, COLOR_GREEN},
	{"error", "err", 999, 350, 0, COLOR_RED},
	{"status", "stat", 255, 255, 255, COLOR_WHITE},
	{"command", "comm", 255, 255, 255, COLOR_WHITE},
	{"window", "win", 0, 800, 900, COLOR_YELLOW},
	{"addrbg", "addrbg", 0, 0, 0, COLOR_CYAN},
	{"", "", 0, 0, 0, 0}	/* end marker */
};

int doset(arg)
char *arg;			/* parameter string */
{
	int i;
	char *s;
	int did_window = FALSE;
	int state = TRUE;	/* new state of boolean parms. */
	char string[80];

	if (arg == NULL) {
		showparms(FALSE);
		return 0;
	}
	if (!strcmp(arg, "all")) {
		showparms(TRUE);
		return 0;
	}
	if (!strncmp(arg, "no", 2)) {
		state = FALSE;
		arg += 2;
	}

	/* extract colors section */
	if (!strncmp(arg, "color", 5)) {
		arg = substr(arg, 6, -1);
		for (i = 0; colors[i].fullname[0] != '\0'; i++) {
			s = colors[i].fullname;
			if (strncmp(arg, s, strlen(s)) == 0)
				break;
			s = colors[i].shortname;
			if (strncmp(arg, s, strlen(s)) == 0)
				break;
		}
		if (i == 0) {
			emsg("Wrong color name!");
			return 0;
		} else {
			colors[i].r = atoi(substr(arg, strlen(s) + 1, 3));
			colors[i].g = atoi(substr(arg, strlen(s) + 5, 3));
			colors[i].b = atoi(substr(arg, strlen(s) + 9, 3));
			set_palette();
			repaint();
		}
		return 0;
	} else {
		emsg(arg);
		return 1;
	}

	for (i = 0; params[i].fullname[0] != '\0'; i++) {
		s = params[i].fullname;
		if (strncmp(arg, s, strlen(s)) == 0)	/* matched full name */
			break;
		s = params[i].shortname;
		if (strncmp(arg, s, strlen(s)) == 0)	/* matched short name */
			break;
	}

	if (params[i].fullname[0] != '\0') {	/* found a match */
		if (arg[strlen(s)] == '?') {
			if (params[i].flags & P_BOOL)
				sprintf(buf, "    %s%s",
					(params[i].nvalue ? "  " : "no"),
					params[i].fullname);
			else if (params[i].flags & P_TEXT)
				sprintf(buf, "      %s=%s", params[i].fullname,
					params[i].svalue);
			else
				sprintf(buf, "      %s=%ld", params[i].fullname,
					params[i].nvalue);
			msg(buf);
			return 0;
		}
		if (!strcmp(params[i].fullname, "term")) {
			emsg("Can't change type of terminal from within bvi");
			return 1;
		}
		if (params[i].flags & P_NUM) {
			if ((i == P_LI) || (i == P_OF))
				did_window++;
			if (arg[strlen(s)] != '=' || state == FALSE) {
				sprintf(string, "Option %s is not a toggle",
					params[i].fullname);
				emsg(string);
				return 1;
			} else {
				s = arg + strlen(s) + 1;
				if (*s == '0') {
					params[i].nvalue = strtol(s, &s, 16);
				} else {
					params[i].nvalue = strtol(s, &s, 10);
				}
				params[i].flags |= P_CHANGED;

				if (i == P_CM) {
					if (((COLS - AnzAdd - 1) / 4) >=
					    P(P_CM)) {
						COLUMNS_DATA = P(P_CM);
					} else {
						COLUMNS_DATA = P(P_CM) =
						    ((COLS - AnzAdd - 1) / 4);
					}
					maxx = COLUMNS_DATA * 4 + AnzAdd + 1;
					COLUMNS_HEX = COLUMNS_DATA * 3;
					status = COLUMNS_HEX + COLUMNS_DATA - 17;
					screen = COLUMNS_DATA * (maxy - 1);
					did_window++;
					stuffin("H");	/* set cursor at HOME */
				}
			}
		} else {	/* boolean */
			if (arg[strlen(s)] == '=') {
				emsg("Invalid set of boolean parameter");
				return 1;
			} else {
				params[i].nvalue = state;
				params[i].flags |= P_CHANGED;
			}
		}
	} else {
		emsg("No such option@- `set all' gives all option values");
		return 1;
	}

	if (did_window) {
		maxy = P(P_LI) - 1;
		new_screen();
	}

	return 0;
}

/* show ALL parameters */
void showparms(all)
int all;
{
	struct param *p;
	int n;

	n = 2;
	msg("Parameters:\n");
	for (p = &params[0]; p->fullname[0] != '\0'; p++) {
		if (!all && ((p->flags & P_CHANGED) == 0))
			continue;
		if (p->flags & P_BOOL)
			sprintf(buf, "    %s%s\n",
				(p->nvalue ? "  " : "no"), p->fullname);
		else if (p->flags & P_TEXT)
			sprintf(buf, "      %s=%s\n", p->fullname, p->svalue);
		else
			sprintf(buf, "      %s=%ld\n", p->fullname, p->nvalue);

		msg(buf);
		n++;
		if (n == params[P_LI].nvalue) {
			if (wait_return(FALSE))
				return;
			n = 1;
		}
	}
	wait_return(TRUE);
}

void save_orig_palette()
{
	int i;
	for (i = 0; colors[i].fullname[0] != '\0'; i++) {
		color_content(colors[i].short_value, &original_colors[i].r, &original_colors[i].g, &original_colors[i].b);
	}
	for (i = 1; i < 8; i++) {
		pair_content(i, &original_colorpairs[i].f, &original_colorpairs[i].b);
	}
}

void load_orig_palette()
{
	int i;
	for (i = 0; colors[i].fullname[0] != '\0'; i++) {
		init_color(colors[i].short_value, original_colors[i].r, original_colors[i].g, original_colors[i].b);
	}
	for (i = 1; i < 8; i++) {
		init_pair(i, original_colorpairs[i].f, original_colorpairs[i].b);
	}
}

void set_palette()
{
	int i;
	if (can_change_color()) {
		for (i = 0; colors[i].fullname[0] != '\0'; i++) {
			if (init_color
			    (colors[i].short_value, C_r(i), C_g(i),
			     C_b(i)) == ERR)
				fprintf(stderr, "Failed to set [%d] color!\n",
					i);
			if (C_s(i) <= 7) {
				init_pair(i + 1, C_s(i), C_s(0));
			} else {
				colors[i].short_value = COLOR_WHITE;
				init_pair(i + 1, C_s(i), C_s(0));
			}
		}
		init_pair(C_AD + 1, C_s(C_AD), COLOR_CYAN);
	} else {		/* if have no support of changing colors */
		for (i = 0; colors[i].fullname[0] != '\0'; i++) {
			if (C_s(i) <= 7) {
				init_pair(i + 1, C_s(i), C_s(0));
			} else {
				colors[i].short_value = COLOR_WHITE;
				init_pair(i + 1, C_s(i), C_s(0));
			}
		}
	}
}

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

int do_logic(mode, str)
int mode;
char *str;
{
	int a, b;
	int value;
	size_t n;
	char *err_str = "Invalid value@for bit manipulation";

	if (mode == LSHIFT || mode == RSHIFT || mode == LROTATE
	    || mode == RROTATE) {
		value = atoi(str);
		if (value < 1 || value > 8) {
			emsg(err_str);
			return 1;
		}
	} else {
		if (strlen(str) == 8) {
			value = strtol(str, NULL, 2);
			for (n = 0; n < 8; n++) {
				if (str[n] != '0' && str[n] != '1') {
					value = -1;
					break;
				}
			}
		} else if (str[0] == 'b' || str[0] == 'B') {
			value = strtol(str + 1, NULL, 2);
		} else if (str[0] == '0') {
			value = strtol(str, NULL, 16);
			for (n = 0; n < strlen(str); n++) {
				if (!isxdigit(str[n])) {
					value = -1;
					break;
				}
			}
		} else {
			value = atoi(str);
		}
		if (value < 0 || value > 255) {
			emsg(err_str);
			return 1;
		}
	}
	if ((undo_count =
	     alloc_buf((off_t) (end_addr - start_addr + 1), &undo_buf))) {
		memcpy(undo_buf, start_addr, undo_count);
	}
	undo_start = start_addr;
	edits = U_EDIT;
	while (start_addr <= end_addr) {
		a = *start_addr;
		a &= 0xff;
		switch (mode) {
		case LSHIFT:
			a <<= value;
			break;
		case RSHIFT:
			a >>= value;
			break;
		case LROTATE:
			a <<= value;
			b = a >> 8;
			a |= b;
			break;
		case RROTATE:
			b = a << 8;
			a |= b;
			a >>= value;
			/*
			   b = a << (8 - value);
			   a >>= value; 
			   a |= b;
			 */
			break;
		case AND:
			a &= value;
			break;
		case OR:
			a |= value;
			break;
		case XOR:
		case NOT:
			a ^= value;
			break;
		case NEG:
			a ^= value;
			a++;	/* Is this true */
			break;
		}
		*start_addr++ = (char)(a & 0xff);
	}
	repaint();
	return (0);
}

int do_logic_block(mode, str, block_number)
int mode;
char *str;
int block_number;
{
	int a, b;
	int value;
	size_t n;
	char *err_str = "Invalid value@for bit manipulation";

	if ((block_number >= BLK_COUNT) & (!(data_block[block_number].pos_start < data_block[block_number].pos_end))) {
		emsg("Invalid block for bit manipulation!");
		return 1;
	}
	if (mode == LSHIFT || mode == RSHIFT || mode == LROTATE
	    || mode == RROTATE) {
		value = atoi(str);
		if (value < 1 || value > 8) {
			emsg(err_str);
			return 1;
		}
	} else {
		if (strlen(str) == 8) {
			value = strtol(str, NULL, 2);
			for (n = 0; n < 8; n++) {
				if (str[n] != '0' && str[n] != '1') {
					value = -1;
					break;
				}
			}
		} else if (str[0] == 'b' || str[0] == 'B') {
			value = strtol(str + 1, NULL, 2);
		} else if (str[0] == '0') {
			value = strtol(str, NULL, 16);
			for (n = 0; n < strlen(str); n++) {
				if (!isxdigit(str[n])) {
					value = -1;
					break;
				}
			}
		} else {
			value = atoi(str);
		}
		if (value < 0 || value > 255) {
			emsg(err_str);
			return 1;
		}
	}
	if ((undo_count =
	     alloc_buf((off_t) (data_block[block_number].pos_end - 
			data_block[block_number].pos_start + 1), &undo_buf))) {
		memcpy(undo_buf, start_addr +  data_block[block_number].pos_start, undo_count);
	}
	undo_start = start_addr + data_block[block_number].pos_start;
	edits = U_EDIT;
	start_addr = start_addr + data_block[block_number].pos_start;
	end_addr = start_addr + data_block[block_number].pos_end - data_block[block_number].pos_start;
	while (start_addr <= end_addr) {
		a = *start_addr;
		a &= 0xff;
		switch (mode) {
		case LSHIFT:
			a <<= value;
			break;
		case RSHIFT:
			a >>= value;
			break;
		case LROTATE:
			a <<= value;
			b = a >> 8;
			a |= b;
			break;
		case RROTATE:
			b = a << 8;
			a |= b;
			a >>= value;
			/*
			   b = a << (8 - value);
			   a >>= value; 
			   a |= b;
			 */
			break;
		case AND:
			a &= value;
			break;
		case OR:
			a |= value;
			break;
		case XOR:
		case NOT:
			a ^= value;
			break;
		case NEG:
			a ^= value;
			a++;	/* Is this true */
			break;
		}
		*start_addr++ = (char)(a & 0xff);
	}
	repaint();
	return (0);
}


int getcmdstr(p, x)
char *p;
int x;
{
	int c;
	int n;
	char *buff, *q;

	attron(COLOR_PAIR(C_CM + 1));

	if (from_file) {
		if (fgets(p, 255, ffp) != NULL) {
			strtok(p, "\n\r");
			return 0;
		} else {
			return 1;
		}
	}

	signal(SIGINT, jmpproc);
	buff = p;
	move(maxy, x);
	do {
		switch (c = vgetc()) {
		case BVICTRL('H'):
		case KEY_BACKSPACE:
		case KEY_LEFT:
			if (p > buff) {
				p--;
				move(maxy, x);
				n = x;
				for (q = buff; q < p; q++) {
					addch(*q);
					n++;
				}
				addch(' ');
				move(maxy, n);
			} else {
				*buff = '\0';
				msg("");
				attroff(COLOR_PAIR(C_CM + 1));
				signal(SIGINT, SIG_IGN);
				return 1;
			}
			break;
		case ESC:	/* abandon command */
			*buff = '\0';
			msg("");
			attroff(COLOR_PAIR(C_CM + 1));
			signal(SIGINT, SIG_IGN);
			return 1;
#if NL != KEY_ENTER
		case NL:
#endif
#if CR != KEY_ENTER
		case CR:
#endif
		case KEY_ENTER:
			break;
		default:	/* a normal character */
			addch(c);
			*p++ = c;
			break;
		}
		refresh();
	} while (c != NL && c != CR && c != KEY_ENTER);
	attroff(COLOR_PAIR(C_CM + 1));

	*p = '\0';
	signal(SIGINT, SIG_IGN);
	return 0;
}
