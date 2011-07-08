/* bm_unix.c - Unix specific subroutines for BMORE
 *
 * 2000-05-31 V 1.3.0 beta
 * 2000-10-12 V 1.3.0 final
 * 2002-02-10 V 1.3.1
 * 2003-07-04 V 1.3.2
 *
 * NOTE: Edit this file with tabstop=4 !
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


#include "bmore.h"
#include <termios.h>

#define TBUFSIZ 1024

#define stty(fd,argp)   tcsetattr(fd,TCSAFLUSH,argp)

struct termios ostate, nstate;


char	*rev_start, *rev_end;	/* enter and exit standout mode */
char	*Home;			/* go to home */
char	*clear_sc;		/* clear screen */
char	*erase_ln;		/* erase line */

extern	off_t	bytepos, screen_home;
extern	FILE	*curr_file;

int		got_int;
int     fnum, no_intty, no_tty, slow_tty;
int     dum_opt, dlines;

#ifdef HAVE_NCURSES_H
#	undef NEED_PUTC_CHAR
#endif
/*
 * A real function, for the tputs routine
 */
#ifdef NEED_PUTC_CHAR

int
putchr(char ch)
{return putchar((int)ch);}

#else

int
putchr(ch)
	int ch;
{return putchar(ch);}

#endif


void
initterm()
{
	char        buf[TBUFSIZ];
	static char clearbuf[TBUFSIZ];
	char        *term;
	char        *clearptr;

	struct  termios nstate;

	no_tty = tcgetattr(fileno(stdout), &ostate);
	if (!no_tty) {
		nstate = ostate;
		/*
		 * is this really necessary??
		 *
		nstate.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHONL);
		 */
		nstate.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHONL);
		nstate.c_cc[VMIN] = 1;
		nstate.c_cc[VTIME] = 0;
		tcsetattr(fileno(stdin), TCSADRAIN, &nstate);
	}

#ifdef DJGPP
	maxx = 80;
	maxy = 25;
#else
	if ((term = getenv("TERM")) == 0 || tgetent(buf, term) <= 0) {
		printf("Dumb terminal\n");
		maxx = 80;
		maxy = 24;
	} else {
		maxy = tgetnum("li");
		maxx = tgetnum("co");
	}
	clearptr = clearbuf;
	erase_ln = tgetstr("ce", &clearptr);
	clear_sc = tgetstr("cl", &clearptr);
	rev_start = tgetstr("so", &clearptr);
	rev_end = tgetstr("se", &clearptr);
#endif

	no_intty = tcgetattr(fileno(stdin), &ostate);
	tcgetattr(fileno(stderr), &ostate);
	  
	nstate = ostate;
	if (!no_tty) {
		ostate.c_lflag &= ~(ICANON|ECHO);
	}
}


void
set_tty()
{
	if (no_tty) return;
	ostate.c_lflag &= ~(ICANON|ECHO);
	stty(fileno(stderr), &ostate);
}


void
reset_tty()
{
	if (no_tty) return;
	ostate.c_lflag |= ICANON|ECHO;
	stty(fileno(stderr), &ostate);
}


void
sig(sig)
	int sig;
{
	reset_tty();
	printf("\r\n");
	exit(0);
}


/*
 * doshell() - run a command or an interactive shell
 */
void
doshell(cmd)
	char	*cmd;
{
#ifndef DJGPP
	char	*getenv();
	char	*shell;
	char	cline[128];
#endif

	printf("\n");

#ifndef DJGPP
	if ((shell = getenv("SHELL")) == NULL) shell = "sh";
	else if(strrchr(shell,'/')) shell=(char *)(strrchr(shell,'/')+1);

	if (cmd[0] == '\0') {
		sprintf(cline, "%s -i", shell);
		cmd = cline;
	} else {
		sprintf(cline, "%s -c \"%s\"", shell, cmd);
		cmd = cline;
	}
#endif

	reset_tty();
	system(cmd);
	set_tty();
	printf("\r");
	home();
	fseek(curr_file, screen_home, SEEK_SET);
	bytepos = screen_home;
}


void
highlight()
{
#ifndef DJGPP
	if (rev_start && rev_end)
		tputs(rev_start, 1, putchr);
#endif
}


void
normal()
{
#ifndef DJGPP
	if (rev_start && rev_end)
		tputs(rev_end, 1, putchr);
#endif
}


void
clearscreen()
{
#ifdef DJGPP
	/* if (!no_tty)
	{
		int	n;

		for (n = 0; n < maxy; n++) {
			cleartoeol();
			printf("\n");
		}
	} */
#else
	tputs(clear_sc, 1, putchr);
#endif
}


void
home()
{
#ifdef DJGPP
	if (!no_tty) printf("\r");
#else
	tputs(Home, 1, putchr);
#endif
}


/* force clear to end of line */
void
cleartoeol()
{
#ifdef DJGPP
	int	n;

	home();
	if (!no_tty) for (n = 1; n < maxx; n++) printf(" ");
	home();
#else
	tputs(erase_ln, 1, putchr);
#endif
}


int
vgetc()
{
    char cha;
    extern int errno;

    errno = 0;
    if (read(2, &cha, 1) <= 0) {
        if (errno != EINTR)
            exit(2);
    }
    return (cha);
}


#ifndef HAVE_MEMMOVE
/*
 * Copy contents of memory (with possible overlapping).
 */
char *
memmove(s1, s2, n)
	char    *s1;
	char    *s2;
	size_t  n;
{
	bcopy(s2, s1, n);
	return(s1);
}
#endif
