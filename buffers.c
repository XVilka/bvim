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
