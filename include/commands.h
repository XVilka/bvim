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

int command__help(char, int, char**);
int command__map(char, int, char**);
int command__unmap(char, int, char**);
int command__set(char, int, char**);
int command__block(char, int, char**);
int command__lua(char, int, char**);
int command__args(char, int, char**);
int command__source(char, int, char**);
int command__run(char, int, char**);
int command__cd(char, int, char**);
int command__edit(char, int, char**);
int command__file(char, int, char**);
int command__read(char, int, char**);
int command__xit(char, int, char**);
int command__next(char, int, char**);
int command__rewind(char, int, char**);
int command__append(char, int, char**);
int command__change(char, int, char**);
int command__mark(char, int, char**);
int command__yank(char, int, char**);
int command__overwrite(char, int, char**);
int command__undo(char, int, char**);
int command__version(char, int, char**);
int command__shell(char, int, char**);
int command__quit(char, int, char**);
int command__sleft(char, int, char**);
int command__sright(char, int, char**);
int command__rleft(char, int, char**);
int command__rright(char, int, char**);
int command__and(char, int, char**);
int command__or(char, int, char**);
int command__xor(char, int, char**);
int command__neg(char, int, char**);
int command__not(char, int, char**);
int command__fuz(char, int, char**);

void commands__Init();
void commands__Destroy();
int commands__Cmd_Add(struct command*);
int commands__Cmd_Del(char *name);


void docmdline(char *);
