/* DOSIO.C - file I/O and alloc subroutines for MSDOS - BVI
 *
 * 1996-02-28 V 1.0.0
 * 1998-04-12 V 1.0.1
 * 1999-01-14 V 1.1.0
 * 1999-04-27 V 1.1.1
 * 1999-07-02 V 1.2.0 beta
 * 1999-09-01 V 1.2.0 final
 * 2000-05-02 V 1.3.0 alpha
 *
 * NOTE: Edit this file with tabstop=4 !
 *
 * Copyright 1996-2002 by Gerhard Buergmann
 * Gerhard.Buergmann@puon.at
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

#include <conio.h>
#include <alloc.h>
#include <fcntl.h>
#include <bios.h>
#include <io.h>
#include <mem.h>
#include "bvi.h"
#include "set.h"

char	*terminal = "ansi";

extern maxy;
struct WINDOW scr;

int	stdscr = 0;

int	COLS = 80;
int	LINES =	25;
int	ECHO = TRUE;
int	NODEL = FALSE;

static  struct  stat    buf;
int		filemode;

int	inch(void);

void
attrset(int attr)
{
	switch(attr) {
	case A_NORMAL:  textcolor(P(P_CO) & 0x07);
					textbackground((P(P_CO) & 0xf0) >> 4);
					break;
	case A_BOLD:	textcolor((P(P_CO) & 0x07) | 0x08);
					textbackground((P(P_CO) & 0xf0) >> 4);
					break;
	case A_REVERSE: textcolor((P(P_CO) & 0xf0) >> 4);
					textbackground(P(P_CO) & 0x0f);
					break;
	case A_BLINK:	textcolor((P(P_CO) & 0xf0) >> 4);
					textbackground(P(P_CO) & 0x0f);
					break;
	}
}


int
getch()
{
	int	x;

	if (NODEL) if (!kbhit()) return(ERR);
	x = bioskey(0);
	if ((x & 0xff) == 0) return(x);
		else {
			x &= 0xff;
			if (ECHO) putch(x);
			return(x); }
}


void
delch()
{
	int x, y;

	x = wherex(); y = wherey();
	movetext(x + 1, y, COLS - x, y, x, y);
	gotoxy(COLS, y);
	putch(' ');
	gotoxy(x, y);
}


void
insch(int c)
{
	char line[80];
	int x, y;

	x = wherex(); y = wherey();
	gettext(x, y, COLS - x, y, line);
	puttext(x + 1, y, COLS - x + 1, y, line);
	putch(c);
}


int
inch()
{
	return peek(0xb800, (wherex() - 1) + (wherey() - 1) * COLS);
}

int
mvinch(int y, int x)
{
	gotoxy(x + 1, y + 1);
	return inch();
}


/*********** Save the patched file ********************/
int
save(char *fname, PTR start, PTR end, int flags)
{
	int		fd;
	char	string[255];
	char	*newstr;
	off_t	written;
	off_t	filesize;
	unsigned n, to_write;

	if (!fname) {
		emsg("No file|No current filename");
		return 0;
		}
	if (stat(fname, &buf) == -1) {
		newstr = "[New file] ";
	 } else {
		 if (S_ISDIR(buf.st_mode)) {
			 sprintf(string, "\"%s\" Is a directory", fname);
			 msg(string);
			 return 0;
		}
		newstr = "";
	}

	if (filemode == PARTIAL) flags = O_RDWR;
	if ((fd = open(fname, flags, 0666)) < 0) {
		sysemsg(fname);
		return 0;
	}
	if (filemode == PARTIAL) {
        if (block_read) {
            filesize = block_read;
            sprintf(string, "\"%s\" range %lu-%lu", fname,
                (unsigned long)block_begin,
                (unsigned long)(block_begin - 1 + filesize));
            if (lseek(fd, block_begin, SEEK_SET) < 0) {
                sysemsg(fname);
                return 0;
            }
        } else {
            msg("Null range");
            return 0;
        }
    } else {
		filesize = end - start + 1L;
		sprintf(string, "\"%s\" %s%lu@bytes", fname, newstr, (long)filesize);
	}

	written = 0;
	do {
		to_write = (filesize - written) > 0xfffe ? 0xfffe : (filesize - written);
		if ((n = write(fd, start + written, to_write)) == 0xffff) {
			sysemsg(fname);
			close(fd);
			return(0L);
		}
		written += (off_t)n;
	} while (written < filesize);
	close(fd);
	edits = 0;
	msg(string);
	return 1;
}


off_t
load(char *fname)
{
	int		fd = -1;
	char	string[MAXCMD];
	unsigned chunk, n;

	buf.st_size = filesize = 0L;
	if (fname != NULL) {
		if (stat(fname, &buf) == -1) {
			filemode = NEW;
		} else if (S_ISDIR(buf.st_mode)) {
			filemode = DIRECTORY;
		} else if (S_ISREG(buf.st_mode)) {
			if ((fd = open(fname, O_RDONLY|O_BINARY)) > 0) {
				filemode = REGULAR;
				if (access(fname, 2)) {
					P(P_RO) = TRUE;
					params[P_RO].flags |= P_CHANGED;
				}
			} else {
				sysemsg(fname);
				filemode = ERROR;
			}
		}
	} else {
		filemode = NEW;
	}
	if (mem != NULL) farfree(mem);
	if (block_flag) {
		memsize = block_size + 1000;
	} else if (filemode == REGULAR) {
		memsize = buf.st_size + 100;
	} else {
		 memsize = 1000;
	}

	if (farcoreleft() < memsize) {
		move(maxy, 0);
		endwin();
		printf("\n\nOut of memory\n");
		exit(0);
	}
	mem = (char huge *)farmalloc(memsize);

	clear_marks();
	if (block_flag && (filemode == REGULAR)) {
		if (lseek(fd, block_begin, SEEK_SET) < 0) {
			sysemsg(fname);
			filemode = ERROR;
		} else {
			chunk = block_size > 0xfffe ? 0xfffe : block_size;
			do {
				if ((n = read(fd, mem + filesize, chunk)) == 0xffff) {
					sysemsg(fname);
					filemode = ERROR;
					break;
				}
				filesize += (off_t)n;
			} while (filesize < buf.st_size);
			if ((filesize == 0) {
                sprintf(string, "\"%s\" No such range: %lu-%lu", fname,
                    (unsigned long)block_begin, (unsigned long)(block_end));
            } else {
                sprintf(string, "\"%s\" range %lu-%lu", fname,
                    (unsigned long)block_begin,
                    (unsigned long)(block_begin + filesize - 1));
            }
            filemode = PARTIAL;
            block_read = filesize;
            msg(string);
            P(P_OF) = block_begin;
            params[P_OF].flags |= P_CHANGED;
		}
	} else if (filemode == REGULAR) {
		chunk = buf.st_size > 0xfffe ? 0xfffe : buf.st_size;
		do {
			if ((n = read(fd, mem + filesize, chunk)) == 0xffff) {
				sysemsg(fname);
				filemode = ERROR;
				break;
			}
			filesize += (off_t)n;
		} while (filesize < buf.st_size);
	}
	if (fd > 0) close(fd);
	if (filemode != REGULAR) {
		filesize = 0L;
	}
	if (fname != NULL) {
		switch (filemode) {
		case NEW:
			sprintf(string, "\"%s\" [New File]", fname);
			break;
		case REGULAR:
			sprintf(string, "\"%s\" %s%lu bytes", fname,
				P(P_RO) ? "[Read only] " : "", (long)filesize);
			break;
		case DIRECTORY:
			sprintf(string, "\"%s\" Directory", fname);
			break;
		}
		if (filemode != ERROR) msg(string);
	}
	pagepos = mem;
	maxpos = (PTR)(mem + filesize);
	loc = HEX;
	x = AnzAdd; y = 0;
	repaint();
	return(filesize);
}


int
addfile(char *fname)
{
	int			fd;
	off_t		oldsize;
	unsigned	chunk, n;

	if (stat(fname, &buf)) {
		sysemsg(fname);
		return 1;
	}
	if ((fd = open(fname, O_RDONLY)) == -1) {
		sysemsg(fname);
		return 1;
	}
	oldsize = filesize;
	if (enlarge(buf.st_size)) return 1;
	chunk = buf.st_size > 0xfffe ? 0xfffe : buf.st_size;
	do {
		if ((n = read(fd, mem + filesize, chunk)) == 0xffff) {
			sysemsg(fname);
			filemode = ERROR;
			return 1;
		}
		filesize += (off_t)n;
	} while (filesize < buf.st_size);
	maxpos = mem + filesize;
	close(fd);
	setpage(mem + oldsize);
	return 0;
}


void
bvi_init(char *dir)
{
	char	*poi;
	char	*initstr;
	char	rcpath[255];

	shell = getenv("COMSPEC");
	if (shell == NULL || *shell == '\0')
		shell = "COMMAND.COM";

	strcpy(rcpath, dir);
	poi = strrchr(rcpath, '\\');
	*poi = '\0';
	strcat(rcpath, "\\BVI.RC");

	if ((initstr = getenv("BVIINIT")) != NULL) {
			docmdline(initstr);
	}
	read_rc("BVI.RC");
	read_rc(rcpath);
}


int
enlarge(off_t add)
{
	PTR		newmem;
	off_t	savecur, savepag, savemax, saveundo;

	savecur = curpos - mem;
	savepag = pagepos - mem;
	savemax = maxpos - mem;
	saveundo = undo_start - mem;

	newmem = (PTR)farrealloc(mem, memsize + add);
	if (newmem == NULL) {
		emsg("Out of memory");
		return 1;
	}
	mem = newmem;
	memsize += add;
	curpos = mem + savecur;
	pagepos = mem + savepag;
	maxpos = mem + savemax;
	undo_start = mem + saveundo;
	current = curpos + 1;
	return 0;
}


void
do_shell()
{
	system("");
}


off_t
alloc_buf(off_t n, char **buffer)
{
	if ((*buffer = (char *)farrealloc(*buffer, n)) == NULL) {
        emsg("No buffer space available");
        return 0L;
    }
	return n;
}


void
d_memmove(PTR dest, PTR src, off_t n)
{
	unsigned	len;
	long		chunk;
	PTR			source;
	PTR			destin;

	chunk = n;
	if (dest < src) {
		/* copy forward */
		source = src;
		destin = dest;
		while (chunk > 0L) {
			len = chunk > 0x7ffe ? 0x7ffe : chunk;
			movmem(source, destin, len);
			chunk -= len;
			source += len;
			destin += len;
		}
	} else {
		/* copy backward */
		source = src + n;
		destin = dest + n;
		while (chunk > 0L) {
			len = chunk > 0x7ffe ? 0x7ffe : chunk;
			chunk -= len;
			source -= len;
			destin -= len;
			movmem(source, destin, len);
		}
	}
}

void
d_memcpy(PTR dest, PTR src, off_t n)
{
	unsigned	len;
	long		chunk;
	PTR			source;
	PTR			destin;

	source = src;
	destin = dest;
	chunk = n;
	while (chunk > 0L) {
		len = chunk > 0x7ffe ? 0x7ffe : chunk;
		movmem(source, destin, len);
		chunk -= len;
		source += len;
		destin += len;
	}
}
