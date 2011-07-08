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

static	int		from_file = 0;
static	FILE	*ffp;
static	char	fbuf[256];
static	char	buf[64];

struct	param	params[] = {
	{ "autowrite",	"aw",		FALSE,	"",	P_BOOL },
	{ "columns",	"cm",		16,		"",	P_NUM },
	{ "errorbells",	"eb",		FALSE,	"",	P_BOOL },
	{ "ignorecase",	"ic",		FALSE,	"",	P_BOOL },
	{ "magic",		"ma",		TRUE,	"",	P_BOOL },
	{ "memmove",	"mm",		FALSE,  "",	P_BOOL },
	{ "offset",		"of",		0,		"",	P_NUM },
	{ "readonly",	"ro",		FALSE,	"",	P_BOOL },
	{ "scroll",		"scroll",	12,		"",	P_NUM },
	{ "showmode",	"mo",		TRUE,	"",	P_BOOL },
	{ "term",		"term",		0,		"",	P_TEXT },
	{ "terse",		"terse",	FALSE,	"",	P_BOOL },
	{ "unixstyle",	"us",		FALSE,	"",	P_BOOL },
	{ "window",		"window",	25,		"",	P_NUM },
	{ "wordlength",	"wl",		4,		"",	P_NUM },
	{ "wrapscan",	"ws",		TRUE,	"",	P_BOOL },
#if defined(__MSDOS__) && !defined(DJGPP)
	{ "color",		"co",		7,		"",	P_NUM  },
#endif
	{ "",			"",			0,		"",	0, }		/* end marker */

};

int
doset(arg)
	char	*arg;		/* parameter string */
{
	int		i;
	char	*s;
	int		did_window = FALSE;
	int		state = TRUE;		/* new state of boolean parms. */
	char	string[80];

	if (arg == NULL) {
		showparms(FALSE);
		return 0;
	}
	if (!strcmp(arg, "all")) {
		showparms(TRUE);
		return 0;	}
	if (!strncmp(arg, "no", 2)) {
		state = FALSE;
		arg += 2; }

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
					(params[i].nvalue ? "  " : "no"), params[i].fullname);
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
			if ((i == P_LI) || (i == P_OF)) did_window++;
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
#if defined(__MSDOS__) && !defined(DJGPP)
				if (i == P_CO) {
					textcolor(P(P_CO) & 0x07);
					textbackground((P(P_CO) & 0xf0) >> 4);
					clrscr();
					repaint();
				}
#endif
				if (i == P_CM) {
					if (((COLS - AnzAdd - 1) / 4) >= P(P_CM)) {
						Anzahl = P(P_CM);
					} else {
						Anzahl = P(P_CM) = ((COLS - AnzAdd - 1) / 4);
					}
				    maxx = Anzahl * 4 + AnzAdd + 1;
				    Anzahl3 = Anzahl * 3;
				    status = Anzahl3 + Anzahl - 17;
				    screen = Anzahl * (maxy - 1);
					did_window++;
					stuffin("H");	/* set cursor at HOME */
				}
			}
		} else {		/* boolean */
			if (arg[strlen(s)] == '=') {
				emsg("Invalid set of boolean parameter");
				return 1;
			} else {
				params[i].nvalue = state;
				params[i].flags |= P_CHANGED;
			}
		}
	} else  {
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
void
showparms(all)
	int	all;
{
	struct	param	*p;
	int		n;

	n = 2;
	msg("Parameters:\n");
	for (p = &params[0]; p->fullname[0] != '\0' ;p++) {
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
			if (wait_return(FALSE)) return;
			n = 1;
		}
	}
	wait_return(TRUE);
}


/* reads the init file (.bvirc) */
int
read_rc(fn)
	char *fn;
{
	if((ffp = fopen(fn, "r")) == NULL) return -1;
	from_file = 1;
	while(fgets(fbuf, 255, ffp) != NULL) {
		strtok(fbuf, "\n\r");
		docmdline(fbuf);
	}
	fclose(ffp);
	from_file = 0;
	return 0;
}


int
do_logic(mode, str)
	int		mode;
	char	*str;
{
	int		a, b;
	int		value;
	size_t	n;
	char	*err_str ="Invalid value@for bit manipulation";

	if (mode==LSHIFT || mode==RSHIFT || mode==LROTATE || mode==RROTATE) {
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
	if ((undo_count = alloc_buf((off_t)(end_addr - start_addr + 1), &undo_buf))) {
		memcpy(undo_buf, start_addr, undo_count);
	}
	undo_start = start_addr;
	edits = U_EDIT;
	while(start_addr <= end_addr) {
		a = *start_addr;
		a &= 0xff;
		switch(mode) {
		case LSHIFT:	a <<= value;
						break;
		case RSHIFT:	a >>= value;
						break;
		case LROTATE:	a <<= value;
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
		case AND:		a &= value;
						break;
		case OR:		a |= value;
						break;
		case XOR:		
		case NOT:		a ^= value;
						break;
		case NEG:		a ^= value;
						a++;		/* Is this true */
						break;
		}
		*start_addr++ = (char)(a & 0xff);
	}
	repaint();
	return(0);
}


int
getcmdstr(p, x)
	char    *p;
	int		x;
{
	int		c;
	int		n;
	char	*buff, *q;

	if (from_file) {
		if(fgets(p, 255, ffp) != NULL) {
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
                for (q = buff; q < p ;q++)  {
                    addch(*q);
					n++;
				}
				addch(' ');
				move(maxy, n);
            } else {
				*buff = '\0';
                msg("");
				signal(SIGINT, SIG_IGN);
                return 1;
            }
            break;
        case ESC:           /* abandon command */
			*buff = '\0';
            msg("");
			signal(SIGINT, SIG_IGN);
            return  1;
#if NL != KEY_ENTER
        case NL:
#endif
#if CR != KEY_ENTER
        case CR:
#endif
		case KEY_ENTER:
            break;
        default:        /* a normal character */
            addch(c);
            *p++ = c;
            break;
        }
		refresh();
    } while (c != NL && c != CR && c != KEY_ENTER);

    *p = '\0';
	signal(SIGINT, SIG_IGN);
	return 0;
}
