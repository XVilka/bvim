/* recomp - regular expression compiler
 *
 * NOTE: Edit this file with tabstop=4 !
 *
 * 1996-01-06 created;
 * 2000-04-25 V 1.3.0 beta
 * 2000-07-12 V 1.3.0 final
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

/* You cannot use a common regexp subroutine, because \0 is a regular
 * character in a binary string !
 */
 

#include	"bvi.h"
#include	"set.h"


char	*poi;
int		smode;
int		again = 0;
int		magic = 1;
int		ignore_case = 0;

extern	long	bytepos;
extern	int		ignore_case;
extern	char	*emptyclass;

/*
 *  Compiling an ASCII sequence to a regex string
 */
int
ascii_comp(smem, pattern)
	char	*smem;
	char	*pattern;
{
	char	*end;
	char	*comp;
	char	cc, cc1;
	char	*counter;
	int		count;
	int		bracket, dot;

	comp = smem;
	poi = pattern;
	while (*poi != END) {
		bracket = FALSE;
		if (magic) {
			if (*poi == '[') bracket = TRUE;
		} else {
			if (*poi == '\\' && *(poi + 1) == '[') {
				bracket = TRUE;
				poi++;
			}
		}
		if (bracket) {
			if (!(end = strchr(poi, ']'))) {
				emsg("Missing ]");
				return 1; }
			poi++;
			if (*poi == ']' || (*poi == '^' && *(poi + 1) == ']')) {
				emsg(emptyclass);
				return 1; }
			if (magic) {
				if (*(end + 1) == '*') *comp++ = STAR;
					else	*comp++ = ONE;
			} else {
				if (*(end + 1) == '\\' && *(end + 2) == '*') *comp++ = STAR;
					else	*comp++ = ONE;
			}
			count = 0;
			counter = comp;
			comp++;
			if (*poi != '^') {
				*comp++ = '\0';
				count++; }

			while (end > poi) {
				if (*poi == '-') {
					if (ignore_case) {
						cc = toupper(*(poi - 1));
						cc1 = toupper(*(poi + 1));
					} else {
						cc = *(poi - 1);
						cc1 = *(poi + 1);
					}
					while (cc <= cc1) {
						*comp++ = cc++;
						count++;
						}
					poi++; poi++;
				} else {
					count++;
					if (ignore_case)
						*comp++ = toupper(*poi++);
					else
						*comp++ = *poi++;
				}
			}
			poi++;
			*counter = count;
		} else {
			dot = FALSE;
			if (magic) {
				if (*poi == '.') dot = TRUE;
			} else {
				if (*poi == '\\' && *(poi + 1) == '.') {
					dot = TRUE; poi++; }
			}

			if (*poi == '\\') {
				switch (*(poi + 1)) {
				case 'n': *++poi = '\n'; break;
				case 'r': *++poi = '\r'; break;
				case 't': *++poi = '\t'; break;
				case '0': *++poi = '\0'; break;
				default : ++poi;
				}
			}
			if (magic)
				if (*(poi + 1) == '*') *comp++ = STAR;
					else	*comp++ = ONE;
			else
				if (*(poi + 1) == '\\' && *(poi + 2) == '*') *comp++ = STAR;
					else    *comp++ = ONE;

			if (dot) {
				*comp++ = 0;
				poi++;
			} else {
				*comp++ = 1;
				if (ignore_case)
					*comp++ = toupper(*poi++);
				else
					*comp++ = *poi++;
			}
		}
		if (magic) {
			if (*poi == '*') poi++;
		} else {
			if (*poi == '\\' && *(poi + 1) == '*') { poi++; poi++; }
		}
	}
	*comp = END;
	smode = ASCII;
	return 0;
}


/*
 * Compiling a hex expression to a regex string
 */
int
hex_comp(smem, pattern)
	char	*smem;
	char	*pattern;
{
	char	*end;
	char	*comp;
	int		cc, ccm;
	char	*counter;
	int		count, nr;

	comp = smem;
	poi = pattern;
	while (*poi != END) {
		while (*poi == ' ' || *poi == '\t') poi++;
		if (*poi == '[') {
			if (!(end = strchr(poi, ']'))) {
				emsg("Missing ]");
				return 1; }
			poi++;
			while (*poi == ' ' || *poi == '\t') poi++;

			if (*poi == ']' || (*poi == '^' && *(poi + 1) == ']')) {
				emsg(emptyclass);
				return 1; }
			if (*(end + 1) == '*') *comp++ = STAR;
				else	*comp++ = ONE;
			count = 1;
			counter = comp;
			comp++;
			if (*poi == '^')
				*comp++ = *poi++;
			else
				*comp++ = '\0';
			while (end > poi) {
				if (*poi == ' ' || *poi == '\t') poi++;
				else if (*poi == '-') {
					cc = *(comp - 1);
					poi++;
					if ((ccm = hexchar()) < 0) return 1;
					while (cc <= ccm) {
						*comp++ = cc++;
						count++;
						}
					poi++; poi++;
				} else {
					if ((nr = hexchar()) < 0) return 1;
					count++;
					*comp++ = nr;
				}
			}
			poi++;
			*counter = count;
			if (*poi == '*') poi++;
		} else if (*poi == '"') {
			poi++;
			if (!(end = strchr(poi, '"'))) {
				/*
				emsg("Missing '\"'");
				return 1;
				*/
				end = poi + strlen(poi);
				}
			while (end > poi) {
				*comp++ = 1;
				*comp++ = 1;
				if (ignore_case)
					*comp++ = toupper(*poi++);
				else
					*comp++ = *poi++;
			}
			poi++;
		} else {
			if (*poi == '.') {
				if (*(poi + 1) == '*') { *comp++ = STAR; poi++; }
					else	*comp++ = ONE;
				*comp++ = 0;
				poi++;
			} else {
				if ((nr = hexchar()) < 0) return 1;
				if (*poi == '*') { poi++; *comp++ = STAR; }
					else    *comp++ = ONE;
				*comp++ = 1;
				*comp++ = nr;
			}
		}
	}
	*comp = END;
	smode = HEX;
	return 0;
}


int
hexchar()
{
	int		nr;
	char	tmpbuf[3];

	if (isxdigit(*poi)) {
		tmpbuf[0] = *poi++;
		tmpbuf[1] = '\0';
		if (isxdigit(*poi)) tmpbuf[1] = *poi++;
		tmpbuf[2] = '\0';
		sscanf(tmpbuf, "%2x", &nr);
        while (*poi == ' ' || *poi == '\t') poi++;
		return nr;
	} else {
		emsg("Bad hex character@in expression");
		return -1;
	}
}
