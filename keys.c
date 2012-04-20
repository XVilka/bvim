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

#include "bvim.h"
#include "blocks.h"
#include "ui.h"
#include "keys.h"
#include "bscript.h"

//extern core_t core;

/* ----------------------------------------------------------
 *                    Internal functions
 * ----------------------------------------------------------
 */

int KeyAdd(core_t *core, bkey_t *item)
{
	if (core->keymap.items == core->keymap.allocated) {
		if (core->keymap.allocated == 0)
			core->keymap.allocated = 3;
		else
			core->keymap.allocated += 4;

		void *_tmp =
		    realloc(core->keymap.arr,
			    (core->keymap.allocated * sizeof(struct key)));

		if (!_tmp) {
			fprintf(stderr, "ERROR [KeyAdd]: Couldn't realloc memory!\n");
			return (-1);
		}
		core->keymap.arr = (struct key *)_tmp;
	}
	core->keymap.arr[core->keymap.items] = *item;
	core->keymap.items++;

	return core->keymap.items;
}

int KeyDel(core_t *core, bkey_t *item)
{
	return 0;
}

int KeyDefaults(core_t *core)
{
	int i = 0;
	struct key keys_def[] = {
		{'^', "Caret", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_HEX}},
		{'$', "Dollar", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_ASCII}},
		{'\t', "Tab", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__toggle}},
		{'~', "Tilda", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__tilda}},
		{KEY_HOME, "Home", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_home}},
		{'H', "H", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_home}},
		{'M', "M", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__M}},
		{KEY_LL, "LL", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__L}},
		{'L', "L", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__L}},
		{BVI_CTRL('H'), "Ctrl-H", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_left}},
		{KEY_BACKSPACE, "Backspace", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_left}},
		{KEY_LEFT, "Left", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_left}},
		{'h', "h", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_left}},
		{' ', "Space", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_right}},
		{KEY_RIGHT, "Right", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_right}},
		{'l', "l", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_right}},
		{'-', "Minus", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_up}},
		{KEY_UP, "Up", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_up}},
		{'k', "k", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_up}},
		{'+', "Plus", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_EOL}},
		{CR, "Enter", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_EOL}},
		{'j', "j", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_down}},
		{BVI_CTRL('J'), "Ctrl-J", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_down}},
		{BVI_CTRL('N'), "Ctrl-N", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_down}},
		{KEY_DOWN, "Down", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_down}},
		{'|', "|", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__line}},
		{'S', "S", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__toolwin_toggle}},
		{':', ":", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__cmdstring}},
		{BVI_CTRL('B'), "Ctrl-B", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__previous_page}},
		{KEY_PPAGE, "PageUp", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__previous_page}},
		{BVI_CTRL('D'), "Ctrl-D", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__scrolldown}},
		{BVI_CTRL('U'), "Ctrl-U", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__scrollup}},
		{BVI_CTRL('E'), "Ctrl-E", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__linescroll_down}},
		{BVI_CTRL('F'), "Ctrl-F", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__nextpage}},
		{KEY_NPAGE, "PageDown", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__nextpage}},
		{BVI_CTRL('G'), "Ctrl-G", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__fileinfo}},
		{BVI_CTRL('L'), "Ctrl-L", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__screen_redraw}},
		{BVI_CTRL('Y'), "Ctrl-Y", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__linescroll_up}},
		{BVI_CTRL('R'), "Ctrl-R", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__luarepl}},
		{BVI_CTRL('S'), "Ctrl-S", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__toggle_selection}},
		{'A', "A", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__append_mode}},
		{'B', "B", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__backsearch}},
		{'b', "b", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__backsearch}},
		{'e', "e", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__setpage}},
		{',', ",", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__doft1}},
		{';', ";", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__doft2}},
		{'F', "F", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__doft3_F}},
		{'f', "f", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__doft3_f}},
		{'t', "t", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__doft3_t}},
		{'T', "T", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__doft3_T}},
		{'G', "G", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto1}},
		{'g', "g", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto2}},
		{'?', "?", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__search_string1}},
		{'/', "/", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__search_string2}},
		{'#', "#", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__search_string3}},
		{'\\', "\\", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__search_string4}},
		{'n', "n", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__search_next}},
		{'N', "N", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__search_next}},
		{'m', "m", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__mark}},
		{'\'', "'", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_mark}},
		{'`', "`", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto_mark}},
		{'D', "D", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__trunc}},
		{'o', "o", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__overwrite}},
		{'P', "P", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__paste}},
		{'r', "r", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__redo}},
		{'R', "R", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__redo}},
		{'u', "u", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__undo}},
		{'U', "U", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__undo}},
		{'v', "v", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__visual}},
		{'W', "W", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__wordsearch}},
		{'w', "w", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__wordsearch}},
		{'y', "y", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__yank}},
		{'z', "z", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__doz}},
		{'Z', "Z", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__exit}},
		{'.', ".", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__stuffin}},
		/* disabled by default */
		{'I', "I", "", 0, BVI_HANDLER_INTERNAL,
		 {.func = handler__insert}},
		{'s', "s", "", 0, BVI_HANDLER_INTERNAL,
		 {.func = handler__s}},
		{'a', "a", "", 0, BVI_HANDLER_INTERNAL,
		 {.func = handler__append2}},
		{'i', "i", "", 0, BVI_HANDLER_INTERNAL,
		 {.func = handler__insert2}},
		{'p', "p", "", 0, BVI_HANDLER_INTERNAL,
		 {.func = handler__paste2}},
		{'c', "c", "", 0, BVI_HANDLER_INTERNAL,
		 {.func = handler__c_or_d}},
		{'d', "d", "", 0, BVI_HANDLER_INTERNAL,
		 {.func = handler__c_or_d}},
		{'x', "x", "", 0, BVI_HANDLER_INTERNAL,
		 {.func = handler__x}},
		{'X', "X", "", 0, BVI_HANDLER_INTERNAL,
		 {.func = handler__x2}},
		{0, NULL, NULL, 0, 0, {NULL}} // end marker
	};
	while (keys_def[i].id != 0) {
		KeyAdd(core, &keys_def[i]);
		i++;
	}
	return 0;
}

/* -----------------------------------------------------
 *              Exported functions
 * -----------------------------------------------------
 */

void keys__Init(core_t *core)
{
	core->keymap.arr = NULL;
	core->keymap.items = 0;
	core->keymap.allocated = 0;
	KeyDefaults(core);
}

void keys__Destroy(core_t *core)
{
	free(core->keymap.arr);
}

// TODO: Implement parsing key strings, like <Ctrl-K>
bkey_t *keys__KeyString_Parse(core_t *core, char *key_string)
{
	/*
	 * Use strtok with "-" delimeter, then compare
	 */
	char* saved = NULL;
	char* stok = NULL;

	short ctrl_flag = 0;
	struct key *k;

	k = (struct key *)malloc(sizeof(struct key *));
	/*
	stok = strtok_r(key_string, "-", &saved);
	while (stok != NULL) {
		switch (stok) {
			case "Ctrl":
				if (ctrl_flag != 0) {
					//error
				} else {
					ctrl_flag = 1;
				}
			case "Tab":
				key = '\t';
			case "Home":
			case "End":
			case "Left":
			case "Right":
			case "Up":
			case "Down":
			case "PgUp":
			case "PgDown":
			case "Ins":
			case "Shift":
			case "Alt":
			case "Bcksp":
				break;
			default:
				break;
		}
		k->id = ; // key integer value
		k->name = key_string;
		k->description = "";
		k->enabled = 1;
		k->handler_type = BVI_HANDLER_SCRIPT;
		k->handler.int_cmd = ;// do string
		stok = strtok_r(NULL, "-", &saved);
	}
	*/
	return k;
}

int keys__Key_Map(core_t *core, bkey_t *map_key)
{
	KeyAdd(core, map_key);
	return 0;
}

int keys__Key_Unmap(core_t *core, bkey_t *map_key)
{
	KeyDel(core, map_key);
	return 0;
}

void keys__KeyMaps_Show(core_t *core)
{
	char dispbuf[256];
	char luacmdbuf[256];
	int i = 0;

	dispbuf[0] = '\0';
	while (i < core->keymap.items) {
		luacmdbuf[0] = '\0';
		sprintf(luacmdbuf, "map %s %s\n", core->keymap.arr[i].name,
			core->keymap.arr[i].description);
		strcat(dispbuf, luacmdbuf);
		i++;
	}
	ui__StatusMsg(dispbuf);
	wait_return(core, TRUE);
}

int keys__Key_Pressed(core_t *core, int key_code)
{
	int j = 0;
	while (j < core->keymap.items) {
		if ((core->keymap.arr[j].id == key_code) & (core->keymap.arr[j].enabled ==
						      1)) {
			if ((core->keymap.arr[j].handler_type ==
			     BVI_HANDLER_INTERNAL) & (core->keymap.arr[j].handler.
						      func != NULL)) {
				(*(core->keymap.arr[j].handler.func)) ();
			} else
			    if ((core->keymap.arr[j].handler_type ==
				 BVI_HANDLER_LUA) & (core->keymap.arr[j].handler.
						     lua_cmd != NULL)) {
				bvim_run_lua_string(core->keymap.arr[j].handler.
						   lua_cmd);
			}
		}
		j++;
	}
	return 0;
}
