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

/* Exported types:
 * int command__handler(char, int, char**); -  for commands handlers
 * int handler__func() - for key handlers
 * bvim_add_lua_function - not yet implemented
 */

typedef struct plugin *plugin_link;

struct plugin_ {
	char* name;
	char* author;
	char* license;
	struct {
		short major;
		short minor;
	} version;
	char* description;
	void* module;
	 struct {
		struct key *keys;
		struct command *cmds;
		struct luaF_item *luaF_list;
	} exports;
};

typedef struct plugin_ plugin_t;

struct plugin {
	plugin_t item;
	plugin_link next;
};

/* Plugin API:
 *
 * plugin_t plugin_register() function - fill plugin info
 * plugin_init(plugin_t plugin);
 *
 * Import core, state, blocks, commands and keys structure.
 *
 * plugin_unregister() function - remove plugin info,
 * remove previously exported handlers, keys, commands, lua 
 * functions, etc.
 */

// TODO: Export all data as set of objects
// TODO: and export iterator functions on these sets

/* ---------------------------------------------------------
 *               for using only in bvimm
 * ---------------------------------------------------------
 */

int plugins__Init();
plugin_t* plugins__GetByName(char* name);
int plugins__Destroy();

int plugin__Load(char *path);
int plugin__Unload(plugin_t plg);
