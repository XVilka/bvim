/* dosconf.h
 *
 * 1996-02-28  V 1.0.0
 * 1999-01-21  V 1.1.0
 * 1999-03-17  V 1.1.1
 * 1999-07-01  V 1.2.0 beta
 * 1999-08-27  V 1.2.0 final
 *
 *  NOTE: Edit this file with tabstop=4 !
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

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you have the memmove function.  */
#define HAVE_MEMMOVE 1

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the strtol function.  */
#define HAVE_STRTOL 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <locale.h> header file.  */
#if __TURBOC__ > 0x0400
#	define HAVE_LOCALE_H 1
#endif

