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

/* ========= User management =========== */

void collaboration__UserList(void) {
}

// Add user
void collaboration__UserAdd(void) {
}

// Remove user
void collaboration__UserDel(void) {
}

/* ========= Server management ========== */

// Connect to the server
void collaboration__ServerConnect(void) {
}

// Auth into the server
void collaboration__ServerAuth(void) {
}

// Disconnect from the server
void collaboration__ServerDisconnect(void) {
}

/* ========= Project management ========= */

// Lists available projects on the server
void collaboration__ProjectList(void) {
}

// Open/enter into the selected project
void collaboration__ProjectOpen(void) {
}

// Create new project and upload it on the server
void collaboration__ProjectAdd(void) {
}

// Remove project from the server
void collaboration__ProjectRemove(void) {
}

/* ========= Edits management ========= */

// Add edit from the user
void collaboration__EditAdd(char* user) {
}

// Undo edit from the user
void collaboration__EditUndo(char* user, char* edit_id) {
}

// Add block from the user
void collaboration__BlockAdd(char* user) {
}

// Remove block from the user
void collaboration__BlockDel(char* user, int block_id) {
}

// Add annotation to the selected block
void collaboration__BlockAnnotation(char* user, int block_id, char* annotation) {
}

// Do command from the user (except :q and :shell)
void collaboration__CmdUser(char* user, char* cmd) {
}

// Show log of the changes in the nice format
void log__Show(int items) {
}
