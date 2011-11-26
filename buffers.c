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

#include "buffers.h"

/* --------------------------------------------------------
 *                  File buffers
 * -------------------------------------------------------- */
int file__BufferAdd() {
	return 0;
}

int file__BufferDel() {
	return 0;
}

/* --------------------------------------------------------
 *          Buffers handling for actions,
 *           e.g. :yank, :paste, etc
 * -------------------------------------------------------- */
unsigned long action__BufferAdd() {
	return 0; // return buffer id
}

int action__BufferDel(unsigned long buffer_id) {
	return 0;
}

/* --------------------------------------------------------
 *                    I/O buffers
 * -------------------------------------------------------- */

unsigned long io__BufferAdd() {
	return 0; // return buffer id
}

int io__BufferAdd2(unsigned long id) {
	return 0;
}

int io__BufferDestroy(unsigned long buffer_id) {
	return 0;
}
/*
iobuf_t io__BufferGet(unsigned long buffer_id) {
	return NULL;
}
*/
unsigned long io__RecordInsert(unsigned long buffer_id, iorecord_t record) {
	return 0; // return record id
}

int io__RecordDelete(unsigned long buffer_id, unsigned long record_id) {
	return 0;
}

iorecord_t* io__RecordGet(unsigned long buffer_id, unsigned long record_id) {
	return 0;
}
