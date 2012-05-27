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

int command__help(core_t*, buf_t*, char, int, char**);
int command__map(core_t*, buf_t*, char, int, char**);
int command__unmap(core_t*, buf_t*, char, int, char**);
int command__set(core_t*, buf_t*, char, int, char**);
int command__block(core_t*, buf_t*, char, int, char**);
int command__lua(core_t*, buf_t*, char, int, char**);
int command__args(core_t*, buf_t*, char, int, char**);
int command__source(core_t*, buf_t*, char, int, char**);
int command__run(core_t*, buf_t*, char, int, char**);
int command__cd(core_t*, buf_t*, char, int, char**);
int command__edit(core_t*, buf_t*, char, int, char**);
int command__file(core_t*, buf_t*, char, int, char**);
int command__read(core_t*, buf_t*, char, int, char**);
int command__xit(core_t*, buf_t*, char, int, char**);
int command__next(core_t*, buf_t*, char, int, char**);
int command__rewind(core_t*, buf_t*, char, int, char**);
int command__append(core_t*, buf_t*, char, int, char**);
int command__change(core_t*, buf_t*, char, int, char**);
int command__mark(core_t*, buf_t*, char, int, char**);
int command__yank(core_t*, buf_t*, char, int, char**);
int command__overwrite(core_t*, buf_t*, char, int, char**);
int command__undo(core_t*, buf_t*, char, int, char**);
int command__version(core_t*, buf_t*, char, int, char**);
int command__shell(core_t*, buf_t*, char, int, char**);
int command__quit(core_t*, buf_t*, char, int, char**);
int command__sleft(core_t*, buf_t*, char, int, char**);
int command__sright(core_t*, buf_t*, char, int, char**);
int command__rleft(core_t*, buf_t*, char, int, char**);
int command__rright(core_t*, buf_t*, char, int, char**);
int command__and(core_t*, buf_t*, char, int, char**);
int command__or(core_t*, buf_t*, char, int, char**);
int command__xor(core_t*, buf_t*, char, int, char**);
int command__neg(core_t*, buf_t*, char, int, char**);
int command__not(core_t*, buf_t*, char, int, char**);
int command__fuz(core_t*, buf_t*, char, int, char**);

void commands__Init(core_t*);
void commands__Destroy(core_t*);
int commands__Cmd_Add(core_t*, command_t*);
int commands__Cmd_Del(core_t*, char*);


void docmdline(core_t*, buf_t *, char *);
int chk_comm(core_t *core, buf_t* buf, int flag);

