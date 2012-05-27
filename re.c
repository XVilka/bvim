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

/* You cannot use a common regexp subroutine, because \0 is a regular
 * character in a binary string !
 */

#include	"bvim.h"
#include	"blocks.h"
#include	"set.h"
#include	"ui.h"

/* Error and informational messages */
#include	"messages.h"

//extern core_t core;
//extern state_t state;

static int sbracket();

char act_pat[MAXCMD];		/* found pattern */
char pattern[MAXCMD + 1];
char search_pat[BUFFER];	/* / or ? command */

char *bvim_substr(const char* str, size_t begin, size_t len)
{
	if (str == 0 || strlen(str) == 0 || strlen(str) < begin
	    || strlen(str) < (begin + len))
		return 0;
	if (len == -1)
		len = strlen(str) - begin;
	return strndup(str + begin, len);
}

int bregexec(core_t *core, buf_t *buf, PTR start, char* scan)
{
	char *act;
	int count, test;

	act = act_pat;
	while (*scan != 0) {
		switch (*scan++) {
		case ONE:	/* exactly one character */
			count = *scan++;
			if (P(P_IC) && smode == ASCII)
				test = toupper(*start);
			else
				test = *start;
			if (count == 1) {
				if (test != *scan)
					return 0;
				scan++;
			} else if (count > 1) {
				if (sbracket(test, scan, count))
					return 0;
				scan += count;
			}
			*act++ = *start++;
			break;
		case STAR:	/* zero or more characters */
			count = *scan++;
			if (P(P_IC) && smode == ASCII)
				test = toupper(*start);
			else
				test = *start;
			if (count == 1) {	/* only one character, 0 - n times */
				while (test == *scan) {
					*act++ = *start++;
					if (P(P_IC) && smode == ASCII)
						test = toupper(*start);
					else
						test = *start;
				}
				scan++;
			} else if (count > 1) {	/* characters in bracket */
				if (*scan == '^') {
					while (start < buf->maxpos) {
						if (bregexec(core, buf, start, scan + count)) {
							*act = '\0';
							return 1;
						}
						if (sbracket(test, scan, count))
							return 0;
						*act++ = *start++;
						if (P(P_IC) && smode == ASCII)
							test = toupper(*start);
						else
							test = *start;
					}
				} else {
					while (!sbracket(test, scan, count)) {
						*act++ = *start++;
						if (P(P_IC) && smode == ASCII)
							test = toupper(*start);
						else
							test = *start;
					}
					scan += count;
				}
			} else {	/* ".*"  */
				while (start < buf->maxpos) {
					if (bregexec(core, buf, start, scan)) {
						*act = '\0';
						return 1;
					}
					start++;
				}
			}
		}
	}
	*act = '\0';
	return 1;		/* found */
}

static int sbracket(int start, char* scan, int count)
{
	if (*scan++ == '^') {
		if (!memchr(scan, start, --count))
			return 0;
	} else {
		if (memchr(scan, start, --count))
			return 0;
	}
	return 1;
}

PTR end_word(core_t *core, buf_t *buf, PTR start)
{
	PTR pos;

	pos = start;
	if (!isprint(*pos & 0xff))
		return start;
	while (isprint(*pos & 0xff))
		if (pos++ > buf->maxpos)
			return start;
	return --pos;
}

/* wordsearch serves the 'W' and 'w' - command
 */
PTR wordsearch(core_t *core, buf_t *buf, PTR start, char mode)
{
	PTR found;
	PTR pos;
	int ccount;

	pos = start + 1;
	do {
		while (isprint(*pos & 0xff))
			if (pos++ > buf->maxpos)
				return start;
		while (!isprint(*pos & 0xff))
			if (pos++ > buf->maxpos)
				return start;
		found = pos;
		ccount = 0;
		while (isprint(*pos & 0xff)) {
			if (pos++ > buf->maxpos)
				return start;
			ccount++;
		}
		if (ccount < P(P_WL))
			continue;

		if (mode == 'W') {
			if (*pos == '\0' || *pos == '\n')
				return found;
		} else {
			return found;
		}
	} while (pos < buf->maxpos);
	return start;
}

/* backsearch serves the 'b' and 'B' command
 */
PTR backsearch(core_t *core, buf_t *buf, PTR start, char mode)
{
	PTR pos;
	int ccount;

	pos = start - 1;
	do {
		if (mode == 'B') {
			while (*pos != '\0' && *pos != '\n')
				if (pos-- < buf->mem)
					return start;
		} else {
			while (!isprint(*pos & 0xff))
				if (pos-- < buf->mem)
					return start;
		}
		pos--;
		ccount = 0;
		while (isprint(*pos & 0xff)) {
			if (pos-- < buf->mem)
				return start;
			ccount++;
		}
		if (ccount >= P(P_WL))
			return (pos + 1);

	} while (pos > buf->mem);
	return start;
}

/* used by :s
 */
int do_substitution(core_t *core, buf_t *buf, int delim, char* line, PTR startpos, PTR endpos)
{
	int n;
	char *found;
	char *cmd = NULL;
	int repl_count = 0;
	int global = 0;
	int conf = 0;
	static int direct;
	static int pat_len = -1;
	static int ch;
	static char find_pat[BUFFER];
	static char repl_pat[MAXCMD];

	ignore_case = P(P_IC);
	magic = P(P_MA);
	switch (delim) {
	case '/':
	case '?':
		ch = delim;
		direct = (ch == '/' ? FORWARD : BACKWARD);
		cmd = patcpy(pattern, line, ch);
		if (pattern[0] == '\0')
			break;
		if (ascii_comp(find_pat, pattern))
			return 0;
		cmd = patcpy(pattern, cmd, ch);	/* Replace Pattern */
		poi = pattern;
		pat_len = 0;
		while (*poi) {
			if (*poi == '\\') {
				switch (*(++poi)) {
				case '0':
					repl_pat[pat_len++] = '\0';
					break;
				case 'n':
					repl_pat[pat_len++] = '\n';
					break;
				case 'r':
					repl_pat[pat_len++] = '\r';
					break;
				case 't':
					repl_pat[pat_len++] = '\t';
					break;
				case '\\':
					repl_pat[pat_len++] = '\\';
					break;
				default:
					sprintf(string, "No such escape sequence \\%c", *poi);
					bvim_error(core, buf, string);
					return 0;
				}
			} else {
				repl_pat[pat_len++] = *poi;
			}
			poi++;
		}
		break;
	case '#':
	case '\\':
		ch = delim;
		direct = (ch == '\\' ? FORWARD : BACKWARD);
		cmd = patcpy(pattern, line, ch);
		if (hex_comp(find_pat, pattern))
			return 0;
		cmd = patcpy(pattern, cmd, ch);	/* Replace Pattern */
		poi = pattern;
		pat_len = 0;
		while (*poi) {
			if (*poi == ' ' || *poi == '\t') {
				poi++;
			} else {
				if ((n = hexchar()) < 0) {
					bvim_error(core, buf, "Badly formed replacement pattern");
					return 0;
				}
				repl_pat[pat_len] = n;
				pat_len++;
			}
		}
		break;
	case '\0':
	case 'g':
	case 'c':
		break;
	default:
		bvim_error(core, buf, "Extra chars|Extra characters at end of command");
		return -1;
	}
	if (pat_len == -1) {
		bvim_error(core, buf, "No previous substitute re|No previous substitute to repeat");
		return -1;
	}
	if (delim != '\0') {
		if (strchr(cmd, 'g'))
			global = 1;
		if (strchr(cmd, 'c'))
			conf = 1;
	}
	if ((strchr("\\#", ch) && buf->state.loc == ASCII)
	    || (strchr("/?", ch) && buf->state.loc == HEX)) {
		toggle(core, buf);
	}
	startpos--;
	move(core->screen.maxy, 0);
	refresh();

	if (global) {
		if ((undo_count = alloc_buf(core, buf, endpos - startpos, &undo_buf))) {
			memcpy(undo_buf, startpos + 1, undo_count);
		}
		undo_start = startpos + 1;
		edits = U_EDIT;
	}

      AGAIN:
	if (direct == FORWARD) {
		found = fsearch(core, buf, startpos + 1, endpos, find_pat);
	} else {
		found = rsearch(core, buf, startpos - 1, buf->mem, find_pat);
	}
	if (!found) {
		if (!repl_count) {
			if (P(P_WS)) {
				bvim_error(core, buf, BVI_ERROR_PATNOTFOUND);
			} else {
				if (P(P_TE))
					sprintf(string, "No match to %s", direct ==	FORWARD ? "BOTTOM" : "TOP");
				else
					sprintf(string, "Address search hit %s without matching pattern", direct ==	FORWARD ? "BOTTOM" : "TOP");
				bvim_error(core, buf, string);
			}
		}
		return repl_count;
	} else {
		setpage(core, buf, found);
		if (conf) {
			ui__Screen_Repaint(core, buf);
			bvim_info(core, buf, "Replace?");
			move(y, x);
			if (vgetc() != 'y')
				goto SKIP;
		}
		repl_count++;
		current_start = buf->state.pagepos + y * core->params.COLUMNS_DATA + xpos(core, buf);
		if (!global) {
			if ((undo_count = alloc_buf(core, buf, pat_len, &undo_buf))) {
				memcpy(undo_buf, current_start, undo_count);
			}
			undo_start = current_start;
			edits = U_EDIT;
		}
		memcpy(current_start, repl_pat, pat_len);
	      SKIP:
		if (global) {
			startpos = found + pat_len - 1;
			goto AGAIN;
		}
	}
	return repl_count;
}

/*
 * Used by /, ?, \, #, n or N command
 * ch		is this command
 * line		are the characters after
 *
 * return	address found
 */
PTR searching(core_t *core, buf_t *buf, int ch, char* line, PTR startpos, PTR endpos, int flag)
{
	char *cmd = NULL;
	PTR found;
	int sdir;
	static char m[2];
	static int direct;

	if (line[0] == '\0' && again == 0) {
		bvim_error(core, buf, BVI_ERROR_NOPREVEXPR);
		return 0L;
	}

	ignore_case = (P(P_IC));
	magic = P(P_MA);
	start_addr--;
	if ((strchr("\\#", ch) && buf->state.loc == ASCII)
	    || (strchr("/?", ch) &&  buf->state.loc == HEX)) {
		toggle(core, buf);
	}
	if (!strchr("Nn", ch)) {
		m[0] = ch;
		m[1] = '\0';
		switch (ch) {
		case '/':
		case '?':
			direct = (ch == '/' ? FORWARD : BACKWARD);
			cmd = patcpy(pattern, line, ch);
			if (pattern[0] != '\0') {
				if (ascii_comp(search_pat, pattern))
					return 0L;
				again = 1;
			}
			break;
		case '#':
		case '\\':
			direct = (ch == '\\' ? FORWARD : BACKWARD);
			cmd = patcpy(pattern, line, ch);
			if (pattern[0] != '\0') {
				if (hex_comp(search_pat, pattern))
					break;
				again = 1;
			}
			break;
		}
		if (!again)
			return 0L;
	} else {
		cmd = "";
		bvim_info(core, buf, m);
	}
	move(core->screen.maxy, 0);
	refresh();
	sdir = (ch == 'N') ? !direct : direct;

	if (sdir == FORWARD) {
		found = fsearch(core, buf, startpos + 1, endpos, search_pat);
		if (flag & S_GLOBAL)
			return (found);
		if (!found)
			if (flag & 1) {
				bvim_info(core, buf, "Search wrapped BOTTOM|Search wrapped around BOTTOM of buffer");
				found = fsearch(core, buf, buf->mem, startpos, search_pat);
			}
	} else {
		found = rsearch(core, buf, startpos - 1, buf->mem, search_pat);
		if (flag & S_GLOBAL)
			return (found);
		if (!found)
			if (flag & 1) {
				bvim_info(core, buf, "Search wrapped TOP|Search wrapped around TOP of buffer");
				found = rsearch(core, buf, endpos, startpos, search_pat);
			}
	}
	if (!found) {
		if (flag & 1) {
			bvim_error(core, buf, BVI_ERROR_PATNOTFOUND);
		} else {
			if (P(P_TE)) {
				sprintf(string, "No match to %s", sdir == FORWARD ? "BOTTOM" : "TOP");
			} else {
				sprintf(string, "Address search hit %s without matching pattern", sdir == FORWARD ? "BOTTOM" : "TOP");
			}
			bvim_error(core, buf, string);
		}
	} else {
		setpage(core, buf, found);
		if (cmd) {
			switch (*cmd) {
			case 'z':
				do_z(core, buf, *(++cmd));
				break;
			case 's':
				do_substitution(core, buf, ch, cmd + 2, found, endpos);
				ui__Screen_Repaint(core, buf);
				break;
			case ';':
				searching(core, buf, *(cmd + 1), cmd + 2, found, buf->maxpos - 1, flag);
			case '\0':
				break;
			default:
				beep();
			}
		}
	}
	return found;
}

/* Copies a string from s2 to s1, up to delim or 0
 * returns pointer to next character
 */
char *patcpy(char *s1, char* s2, char delim)
{
	while (*s2 != '\0' && *s2 != delim) {
		if (*s2 == '\\' && *(s2 + 1) == delim)
			s2++;
		*s1++ = *s2++;
	}
	*s1 = '\0';
	if (*s2 == delim)
		s2++;
	return s2;
}

PTR fsearch(core_t* core, buf_t* buf, PTR start, PTR end, char* smem)
{
	PTR spos;

	signal(SIGINT, jmpproc);
	for (spos = start; spos <= end; spos++) {
		if (bregexec(core, buf, spos, smem)) {
			signal(SIGINT, SIG_IGN);
			return (spos);
		}
	}
	signal(SIGINT, SIG_IGN);
	return (NULL);
}

PTR rsearch(core_t* core, buf_t* buf, PTR start, PTR end, char *smem)
{
	PTR spos;

	signal(SIGINT, jmpproc);
	for (spos = start; spos >= end; spos--) {
		if (bregexec(core, buf, spos, smem)) {
			signal(SIGINT, SIG_IGN);
			return (spos);
		}
	}
	signal(SIGINT, SIG_IGN);
	return (NULL);
}

/* Calculates an address of a colon command
 * returns NULL on error or default_address, if nothing found
 */
PTR calc_addr(core_t* core, buf_t* buf, char** pointer, PTR def_addr)
{
	PTR addr;
	int ch, mark;
	char *cmd;

	cmd = *pointer;
	addr = def_addr;
	SKIP_WHITE if (*cmd >= '1' && *cmd <= '9') {
		addr = buf->mem + strtol(cmd, &cmd, 10) - P(P_OF);
	} else {
		ch = *cmd;
		switch (ch) {
		case '.':	/* Current position */
			addr = buf->state.current;
			cmd++;
			break;
		case '^':
			addr = buf->mem;
			cmd++;
			break;
		case '$':
			addr = buf->maxpos - 1;
			cmd++;
			break;
		case '\'':	/* Mark */
			mark = (*++cmd);
			if (mark == '\'') {
				addr = last_motion;
				cmd++;
				break;
			} else if (mark < 'a' || mark > 'z') {
				bvim_error(core, buf, "Marks are ' and a-z");
				return NULL;
			}
			if (markbuf[mark - 'a'] == NULL) {
				bvim_error(core, buf, "Mark not defined");
				return NULL;
			}
			addr = markbuf[mark - 'a'];
			cmd++;
			break;
		case '\\':
		case '/':
			cmd = patcpy(pattern, cmd + 1, ch);
			if (pattern[0] == '\0' && again == 0) {
				bvim_error(core, buf, BVI_ERROR_NOPREVEXPR);
				return NULL;
			}
			if (pattern[0] != '\0') {
				again = 1;
				if (ch == '/') {
					if (ascii_comp(search_pat, pattern))
						return NULL;
				} else {
					if (hex_comp(search_pat, pattern))
						return NULL;
				}
			}
			addr = fsearch(core, buf, buf->mem, buf->maxpos - 1, search_pat);
			break;
		case '#':
		case '?':
			cmd = patcpy(pattern, cmd + 1, ch);
			if (pattern[0] == '\0' && again == 0) {
				bvim_error(core, buf, BVI_ERROR_NOPREVEXPR);
				return NULL;
			}
			if (pattern[0] != '\0') {
				again = 1;
				if (ch == '?') {
					if (ascii_comp(search_pat, pattern))
						return NULL;
				} else {
					if (hex_comp(search_pat, pattern))
						return NULL;
				}
			}
			addr = rsearch(core, buf, buf->maxpos - 1, buf->mem, search_pat);
			break;
		case '0':
			addr = buf->mem + strtol(cmd, &cmd, 16) - P(P_OF);
			break;
		}
	}
	SKIP_WHITE while (*cmd == '+' || *cmd == '-') {
		if (*cmd == '+') {
			cmd++;
			SKIP_WHITE if (*cmd >= '1' && *cmd <= '9') {
				addr += strtol(cmd, &cmd, 10);
			} else if (*cmd == '0') {
				addr += strtol(cmd, &cmd, 16);
			}
		} else {
			cmd++;
			SKIP_WHITE if (*cmd >= '1' && *cmd <= '9') {
				addr -= strtol(cmd, &cmd, 10);
			} else if (*cmd == '0') {
				addr -= strtol(cmd, &cmd, 16);
			}
		}
	SKIP_WHITE}
	if (*pointer != cmd) {
		*pointer = cmd;
		addr_flag++;
	}
	return addr;
}

