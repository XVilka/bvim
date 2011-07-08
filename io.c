/* io.c - file in/out and alloc subroutines for BVI
 *
 * 1996-02-28 V 1.0.0
 * 1999-01-20 V 1.1.0
 * 1999-04-27 V 1.1.1
 * 1999-07-02 V 1.2.0 beta
 * 1999-10-15 V 1.2.0 final
 * 2000-03-23 V 1.3.0 beta
 * 2000-08-17 V 1.3.0 final
 * 2004-01-04 V 1.3.2
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

#include "bvi.h"
#include "set.h"

#include <limits.h>
#ifndef SIZE_T_MAX
#	define SIZE_T_MAX	ULONG_MAX
#endif

#ifdef HAVE_UNISTD_H
#	include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
#	include <fcntl.h>
#endif

int		filemode;
static	struct	stat	buf;
static	off_t	block_read;
char	*terminal;


/*********** Save the patched file ********************/
int
save(fname, start, end, flags)
	char	*fname;
	char	*start;
	char	*end;
	int		flags;
{
	int		fd;
	char	string[255];
	char	*newstr;
	off_t	filesize;

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
		} else if (S_ISCHR(buf.st_mode)) {
			sprintf(string, "\"%s\" Character special file", fname);
			msg(string);
			return 0;
		} else if (S_ISBLK(buf.st_mode)) {
			/*
			sprintf(string, "\"%s\" Block special file", fname);
			msg(string);
			return 0;
			*/
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
		sprintf(string, "\"%s\" %s%lu bytes", fname, newstr,
			(unsigned long)filesize);
	}

	if (write(fd, start, filesize) != filesize) {
		sysemsg(fname);
		close(fd);
		return 0;
	}
	close(fd);
	edits = 0;
	msg(string);
	return 1;
}


/* loads a file, returns the filesize */
off_t
load(fname)
	char	*fname;
{
	int		fd = -1;
	char	string[MAXCMD];

	buf.st_size = 0L;
	if (fname != NULL) {
		sprintf(string, "\"%s\"", fname);
		msg(string);
		refresh();
		if (stat(fname, &buf) == -1) {
			filemode = NEW;
		} else if (S_ISDIR(buf.st_mode)) {
			filemode = DIRECTORY;
		} else if (S_ISCHR(buf.st_mode)) {
			filemode = CHARACTER_SPECIAL;
		} else if (S_ISBLK(buf.st_mode)) {
			filemode = BLOCK_SPECIAL;
			if (!block_flag) {
				block_flag = 1;
				block_begin = 0;
				block_size = 1024;
				block_end = block_begin + block_size - 1;
			}
			if ((fd = open(fname, O_RDONLY)) > 0) {
				P(P_RO) = TRUE;
				params[P_RO].flags |= P_CHANGED;
			} else {
				sysemsg(fname);
				filemode = ERROR;
			}
		} else if (S_ISREG(buf.st_mode)) {
			if ((unsigned long)buf.st_size > (unsigned long)SIZE_T_MAX) {
				move(maxy, 0);
				endwin();
				printf("File too large\n");
				exit(0);
			}
			if ((fd = open(fname, O_RDONLY)) > 0) {
				filemode = REGULAR;
				if (access(fname, W_OK)) {
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
	if (mem != NULL) free(mem);
	memsize = 1024;
	if (block_flag) {
		memsize += block_size;
	} else if (filemode == REGULAR) {
		memsize += buf.st_size;
	}
	if ((mem = (char *)malloc(memsize)) == NULL) {
		move(maxy, 0);
		endwin();
		printf("Out of memory\n");
		exit(0);
	}
	clear_marks();


	if (block_flag && ((filemode == REGULAR) || (filemode == BLOCK_SPECIAL))) {
		if (lseek(fd, block_begin, SEEK_SET) < 0) {
			sysemsg(fname);
			filemode = ERROR;
		} else {
			if ((filesize = read(fd, mem, block_size)) == 0) {
				sprintf(string, "\"%s\" Empty file", fname);
				filemode = ERROR;
			} else {
				sprintf(string, "\"%s\" range %lu-%lu", fname,
					(unsigned long)block_begin,
					(unsigned long)(block_begin + filesize - 1));
				filemode = PARTIAL;
				block_read = filesize;
				P(P_OF) = block_begin;
				params[P_OF].flags |= P_CHANGED;
			}
			msg(string);
			refresh();
		}
	} else if (filemode == REGULAR) {
		filesize = buf.st_size;
		if (read(fd, mem, filesize) != filesize) {
            sysemsg(fname);
			filemode = ERROR;
		}
	} else {
		filesize = 0L;
	}
	if (fd > 0) close(fd);
	if (fname != NULL) {
		switch (filemode) {
		case NEW:
			sprintf(string, "\"%s\" [New File]", fname);
			break;
		case REGULAR:
			sprintf(string, "\"%s\" %s%lu bytes", fname,
				P(P_RO) ? "[Read only] " : "",
				(unsigned long)filesize);
			break;
		case DIRECTORY:
			sprintf(string, "\"%s\" Directory", fname);
			break;
		case CHARACTER_SPECIAL:
			sprintf(string, "\"%s\" Character special file", fname);
			break;
		case BLOCK_SPECIAL:
			sprintf(string, "\"%s\" Block special file", fname);
			break;
		}
		if (filemode != ERROR) msg(string);
	}
	pagepos = mem;
	maxpos = mem + filesize;
	loc = HEX;
	x = AnzAdd; y = 0;
	repaint();
	return(filesize);
}


/* argument "dir" not used! 
 * Needed for DOS version only
 */
void
bvi_init(dir)
	char *dir;
{
	char    *initstr;
	char    rcpath[MAXCMD];

	terminal = getenv("TERM");
	shell = getenv("SHELL");
	if (shell == NULL || *shell == '\0')
		shell = "/bin/sh";

	if ((initstr = getenv("BVIINIT")) != NULL) {
		docmdline(initstr);
		return;
	}
	
#ifdef DJGPP
	strcpy(rcpath, "c:");
	strcpy(rcpath, dir);
	poi = strrchr(rcpath, '\\');
	*poi = '\0';
	strcat(rcpath, "\\BVI.RC");
	read_rc(rcpath);
	read_rc("BVI.RC");
#else
	strncpy(rcpath, getenv("HOME"), MAXCMD - 8);
	rcpath[MAXCMD - 8] = '\0';
	strcat(rcpath, "/.bvirc");
	if (stat(rcpath, &buf) == 0) {
		if (buf.st_uid == getuid()) read_rc(rcpath);
	}

	strcpy(rcpath, ".bvirc");
	if (stat(rcpath, &buf) == 0) {
		if (buf.st_uid == getuid())	read_rc(rcpath);
	}
#endif
}


int
enlarge(add)
	off_t	add;
{
	char	*newmem;
	off_t	savecur, savepag, savemax, saveundo;

	savecur = curpos - mem;
	savepag = pagepos - mem;
	savemax = maxpos - mem;
	saveundo = undo_start - mem;

	if (mem == NULL) {
		newmem = malloc(memsize + add);
	} else {
		newmem = realloc(mem, memsize + add);
	}
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
	current = curpos + 1L;
	return 0;
}


void
do_shell()
{
	addch('\n');
	savetty();
#ifdef DJGPP
	system("");
#else
	system(shell);
#endif
	resetty();
}


#ifndef HAVE_STRDUP
char *
strdup(s)
	char	*s;
{
	char    *p;
	size_t	 n;

	n = strlen(s) + 1;
	if ((p = (char *)malloc(n)) != NULL)
		memcpy(p, s, n);
	return (p);
}
#endif


#ifndef HAVE_MEMMOVE
/*
 * Copy contents of memory (with possible overlapping).
 */
char *
memmove(s1, s2, n)
	char	*s1;
	char	*s2;
	size_t	n;
{
	bcopy(s2, s1, n);
	return(s1);
}
#endif


off_t
alloc_buf(n, buffer)
	off_t	n;
	char	**buffer;
{
	if (*buffer == NULL) {
		*buffer = (char *)malloc(n);
	} else {
		*buffer = (char *)realloc(*buffer, n);
	}
	if (*buffer == NULL) {
		emsg("No buffer space available");
		return 0L;
	}
	return n;
}


int
addfile(fname)
	char	*fname;
{
	int		fd;
	off_t	oldsize;

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
	if (read(fd, mem + filesize, buf.st_size) == -1) {
		sysemsg(fname);
		return 1;
	}
	filesize += buf.st_size;
	maxpos = mem + filesize;
	close(fd);
	setpage(mem + oldsize);
	return 0;
}
