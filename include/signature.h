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

/* ---------------------------------------------------------------
 *                     Internal abstractions
 * ---------------------------------------------------------------
 */

struct magic {
	int record_size;
	int type;
	char* str;
	char* description;
};

/* Signature item */
struct sig {
	char* name;
	char* description;
	int size;
	unsigned char* signature;
};

typedef struct sig signature_t;

/* List of signatures (named signatures pack) */
typedef struct sigl *siglist;

struct sigl {
	signature_t sg;
	siglist next;
};

struct sigpack {
	char* name;
	char* description;
	int size;
	siglist sigs;
};

typedef struct sigpack sigpack_t;
