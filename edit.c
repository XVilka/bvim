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

#include    "bvim.h"
#include	"blocks.h"
#include	"keys.h"
#include	"commands.h"
#include    "set.h"
#include	"ui.h"

extern int precount;
extern core_t core;
extern state_t state;

// TODO: Store all edits in the tree (in linked list, for simple cases
// TODO: Implement simple versioning system

char contrd[][4] = { "NUL", " ^A", " ^B", " ^C", " ^D", " ^E", " ^F", "BEL",
	" BS", "TAB", " NL", "HOM", "CLR", " CR", " ^N", " ^O",
	" ^P", " ^Q", " ^R", " ^S", " ^T", " ^U", " ^V", " ^W",
	" ^X", " ^Y", " ^Z", "ESC", " FS", " GS", " RS", " US",
	"DEL"
};

char contru[][4] = { "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
	" BS", " HT", " NL", " VT", " NP", " CR", " SO", " SI",
	"DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
	"CAN", " EM", "SUB", "ESC", " FS", " GS", " RS", " US",
	"DEL"
};

struct MARKERS_ markers[MARK_COUNT];

char tmpbuf[10];
char linbuf[256];

static char getcbuff[BUFFER];
static char *getcnext = NULL;

/* mode: ('A') append
 *       ('R') replace one or more different characters
 *       ('r') replace 1 character
 *       ('i') insert characters
 *       ('a') insert after cursor
 * a precount can be used
 *
 * for insert and append we misuse the undo buffer for the inserted
 * characters (for "." command)
 */
off_t edit(int mode)
{
	unsigned int ch, ch1;
	size_t len;
	off_t count = 0L;
	off_t buffer = BUFFER;
	off_t psize;

	if (!filesize && mode == 'i') {
		mode = 'A';
	}
	if (mode != 'A' && mode != 'a') {
		if (state.current - core.editor.mem >= filesize) {
			beep();
			return 0L;
		}
	}
	if (precount < 1)
		precount = 1;
	len = strlen(rep_buf);
	if (mode == 'r' && state.current + precount > core.editor.maxpos) {
		beep();
		rep_buf[len] = '\0';
		return 0L;
	}
	if (alloc_buf(buffer, &undo_buf) == 0L) {
		rep_buf[len] = '\0';
		return 0L;
	}
	switch (mode) {
	case 'A':
		edits = U_APPEND;
		break;
	case 'R':
		edits = U_EDIT;
		smsg("REPLACE MODE");
		break;
	case 'r':
		edits = U_EDIT;
		smsg("REPLACE 1 CHAR");
		break;
	case 'a':
	case 'i':
		edits = U_INSERT;
		smsg("INSERT MODE");
		break;
	}

	undo_start = state.current;

	while ((ch = vgetc()) != ESC) {
		ch &= 0xff;
		rep_buf[len++] = ch;
		if (ch == '\t') {
			toggle();
			setcur();
			continue;
		}
		if (ch == KEY_BACKSPACE || ch == BVI_CTRL('H')) {
			if (count > 0) {
				len--;
				count--;
				if (mode == 'A' || mode == 'a' || mode == 'i') {
					filesize--;
					core.editor.maxpos--;
				}
				state.current--;
				cur_back();
				setcur();
			} else
				beep();
			continue;
		}
		if (state.loc == HEX) {
			if (isxdigit(ch)) {
				mvaddch(y, x + 1, ' ');
				mvaddch(y, x, ch);
				do {
					ch1 = vgetc() & 0xff;
					if (ch1 == ESC) {
						mvaddch(y, x, ' ');
						state.current--;
						cur_back();
						goto escape;
					}
					if (!isxdigit(ch1)) {
						beep();
						ch1 = -1;
					}
				} while (ch1 == -1);
				rep_buf[len++] = ch1;
				mvaddch(y, x + 1, ch1);
				sprintf(tmpbuf, "%c%c", ch, ch1);
				sscanf(tmpbuf, "%2x", &ch);
			} else {
				beep();
				len--;
				goto wrong;
			}
		} else {			/*** ASCII - Bereich ***/
			if (isprint(ch)) {
				mvaddch(y, x, ch);
			} else {
				beep();
				goto wrong;
			}
		}
		core.editor.curpos = state.current++;
		if (mode == 'i' || mode == 'a') {
			memmove(state.current, core.editor.curpos, core.editor.maxpos - core.editor.curpos);
		}
		if (mode == 'A' || mode == 'i' || mode == 'a') {
			core.editor.maxpos++;
			filesize++;
			/* NEU
			   undo_buf[count++] = ch;
			 */
			count++;
		} else {
			undo_buf[count++] = *core.editor.curpos;
		}
		if (count == buffer) {
			buffer += BUFFER;
			if (alloc_buf(buffer, &undo_buf) == 0L) {
				rep_buf[len] = '\0';
				return count;
			}
		}

		*core.editor.curpos = (char)ch;
		cur_forw(0);
		statpos();
		if (mode == 'i' || mode == 'a') {
			ui__Screen_Repaint();
		} else {
			ui__lineout();
		}
		setcur();

		if (filesize > memsize - 2L) {
			if (enlarge(100L))
				break;
		}

		if ((mode != 'A' && mode != 'a') && core.editor.curpos == core.editor.maxpos - 1)
			break;
		if (mode == 'r') {
			break;
		}
	      wrong:
		continue;
	}
	rep_buf[len++] = ESC;
	rep_buf[len] = '\0';
	if (!count)
		goto escape;

	if (precount > 1) {
		switch (mode) {
		case 'i':
		case 'a':
		case 'A':
			psize = count * (precount - 1);
			if (filesize + psize > memsize - 2L) {
				if (enlarge(psize + 100L))
					return count;
			}
			if (psize + count > buffer) {
				if (alloc_buf(psize + count, &undo_buf) == 0L)
					return count;
			}

			if (mode == 'i' || mode == 'a') {
				memmove(state.current + psize, state.current,
					core.editor.maxpos - core.editor.curpos);
			}

			/* NEU
			   undo_pos = undo_buf + count - 1L;
			 */
			while (--precount) {
				/* NEU
				   memcpy(undo_pos + 1L, undo_pos - count + 1L, count);
				   undo_pos += count;
				 */
				memcpy(core.editor.curpos + 1L, core.editor.curpos - count + 1L, count);
				core.editor.curpos += count;
			}
			filesize += psize;
			count += psize;
			core.editor.maxpos += psize;
			undo_count += psize;
			state.current = core.editor.curpos + 1L;
			setpage(state.current);
			ui__Screen_Repaint();
			break;
		case 'R':
			if (state.current + count * (precount - 1) > core.editor.maxpos)
				break;
			psize = count;
			while (--precount) {
				memcpy(undo_buf + psize, core.editor.curpos + 1L, count);
				psize += count;
				memcpy(core.editor.curpos + 1L, core.editor.curpos - count + 1L, count);
				core.editor.curpos += count;
			}
			count = psize;
			setpage(++core.editor.curpos);
			ui__Screen_Repaint();
			break;
		case 'r':
			while (--precount) {
				undo_buf[count++] = *(++core.editor.curpos);
				*core.editor.curpos = (char)ch;
				cur_forw(0);
				statpos();
				ui__lineout();
			}
			break;
		}
	}
	cur_back();
      escape:
	setcur();
	smsg("");
	return (count);
}

/* Do the f, F, t ot T command
 * If flag == 1 save the character in rep_buf
 * else setpage()
 */
PTR do_ft(core_t *core, buf_t *buf, int ch, int flag)
{
	static int chi;
	static int chp = 1;
	int dir;
	size_t n;
	PTR ptr;

	switch (ch) {
	case 1:
		beep();
		return NULL;	/* no previous command */
	case -1:
		if (chp == 'f' || chp == 't')
			dir = BACKWARD;
		else
			dir = FORWARD;
		break;		/* same again */
	case 0:
		if (chp == 'f' || chp == 't')
			dir = FORWARD;
		else
			dir = BACKWARD;
		break;		/* same again */
	default:
		chp = ch;
		if (chp == 'f' || chp == 't')
			dir = FORWARD;
		else
			dir = BACKWARD;
		chi = vgetc();
		if (flag) {
			n = strlen(rep_buf);
			rep_buf[n++] = chi;
			rep_buf[n] = '\0';
		}
	}
	ptr = buf->state.current;
	do {
		if (dir == FORWARD) {
			do {
				ptr++;
				if (ptr > buf->maxpos)
					break;
			} while (*ptr != chi);
			if (ptr > buf->maxpos)
				break;
		} else {
			do {
				ptr--;
				if (ptr < buf->mem)
					break;
			} while (*ptr != chi);
			if (ptr < buf->mem)
				break;
		}
	} while (--precount > 0);
	if (*ptr == chi) {
		if (buf->state.loc == HEX)
			toggle();
		if (chp == 't')
			ptr--;
		if (chp == 'T')
			ptr++;
		if (!flag) {
			setpage(ptr);
		}
		return (ptr);
	}
	beep();
	return NULL;
}

void do_z(core_t *core, buf_t *buf, int mode)
{
	switch (mode) {
	case '.':
		while (y != core->screen.maxy / 2) {
			if (y > core->screen.maxy / 2) {
				buf->state.pagepos += core->params.COLUMNS_DATA;
				y--;
			} else {
				if (buf->state.pagepos == buf->mem)
					break;
				buf->state.pagepos -= core->params.COLUMNS_DATA;
				y++;
			}
		}
		break;
	case '-':
		while (y < core->screen.maxy - 1) {
			if (buf->state.pagepos == buf->mem)
				break;
			buf->state.pagepos -= core->params.COLUMNS_DATA;
			y++;
		}
		break;
	case '\0':
	case '\n':
	case '\r':
		while (y > 0) {
			y--;
			buf->state.pagepos += core->params.COLUMNS_DATA;
		}
		break;
	default:
		beep();
		break;
	}
	ui__Screen_Repaint();
}

/* Scroll down on <count> lines */
void scrolldown(core_t *core, buf_t *buf, int lines)
{
	while (lines--) {
		if (buf->maxpos >= (buf->state.pagepos + core->params.COLUMNS_DATA))
			buf->state.pagepos += core->params.COLUMNS_DATA;
		else {
			beep();
			lines = 0;
		}
		ui__Screen_Repaint();
		refresh();
	}
}

/* Scroll up on <count> lines */
void scrollup(core_t *core, buf_t *buf, int lines)
{
	while (lines--) {
		if (buf->mem <= (PTR) (buf->state.pagepos - core->params.COLUMNS_DATA))
			buf->state.pagepos -= core->params.COLUMNS_DATA;
		else {
			beep();
			lines = 0;
		}
		ui__Screen_Repaint();
		refresh();
	}
}

/* return position from screen to byte offset */
int xpos(core_t *core, buf_t *buf)
{
	if (buf->state.loc == HEX)
		return ((x - core->params.COLUMNS_ADDRESS) / 3);
	else
		return (x - core->params.COLUMNS_ADDRESS -
			core->params.COLUMNS_HEX);
}

int get_cursor_position(core_t *core, buf_t *buf)
{
	return (buf->state.current - core->editor.mem);
}

/* toggle between ASCII and HEX windows positions */
void toggle(core_t *core, buf_t *buf)
{
	if (buf->state.loc == HEX) {
		x = xpos(core, buf) + core->params.COLUMNS_ADDRESS +
		    core->params.COLUMNS_HEX;
		buf->state.loc = ASCII;
	} else {
		x = xpos(core, buf) * 3 + core->params.COLUMNS_ADDRESS;
		buf->state.loc = HEX;
	}
}

void setcur()
{
	move(y, x);
	refresh();
}

/* ------------------------------------------------
 * display current position 
 * ------------------------------------------------ */

void statpos(core_t *core, buf_t *buf)
{
	unsigned char Char1;
	off_t bytepos;
	char string[30], str[6];

	if (!P(P_MO))
		return;
	bytepos = buf->state.current - buf->mem;
	if (bytepos >= filesize) {
		mvaddstr(core->screen.maxy, status,
			 "                           ");
		return;
	}

	/* Display char, if printable */
	Char1 = *(buf->mem + bytepos);

	if (isprint(Char1)) {
		sprintf(str, "'%c'", Char1);
	} else if (Char1 < 32) {
		if (P(P_US))
			strcpy(str, contru[Char1]);
		else
			strcpy(str, contrd[Char1]);
	} else if (Char1 == 127) {
		if (P(P_US))
			strcpy(str, contru[32]);
		else
			strcpy(str, contrd[32]);
	} else
		strcpy(str, "   ");

	sprintf(string, "%08lX  \\%03o 0x%02X %3d %3s",
		(long)(bytepos + P(P_OF)), Char1, Char1, Char1, str);
	
	// ncurses ! Split this out !
	attrset(A_BOLD);
	mvaddstr(core->screen.maxy, status, string);
	attrset(A_NORMAL);
}

/* -----------------------------------------------------
 * display an arbitrary address on screen 
 * ----------------------------------------------------- */

void setpage(core_t *core, buf_t *buf, PTR addr)
{
	if ((addr >= buf->state.pagepos) && ((addr - buf->state.pagepos) < buf->state.screen)) {
		y = (addr - buf->state.pagepos) / core->params.COLUMNS_DATA;
		if (buf->state.loc == HEX)
			x = core->params.COLUMNS_ADDRESS +
			    ((addr - buf->state.pagepos) -
			     y * core->params.COLUMNS_DATA) * 3;
		else
			x = core->params.COLUMNS_ADDRESS +
			    core->params.COLUMNS_HEX + ((addr - buf->state.pagepos) -
						       y * core->params.COLUMNS_DATA);
	} else {
		buf->state.pagepos =
		    (((addr -
		       buf->mem) / core->params.COLUMNS_DATA) *
		     core->params.COLUMNS_DATA + buf->mem)
		    - (core->params.COLUMNS_DATA * (core->screen.maxy / 2));
		if (buf->state.pagepos < buf->mem)
			buf->state.pagepos = buf->mem;
		y = (addr - buf->state.pagepos) / core->params.COLUMNS_DATA;
		if (buf->state.loc == HEX)
			x = core->params.COLUMNS_ADDRESS +
			    ((addr - buf->state.pagepos) -
			     y * core->params.COLUMNS_DATA) * 3;
		else
			x = core->params.COLUMNS_ADDRESS +
			    core->params.COLUMNS_HEX + ((addr - buf->state.pagepos) -
						       y * core->params.COLUMNS_DATA);
		ui__Screen_Repaint();
	}
}

int cur_forw(core_t *core, buf_t *buf, int check)
{
	if (check) {
		if (buf->state.current - buf->mem >= filesize) {
			beep();
			return 1;
		}
	}
	if (buf->state.loc == ASCII) {
		if (x <
		    core->params.COLUMNS_ADDRESS - 1 + core->params.COLUMNS_HEX +
		    core->params.COLUMNS_DATA) {
			x++;
			return 0;
		} else
			x = core->params.COLUMNS_ADDRESS +
			    core->params.COLUMNS_HEX;
	} else {
		if (x < 5 + core->params.COLUMNS_HEX) {
			x += 3;
			return 0;
		} else
			x = core->params.COLUMNS_ADDRESS;
	}
	statpos();
	ui__lineout();
	if (y < core->screen.maxy - 1) {
		y++;
		return 0;
	} else {
		if (buf->state.pagepos < (PTR) (buf->mem + filesize)) {
			buf->state.pagepos += core->params.COLUMNS_DATA;
			ui__Screen_Repaint();
			return 0;
		} else {
			beep();
			return 1;
		}
	}
}

int cur_back(core_t *core, buf_t *buf)
{
	if (buf->state.loc == ASCII) {
		if (x > core->params.COLUMNS_ADDRESS + core->params.COLUMNS_HEX) {
			x--;
			return 0;
		} else {
			x = core->params.COLUMNS_ADDRESS - 1 +
			    core->params.COLUMNS_HEX + core->params.COLUMNS_DATA;
		}
	} else {
		if (x > core->params.COLUMNS_ADDRESS + 2) {
			x -= 3;
			return 0;
		} else {
			if (buf->state.current == buf->mem)
				return 0;
			x = core->params.COLUMNS_ADDRESS +
			    core->params.COLUMNS_HEX - 3;
		}
	}
	statpos();
	ui__lineout();
	if (y > 0) {
		y--;
		return 0;
	} else {
		if (buf->state.pagepos > buf->mem) {
			buf->state.pagepos -= core->params.COLUMNS_DATA;
			ui__Screen_Repaint();
			return 0;
		} else {
			beep();
			return 1;
		}
	}
}

void fileinfo(char* fname)
{
	off_t bytepos;
	char fstatus[64];

	if (fname) {
		sprintf(string, "\"%s\" ", fname);
	} else {
		strcpy(string, "No file ");
	}
	if (filemode != NEW && filemode != REGULAR)
		strcat(string, "[Not edited] ");
	if (P(P_RO))
		strcat(string, "[Read only] ");
	if (edits)
		strcat(string, "[Modified] ");
	if (filesize) {
		bytepos =
		    (state.pagepos + y * core.params.COLUMNS_DATA + xpos()) -
		    core.editor.mem + 1L;
		sprintf(fstatus, "byte %lu of %lu --%lu%%--", (long)bytepos,
			(long)filesize, (long)(bytepos * 100L / filesize));
		strcat(string, fstatus);
	} else {
		strcat(string, " 0 bytes");
	}
	ui__StatusMsg(string);
}

int vgetc()
{
	int nextc;

	if (getcnext != NULL) {
		nextc = *getcnext++;
		if (*getcnext == '\0') {
			*getcbuff = '\0';
			getcnext = NULL;
		}
		return (nextc);
	}
	return getch();
}

void stuffin(char* s)
{
	if (s == NULL) {	/* clear the stuff buffer */
		getcnext = NULL;
		return;
	}
	if (getcnext == NULL) {
		strcpy(getcbuff, s);
		getcnext = getcbuff;
	} else
		strcat(getcbuff, s);
}

void do_back(off_t n, PTR start)
{
	if (start - n < core.editor.mem) {
		beep();
		return;
	}
	if ((undo_count = alloc_buf(n, &undo_buf)) == 0L)
		return;
	yanked = alloc_buf(n, &yank_buf);
	edits = U_BACK;
	undo_start = start - n;
	memcpy(undo_buf, start - undo_count, undo_count);
	memcpy(yank_buf, start - undo_count, undo_count);
	memmove(start - undo_count, start, core.editor.maxpos - start);
	filesize -= undo_count;
	core.editor.maxpos -= undo_count;
	setpage(start - undo_count);
	ui__Screen_Repaint();
}

int do_delete(off_t n, PTR start)
{
	if (n + start > core.editor.maxpos) {
		beep();
		return 1;
	}
	if ((undo_count = alloc_buf(n, &undo_buf)) == 0L)
		return 1;
	yanked = alloc_buf(n, &yank_buf);
	edits = U_DELETE;
	undo_start = start;
	memcpy(undo_buf, start, undo_count);
	memcpy(yank_buf, start, undo_count);
	memmove(start, start + undo_count, core.editor.maxpos - (start + undo_count));
	filesize -= undo_count;
	core.editor.maxpos -= undo_count;
	if (start == core.editor.maxpos && start > core.editor.mem) {
		start--;
		cur_back();
	}
	setpage(start);
	ui__Screen_Repaint();
	return 0;
}

/*
 * The :insert, :append and :change command
 */
void do_ins_chg(PTR start, char* arg, int mode)
{
	int base;
	off_t buffer = BUFFER;
	off_t count = 0L;
	size_t len;
	long val;
	char *tempbuf = NULL;
	char *poi, *epoi;

	if ((mode == U_EDIT) && (state.current - core.editor.mem >= filesize)) {
		beep();
		return;
	}
	len = strlen(arg);
	if (!strncmp("ascii", arg, len) && CMDLNG(5, 1)) {
		base = 1;
	} else if (!strncmp("binary", arg, len) && CMDLNG(6, 1)) {
		base = 2;
	} else if (!strncmp("octal", arg, len) && CMDLNG(5, 1)) {
		base = 8;
	} else if (!strncmp("decimal", arg, len) && CMDLNG(7, 1)) {
		base = 10;
	} else if (!strncmp("hexadecimal", arg, len) && CMDLNG(11, 1)) {
		base = 16;
	} else {
		ui__ErrorMsg("No such option");
		return;
	}
	addch('\n');
	if (getcmdstr(cmdstr, 0) == 1) {
		ui__Screen_Repaint();
		return;
	}
	if (alloc_buf(buffer, &tempbuf) == 0L)
		return;
	while (strcmp(cmdstr, ".")) {
		poi = cmdstr;
		if (base == 1) {	/* ASCII */
			while (*poi != '\0') {
				if (*poi == '\\') {
					switch (*(++poi)) {
					case 'n':
						val = '\n';
						break;
					case 'r':
						val = '\r';
						break;
					case 't':
						val = '\t';
						break;
					case '0':
						val = '\0';
						break;
					case '\\':
						val = '\\';
						break;
					default:
						val = '\\';
						poi--;
					}
					poi++;
				} else {
					val = *poi++;
				}
				*(tempbuf + count++) = val;
			}
		} else {
			while (isspace(cmdstr[strlen(cmdstr) - 1]))
				cmdstr[strlen(cmdstr) - 1] = '\0';
			while (*poi != '\0') {
				val = strtol(poi, &epoi, base);
				if (val > 255 || val < 0 || poi == epoi) {
					ui__Screen_Repaint();
					ui__ErrorMsg("Invalid value");
					free(tempbuf);
					return;
				}
				poi = epoi;
				*(tempbuf + count++) = val;
			}
		}
		addch('\n');
		if (getcmdstr(cmdstr, 0) == 1) {
			ui__Screen_Repaint();
			free(tempbuf);
			return;
		}
	}
	if (count == 0) {
		ui__Screen_Repaint();
		free(tempbuf);
		return;
	}
	switch (mode) {
	case U_INSERT:
		do_put(start, count, tempbuf);
		break;
	case U_EDIT:
		do_over(start, count, tempbuf);
		break;
	case U_APPEND:
		if ((undo_count = alloc_buf(count, &undo_buf)) == 0L) {
			ui__Screen_Repaint();
			free(tempbuf);
			return;
		}
		do_append(count, tempbuf);
		memcpy(undo_buf, tempbuf, count);
		ui__Screen_Repaint();
		break;
	}
	free(tempbuf);
	return;
}

void clear_marks()
{
	int n;

	for (n = 0; n < 26; markbuf[n++] = NULL) ;
	undo_count = 0;
	last_motion = core.editor.mem;
}

void do_mark(int mark, PTR addr)
{
	if (mark < 'a' || mark > 'z' || state.current >= core.editor.maxpos)
		return;
	markbuf[mark - 'a'] = addr;
}

void movebyte()
{
	bvim_error(state.mode, "Command disabled@- use ':set memmove' to enable ");
}
