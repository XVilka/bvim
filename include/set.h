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

struct param {
	char *fullname;		/* full parameter name */
	char *shortname;	/* permissible abbreviation */
	long nvalue;
	char *svalue;
	int flags;
};

struct color {
	char *fullname;		/* full name of color attribute */
	char *shortname;	/* permissible abbreviation */
	unsigned int r;
	unsigned int g;
	unsigned int b;
	unsigned int short_value;
};

extern struct param params[];
extern struct color colors[];

#define	P_BOOL		0x01	/* the parameter is boolean */
#define	P_NUM		0x02	/* the parameter is numeric */
#define P_TEXT		0x04	/* the paameter is text */
#define	P_CHANGED	0x08	/* the parameter has been changed */

/*
 * The following are the indices in the params array for each parameter
 */

/*
 * parameters
 */
#define	P_AW		0	/* Autowrite */
#define P_CM		1	/* Columns */
#define	P_EB		2	/* error bells */
#define	P_IC		3	/* ignore case in searches */
#define	P_MA		4	/* Magic characters in reg expr */
#define P_MM		5	/* move bytes in file */
#define	P_OF		6	/* address offset */
#define P_RO		7	/* Readonly */
#define	P_SS		8	/* scroll size */
#define	P_MO		9	/* show mode */
#define P_TT		10	/* Terminal type */
#define P_TE		11	/* Terse (short messages) */
#define P_US		12	/* Unix-Style of ASCII representation */
#define	P_LI		13	/* lines */
#define P_WL		14	/* Wordlength for w, W, b, B command */
#define	P_WS		15	/* wrapscan */

/*
 * colors
 */

#define C_BG		0	/* Background color */
#define C_AD		1	/* Addresses color */
#define C_HX		2	/* Hexadecimal data window color */
#define C_DT		3	/* Source data window color */
#define C_ER		4	/* Error message color */
#define C_ST		5	/* Status line color */
#define C_CM		6	/* Command line color */
#define C_WN		7	/* Window color */
#define C_AB		8	/* Addresses background color */

/*
 * Macro to get the value of a parameter
 */
#define	P(n)	(params[n].nvalue)
#define C_r(n)	(colors[n].r)
#define C_g(n)	(colors[n].g)
#define	C_b(n)	(colors[n].b)
#define C_s(n)	(colors[n].short_value)

void set_palette(void);
