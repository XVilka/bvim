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

#include "bvim.h"
#include "blocks.h"
#include "commands.h"
#include "set.h"
#include "ui.h"

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

//extern core_t core;
//extern state_t state;

int filemode;
static struct stat sbuf;
static off_t block_read;
char *terminal;

/* Save the patched file */
int save(core_t* core, buf_t* buf, char* fname, char* start, char* end, int flags)
{
	int fd;
	char string[255];
	char *newstr;
	off_t filesize;

	if (!fname) {
		ui__ErrorMsg("No file|No current filename");
		return 0;
	}
	if (stat(fname, &sbuf) == -1) {
		newstr = "[New file] ";
	} else {
		if (S_ISDIR(sbuf.st_mode)) {
			sprintf(string, "\"%s\" Is a directory", fname);
			ui__StatusMsg(string);
			return 0;
		} else if (S_ISCHR(sbuf.st_mode)) {
			sprintf(string, "\"%s\" Character special file", fname);
			ui__StatusMsg(string);
			return 0;
		} else if (S_ISBLK(sbuf.st_mode)) {
			/*
			   sprintf(string, "\"%s\" Block special file", fname);
			   msg(string);
			   return 0;
			 */
		}
		newstr = "";
	}

	if (filemode == PARTIAL)
		flags = O_RDWR;
	if ((fd = open(fname, flags, 0666)) < 0) {
		ui__SystemErrorMsg(fname);
		return 0;
	}
	if (filemode == PARTIAL) {
		if (block_read) {
			filesize = block_read;
			sprintf(string, "\"%s\" range %lu-%lu", fname,
				(unsigned long)block_begin,
				(unsigned long)(block_begin - 1 + filesize));
			if (lseek(fd, block_begin, SEEK_SET) < 0) {
				ui__SystemErrorMsg(fname);
				return 0;
			}
		} else {
			ui__StatusMsg("Null range");
			return 0;
		}
	} else {
		filesize = end - start + 1L;
		sprintf(string, "\"%s\" %s%lu bytes", fname, newstr, (unsigned long)filesize);
	}

	if (write(fd, start, filesize) != filesize) {
		ui__SystemErrorMsg(fname);
		close(fd);
		return 0;
	}
	close(fd);
	edits = 0;
	ui__StatusMsg(string);
	return 1;
}

/* loads a file, returns the filesize */
off_t load(core_t* core, buf_t* buf, char* fname)
{
	int fd = -1;
	char string[MAXCMD];

	sbuf.st_size = 0L;
	if (fname != NULL) {
		sprintf(string, "\"%s\"", fname);
		ui__StatusMsg(string);
		refresh();
		if (stat(fname, &sbuf) == -1) {
			filemode = NEW;
		} else if (S_ISDIR(sbuf.st_mode)) {
			filemode = DIRECTORY;
		} else if (S_ISCHR(sbuf.st_mode)) {
			filemode = CHARACTER_SPECIAL;
		} else if (S_ISBLK(sbuf.st_mode)) {
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
				ui__SystemErrorMsg(fname);
				filemode = ERROR;
			}
		} else if (S_ISREG(sbuf.st_mode)) {
			if ((unsigned long)sbuf.st_size >
			    (unsigned long)SIZE_T_MAX) {
				move(core->screen.maxy, 0);
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
				ui__SystemErrorMsg(fname);
				filemode = ERROR;
			}
		}
	} else {
		filemode = NEW;
	}
	if (buf->mem != NULL)
		free(buf->mem);
	memsize = 1024;
	if (block_flag) {
		memsize += block_size;
	} else if (filemode == REGULAR) {
		memsize += sbuf.st_size;
	}
	if ((buf->mem = (char *)malloc(memsize)) == NULL) {
		move(core->screen.maxy, 0);
		endwin();
		printf("Out of memory\n");
		exit(0);
	}
	clear_marks();

	if (block_flag
	    && ((filemode == REGULAR) || (filemode == BLOCK_SPECIAL))) {
		if (lseek(fd, block_begin, SEEK_SET) < 0) {
			ui__SystemErrorMsg(fname);
			filemode = ERROR;
		} else {
			if ((filesize = read(fd, buf->mem, block_size)) == 0) {
				sprintf(string, "\"%s\" Empty file", fname);
				filemode = ERROR;
			} else {
				sprintf(string, "\"%s\" range %lu-%lu", fname,
					(unsigned long)block_begin,
					(unsigned long)(block_begin + filesize -
							1));
				filemode = PARTIAL;
				block_read = filesize;
				P(P_OF) = block_begin;
				params[P_OF].flags |= P_CHANGED;
			}
			ui__StatusMsg(string);
			refresh();
		}
	} else if (filemode == REGULAR) {
		filesize = sbuf.st_size;
		if (read(fd, buf->mem, filesize) != filesize) {
			ui__SystemErrorMsg(fname);
			filemode = ERROR;
		}
	} else {
		filesize = 0L;
	}
	if (fd > 0)
		close(fd);
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
		if (filemode != ERROR)
			ui__StatusMsg(string);
	}
	buf->state.pagepos = buf->mem;
	buf->maxpos = buf->mem + filesize;
	buf->state.loc = HEX;
	x = core->params.COLUMNS_ADDRESS;
	y = 0;
	ui__Screen_Repaint();
	return (filesize);
}

/* argument "dir" not used! 
 * Needed for DOS version only
 */
void bvim_init(core_t* core, char* dir)
{
	char *initstr;
	char rcpath[MAXCMD];

	terminal = getenv("TERM");
	shell = getenv("SHELL");
	if (shell == NULL || *shell == '\0')
		shell = "/bin/sh";

	if ((initstr = getenv("BVIINIT")) != NULL) {
		docmdline(core, core->curbuf, initstr);
		return;
	}
	strncpy(rcpath, getenv("HOME"), MAXCMD - 8);
	rcpath[MAXCMD - 8] = '\0';
	strcat(rcpath, "/.bvimrc");
	if (stat(rcpath, &sbuf) == 0) {
		if (sbuf.st_uid == getuid())
			read_rc(core, rcpath);
	}

	strcpy(rcpath, ".bvimrc");
	if (stat(rcpath, &sbuf) == 0) {
		if (sbuf.st_uid == getuid())
			read_rc(core, rcpath);
	}

	strcpy(rcpath, ".bvimhistory");
	if (stat(rcpath, &sbuf) == 0) {
		if (sbuf.st_uid == getuid())
			read_history(core, rcpath);
	}

}

int enlarge(core_t* core, buf_t* buf, off_t add)
{
	char *newmem;
	off_t savecur, savepag, savemax, saveundo;

	savecur = buf->state.curpos - buf->mem;
	savepag = buf->state.pagepos - buf->mem;
	savemax = buf->maxpos - buf->mem;
	saveundo = undo_start - buf->mem;

	if (buf->mem == NULL) {
		newmem = malloc(memsize + add);
	} else {
		newmem = realloc(buf->mem, memsize + add);
	}
	if (newmem == NULL) {
		ui__ErrorMsg("Out of memory");
		return 1;
	}

	buf->mem = newmem;
	memsize += add;
	buf->state.curpos = buf->mem + savecur;
	buf->state.pagepos = buf->mem + savepag;
	buf->maxpos = buf->mem + savemax;
	undo_start = buf->mem + saveundo;
	buf->state.current = buf->state.curpos + 1L;
	return 0;
}

void do_shell()
{
	addch('\n');
	savetty();
	shresult = system(shell);
	ui__StatusMsg("shell executed successfully!");
	resetty();
}

off_t alloc_buf(off_t n, char** buffer)
{
	if (*buffer == NULL) {
		*buffer = (char *)malloc(n);
	} else {
		*buffer = (char *)realloc(*buffer, n);
	}
	if (*buffer == NULL) {
		ui__ErrorMsg("No buffer space available");
		return 0L;
	}
	return n;
}

int addfile(core_t* core, buf_t* buf, char* fname)
{
	int fd;
	off_t oldsize;

	if (stat(fname, &sbuf)) {
		ui__SystemErrorMsg(fname);
		return 1;
	}
	if ((fd = open(fname, O_RDONLY)) == -1) {
		ui__SystemErrorMsg(fname);
		return 1;
	}
	oldsize = filesize;
	if (enlarge(sbuf.st_size))
		return 1;
	if (read(fd, buf->mem + filesize, sbuf.st_size) == -1) {
		ui__SystemErrorMsg(fname);
		return 1;
	}
	filesize += sbuf.st_size;
	buf->maxpos = buf->mem + filesize;
	close(fd);
	setpage(buf->mem + oldsize);
	return 0;
}

#ifndef HAVE_STRDUP
char *strdup(char* s)
{
	char *p;
	size_t n;

	n = strlen(s) + 1;
	if ((p = (char *)malloc(n)) != NULL)
		memcpy(p, s, n);
	return (p);
}
#endif

#ifndef HAVE_MEMMOVE
char *memmove(char* s1, char* s2, size_t n)
{
	bcopy(s2, s1, n);
	return (s1);
}
#endif


