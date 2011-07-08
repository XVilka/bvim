/* SET.H
 *
 * NOTE: Edit this file with tabstop=4 !
 *
 * Copyright 1996-2002 by Gerhard Buergmann
 * Gerhard.Buergmann@puon.at
 *
 * 1998-03-14 V 1.0.0
 * 1999-01-14 V 1.1.0
 * 1999-03-23 V 1.1.1
 * 1999-07-02 V 1.2.0 beta
 * 1999-08-14 V 1.2.0 final
 * 2000-08-21 V 1.3.0 final
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


struct	param {
	char	*fullname;	/* full parameter name */
	char	*shortname;	/* permissible abbreviation */
	long	nvalue;
	char	*svalue;
	int		flags;
};

extern	struct	param	params[];

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
#define	P_CO		16	/* color/attribute setting */

/*
 * Macro to get the value of a parameter
 */
#define	P(n)	(params[n].nvalue)
