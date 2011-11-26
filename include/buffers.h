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

/* e.g.
 * input = "print(4+9)"
 * output = "13"
 */
struct iorecord {
	char* input;
	char* output;
};

typedef struct iorecord iorecord_t;

typedef struct iorecord_l *iorecord_link;

struct iorecord_l {
	iorecord_t item;
	iorecord_link next;
};

unsigned long io__BufferAdd();
int io__BufferAdd2(unsigned long id);
int io__BufferDestroy(unsigned long buffer_id);
unsigned long io__RecordInsert(unsigned long buffer_id, iorecord_t record);
int io__RecordDelete(unsigned long buffer_id, unsigned long record_id);
iorecord_t* io__RecordGet(unsigned long buffer_id, unsigned long record_id);
