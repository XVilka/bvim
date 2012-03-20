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

#include <fcntl.h>

/* Include the Lua API header files. */
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "bvim.h"
#include "set.h"
#include "bmath.h"
#include "keys.h"
#include "blocks.h"
#include "buffers.h"
#include "search.h"
#include "bscript.h"
#include "commands.h"
#include "ui.h"
#include "plugins.h"

lua_State *lstate;

extern core_t core;
extern state_t state;

extern struct MARKERS_ markers[MARK_COUNT];
extern WINDOW *tools_win;

/* -----------------------------------------------------------------------------
 *                       Lua errors handling
 * -----------------------------------------------------------------------------
 */

static int bvim_lua_error_handler(lua_State *L)
{
	char* msg = NULL;

	if (state.mode != BVI_MODE_REPL) {
		msg = (char*)lua_tostring(L, 1);
		if (msg != NULL) 
			ui__ErrorMsg(msg);
		else
			ui__ErrorMsg("Unknown error");
	} else {
		msg = (char*)lua_tostring(L, 1);
		if (msg != NULL)
			ui__REPLWin_print(msg);
		else
			ui__REPLWin_print("Unknown error");
	}
	return 0;
}

static int bvim_lua_error_raise(lua_State *L, char* fmt, ...)
{
	char msg[1024];
	va_list ap;

	msg[0] = '\0';
	
	va_start(ap, fmt);
	vsprintf(msg, fmt, ap);
	va_end(ap);

	/* Exits on luaL_error(), so disabled
	if (state.mode != BVI_MODE_REPL) {
		ui__ErrorMsg(msg);
	} else {
		luaL_error(L, "%s", msg);
	}
	*/
	if (msg[0] != '\0') 
		ui__ErrorMsg(msg);
	return 0;
}

// TODO: Store errors in linked list
// TODO: Store lua output in linked list

/* Add new editor command, like :command */
/* lua: command_add(name, description, type, script) */
static int bvim_command_add(lua_State *L)
{
	int handler_type = 0;
	struct command new_cmd;
	if (lua_gettop(L) == 4) {
		new_cmd.id = 1;
		new_cmd.name = (char *)lua_tostring(L, 1);
		new_cmd.description = (char *)lua_tostring(L, 2);
		new_cmd.enabled = 1;
		handler_type = (int)lua_tonumber(L, 3);
		if (handler_type == 2) {
			new_cmd.handler_type = BVI_HANDLER_LUA;
			new_cmd.handler.lua_cmd = (char *)lua_tostring(L, 4);
		} else if (handler_type == 1) {
			new_cmd.handler_type = BVI_HANDLER_SCRIPT;
			new_cmd.handler.int_cmd = (char *)lua_tostring(L, 4);
		}
		new_cmd.size1 = strlen(new_cmd.name);
		new_cmd.size2 = 2;
		commands__Cmd_Add(&new_cmd);
	} else {
		bvim_lua_error_raise(L, "Error in lua command_add function! Wrong format!");
	}
	return 0;
}

/* Remove editor command, like :command */
/* lua: command_del(name) */
static int bvim_command_del(lua_State *L)
{
	char* name;
	if (lua_gettop(L) == 1) {
		name = (char *)lua_tostring(L, 1);
		commands__Cmd_Del(name);
	} else {
		bvim_lua_error_raise(L, "Error in lua command_del function! Wrong format!");
	}
	return 0;
}

/* -----------------------------------------------------------------------------
 */

/* Save current buffer into file */
/* lua: save(filename, [start, end, flags]) */
static int bvim_save(lua_State * L)
{
	char *filename;
	int flags = O_RDWR;
	int n = lua_gettop(L);
	if (n == 1) {
		filename = (char*)lua_tostring(L, 1);
		save(filename, core.editor.mem, core.editor.maxpos, flags);
	} else if (n == 2) {
		filename = (char*)lua_tostring(L, 1);
		flags = (int)lua_tonumber(L, 2);
		save(filename, core.editor.mem, core.editor.maxpos, flags);
	}
	return 0;
}

/* Load file into current buffer */
/* lua: load(filename, [mode]) */
static int bvim_load(lua_State * L)
{
	char *filename;
	int mode;
	int n = lua_gettop(L);

	if (n == 1) {
		filename = (char *)lua_tostring(L, 1);
		load(filename);
	} else if (n == 2) {
		filename = (char *)lua_tostring(L, 1);
		mode = (int)lua_tonumber(L, 2);
		if (mode == 1) {
			addfile(filename);
		} else {
			load(filename);
		}
	}
	return 0;
}

/* Get file path for current buffer */
static int bvim_file(lua_State * L)
{
	char *filename = "dummy";
	lua_pushstring(L, filename);
	return 1;
}

/* Load plugin */
/* lua: plugin_load(filename) */
static int bvim_plugin_load(lua_State * L)
{
	char *filename;
	int n = lua_gettop(L);

	if (n == 1) {
		filename = (char *)lua_tostring(L, 1);
		plugin__Load(filename);
	} else {
		bvim_lua_error_raise(L, "Fail to load plugin - wrong format");
	}
	return 0;
}

// FIXME: Plugins list stub
static int bvim_plugin_list(lua_State *L)
{
	return 0;
}

static int bvim_plugin_info(lua_State *L)
{
	char* name;
	char tmpstr[1024];
	plugin_t* plg;
	int n = lua_gettop(L);

	if (n == 1) {
		name = (char *)lua_tostring(L, 1);
		plg = plugins__GetByName(name);
		if (plg != NULL) {
			if (state.mode == BVI_MODE_REPL) {
				tmpstr[0] = '\0';
				sprintf(tmpstr, "Name: %s", plg->name);
				ui__REPLWin_print(tmpstr);
				sprintf(tmpstr, "Author: %s", plg->author);
				ui__REPLWin_print(tmpstr);
				sprintf(tmpstr, "License: %s", plg->license);
				ui__REPLWin_print(tmpstr);
				sprintf(tmpstr, "Version: %d.%d", plg->version.major, plg->version.minor);
				ui__REPLWin_print(tmpstr);
				sprintf(tmpstr, "Description: %s", plg->description);
				ui__REPLWin_print(tmpstr);
			} else {
				bvim_info(state.mode, "%s %d.%d - %s", plg->name, plg->version.major, plg->version.minor, plg->description);
			}
		} else {
			bvim_lua_error_raise(L, "Haven't such plugin");
		}
	} else {
		bvim_lua_error_raise(L, "Wrong format of plugin_info function");
	}
	return 0;
}

/* TODO: add function , which returns list of all presented blocks */

/* Select block in the buffer */
/* lua: block_select(block_number, start, end, pattern) */
static int bvim_block_select(lua_State * L)
{
	struct block_item tmp_blk;
	if (lua_gettop(L) == 4) {
		tmp_blk.id = (unsigned int)lua_tonumber(L, 1);
		if (blocks__GetByID(tmp_blk.id) == NULL) {
			tmp_blk.pos_start = (unsigned long)lua_tonumber(L, 2);
			tmp_blk.pos_end = (unsigned long)lua_tonumber(L, 3);
			tmp_blk.palette = (unsigned int)lua_tonumber(L, 4);
			tmp_blk.hl_toggle = 1;
			blocks__Add(tmp_blk);
			ui__BlockHighlightAdd(&tmp_blk);
			if (state.mode != BVI_MODE_REPL)
				ui__Screen_Repaint();
		} else {
			bvim_lua_error_raise(L, "Wrong block ID! It already exist!");
		}
	} else {
		bvim_lua_error_raise
		    (L, "Error in lua block_select function! Wrong format!");
	}
	return 0;
}

/* Fold block in the buffer */
/* lua: block_fold(block_number) */

static int bvim_block_fold(lua_State * L)
{
	struct block_item *tmp_blk;
	unsigned int id = 0;
	if (lua_gettop(L) == 1) {
		id = (int)lua_tonumber(L, 1);
		tmp_blk = blocks__GetByID(id);
		if (tmp_blk != NULL) {
			tmp_blk->folding = 1;
			if (state.mode != BVI_MODE_REPL)
				ui__Screen_Repaint();
		} else {
			bvim_lua_error_raise(L, "Wrong block number!");
		}
	} else {
		bvim_lua_error_raise(L, "Error in lua block_fold function! Wrong format!");
	}
	return 0;
}


/* Read block in the buffer */
/* lua: block_read(block_id) */
static int bvim_block_read(lua_State * L)
{
	struct block_item *tmp_blk;
	unsigned int id = 0;
	char *blck = NULL;
	if (lua_gettop(L) == 1) {
		id = (int)lua_tonumber(L, 1);
		tmp_blk = blocks__GetByID(id);
		if (tmp_blk != NULL) {
			if (tmp_blk->pos_end > tmp_blk->pos_start) {
				blck = (char *)malloc(tmp_blk->pos_end - tmp_blk->pos_start + 1);
				memcpy(blck, core.editor.mem + tmp_blk->pos_start, tmp_blk->pos_end - tmp_blk->pos_start + 1);
				lua_pushstring(L, blck);
			} else {
				bvim_lua_error_raise(L, "You need select valid block before read!");
			}
		} else {
			bvim_lua_error_raise(L, "Error in lua block_read function! Wrong format!");
		}
	}
	return 1;
}

/* AND bit operation on block */
/* lua: block_and(block_number, mask) */
static int bvim_block_and(lua_State * L)
{
	unsigned int id = 0;
	struct block_item *tmp_blk;

	if (lua_gettop(L) == 2) {
		id = (int)lua_tonumber(L, 1);
		tmp_blk = blocks__GetByID(id);
		if ((tmp_blk != NULL) & (tmp_blk->pos_end > tmp_blk->pos_start)) {
			math__logic_block(AND, (char *)lua_tostring(L, 2), id);
		} else {
			bvim_lua_error_raise(L, "You need select valid block before and operation!");
		}
	} else {
		bvim_lua_error_raise(L, "Error in lua block_and function! Wrong format!");
	}
	return 0;
}

/* OR bit operation on block */
/* lua: block_or(block_number, mask) */
static int bvim_block_or(lua_State * L)
{
	unsigned int id = 0;
	struct block_item *tmp_blk;

	if (lua_gettop(L) == 2) {
		id = (unsigned int)lua_tonumber(L, 1);
		tmp_blk = blocks__GetByID(id);
		if ((tmp_blk != NULL) & (tmp_blk->pos_end > tmp_blk->pos_start)) {
			math__logic_block(OR, (char *)lua_tostring(L, 2), id);
		} else {
			bvim_lua_error_raise
			    (L, "You need select valid block before or operation!");
		}
	} else {
		bvim_lua_error_raise(L, "Error in lua block_and function! Wrong format!");
	}
	return 0;
}

/* XOR bit operation on block */
/* lua: block_xor(block_number, mask) */
static int bvim_block_xor(lua_State * L)
{
	unsigned int id = 0;
	struct block_item *tmp_blk;

	if (lua_gettop(L) == 2) {
		id = (unsigned int)lua_tonumber(L, 1);
		tmp_blk = blocks__GetByID(id);
		if ((tmp_blk != NULL) & (tmp_blk->pos_end > tmp_blk->pos_start)) {
			math__logic_block(XOR, (char *)lua_tostring(L, 2), id);
		} else {
			bvim_lua_error_raise
			    (L, "You need select valid block before xor operation!");
		}
	} else {
		bvim_lua_error_raise(L, "Error in lua block_and function! Wrong format!");
	}
	return 0;
}

/* LSHIFT bit operation on block */
/* lua: block_lshift(block_number, count) */
static int bvim_block_lshift(lua_State * L)
{
	unsigned int id = 0;
	struct block_item *tmp_blk;

	if (lua_gettop(L) == 2) {
		id = (unsigned int)lua_tonumber(L, 1);
		tmp_blk = blocks__GetByID(id);
		if ((tmp_blk != NULL) & (tmp_blk->pos_end > tmp_blk->pos_start)) {
			math__logic_block(LSHIFT, (char *)lua_tostring(L, 2), id);
		} else {
			bvim_lua_error_raise
			    (L, "You need select valid block before lshift operation!");
		}
	} else {
		bvim_lua_error_raise(L, "Error in lua block_and function! Wrong format!");
	}
	return 0;
}

/* RSHIFT bit operation on block */
/* lua: block_rshift(block_number, count) */
static int bvim_block_rshift(lua_State * L)
{
	unsigned int id = 0;
	struct block_item *tmp_blk;

	if (lua_gettop(L) == 2) {
		id = (unsigned int)lua_tonumber(L, 1);
		tmp_blk = blocks__GetByID(id);
		if ((tmp_blk != NULL) & (tmp_blk->pos_end > tmp_blk->pos_start)) {
			math__logic_block(RSHIFT, (char *)lua_tostring(L, 2), id);
		} else {
			bvim_lua_error_raise
			    (L, "You need select valid block before rshift operation!");
		}
	} else {
		bvim_lua_error_raise(L, "Error in lua block_and function! Wrong format!");
	}
	return 0;
}

/* LROTATE bit operation on block */
/* lua: block_lrotate(block_number, count) */
static int bvim_block_lrotate(lua_State * L)
{
	unsigned int id = 0;
	struct block_item *tmp_blk;

	if (lua_gettop(L) == 2) {
		id = (unsigned int)lua_tonumber(L, 1);
		tmp_blk = blocks__GetByID(id);
		if ((tmp_blk != NULL) & (tmp_blk->pos_end > tmp_blk->pos_start)) {
			math__logic_block(LROTATE, (char *)lua_tostring(L, 2), id);
		} else {
			bvim_lua_error_raise
			    (L, "You need select valid block before lrotate operation!");
		}
	} else {
		bvim_lua_error_raise(L, "Error in lua block_and function! Wrong format!");
	}
	return 0;
}

/* RROTATE bit operation on block */
/* lua: block_rrotate(block_number, count) */
static int bvim_block_rrotate(lua_State * L)
{
	unsigned int id = 0;
	struct block_item *tmp_blk;

	if (lua_gettop(L) == 2) {
		id = (unsigned int)lua_tonumber(L, 1);
		tmp_blk = blocks__GetByID(id);
		if ((tmp_blk != NULL) & (tmp_blk->pos_end > tmp_blk->pos_start)) {
			math__logic_block(RROTATE, (char *)lua_tostring(L, 2), id);
		} else {
			bvim_lua_error_raise
			    (L, "You need select valid block before rrotate operation!");
		}
	} else {
		bvim_lua_error_raise(L, "Error in lua block_and function! Wrong format!");
	}
	return 0;
}

/* Calculate entropy of buffer */
/* lua: entropy(block_number) */
static int bvim_entropy(lua_State * L)
{
	unsigned int id = 0;
	struct block_item *tmp_blk;
	double entropy = 0;

	if (lua_gettop(L) == 1) {
		if (lua_type(L, 1) == LUA_TNUMBER) {
			id = (unsigned int)lua_tonumber(L, 1);
			tmp_blk = blocks__GetByID(id);
			if (tmp_blk != NULL) {
				entropy = math__entropy(id);
				lua_pushnumber(L, entropy);
				return 1;
			}
		} else {
			bvim_lua_error_raise(L, "Argument must be block id!");
		}
	} else {
		bvim_lua_error_raise(L, "Error in lua entropy function! Wrong format!");
	}
	return 0;
}


/* Calculate CRC16 checksum of buffer */
/* lua: crc16(block_number) */
/* lua: crc16("string") */
static int bvim_crc16(lua_State * L)
{
	unsigned int id = 0;
	char *blck = NULL;
	struct block_item *tmp_blk;
	unsigned int crc = 0;
	if (lua_gettop(L) == 1) {
		if (lua_type(L, 1) == LUA_TNUMBER) {
			id = (unsigned int)lua_tonumber(L, 1);
			tmp_blk = blocks__GetByID(id);
			if ((tmp_blk != NULL) & (tmp_blk->pos_end > tmp_blk->pos_start)) {
				blck = (char *)malloc(tmp_blk->pos_end -  tmp_blk->pos_start + 1);
				memcpy(blck, core.editor.mem + tmp_blk->pos_start, tmp_blk->pos_end - tmp_blk->pos_start + 1);
				crc = crc16(blck, tmp_blk->pos_end - tmp_blk->pos_start + 1, crc);
				lua_pushnumber(L, crc);
				return 1;
			}
		} else if (lua_type(L, 1) == LUA_TSTRING) {
			blck = (char *)malloc(strlen((char *)lua_tostring(L, 1)));
			blck = (char *)lua_tostring(L, 1);
			crc = crc16(blck, strlen(blck), crc);
			lua_pushnumber(L, crc);
			return 1;
		}
	} else {
		bvim_lua_error_raise(L, "Error in lua crc16 function! Wrong format!");
	}
	return 0;
}

/* Calculate CRC32 checksum of buffer */
/* lua: crc32(block_number) */
/* lua: crc32("string") */
static int bvim_crc32(lua_State * L)
{
	unsigned int id = 0;
	char *blck = NULL;
	struct block_item *tmp_blk;
	unsigned int crc = 0;
	if (lua_gettop(L) == 1) {
		if (lua_type(L, 1) == LUA_TNUMBER) {
			id = (unsigned int)lua_tonumber(L, 1);
			tmp_blk = blocks__GetByID(id);
			if ((tmp_blk != NULL) & (tmp_blk->pos_end > tmp_blk->pos_start)) {
				blck = (char *)malloc(tmp_blk->pos_end - tmp_blk->pos_start + 1);
				memcpy(blck, core.editor.mem + tmp_blk->pos_start, tmp_blk->pos_end - tmp_blk->pos_start + 1);
				crc = crc32(blck,  tmp_blk->pos_end - tmp_blk->pos_start + 1, crc);
				lua_pushnumber(L, crc);
				return 1;
			}
		} else if (lua_type(L, 1) == LUA_TSTRING) {
			blck = (char *)malloc(strlen((char *)lua_tostring(L, 1)));
			blck = (char *)lua_tostring(L, 1);
			crc = crc32(blck, strlen(blck), crc);
			lua_pushnumber(L, crc);
			return 1;
		}
	} else {
		bvim_lua_error_raise(L, "Error in lua crc32 function! Wrong format!");
	}
	return 0;
}

/* Calculate MD4 hash of buffer */
/* lua: md4_hash(block_number) */
/* lua: md4_hash("string") */
static int bvim_md4_hash(lua_State * L)
{
	unsigned int id = 0;
	char *blck = NULL;
	struct block_item *tmp_blk;
	char hash[65];
	hash[0] = '\0';
	if (lua_gettop(L) == 1) {
		if (lua_type(L, 1) == LUA_TNUMBER) {
			id = (unsigned int)lua_tonumber(L, 1);
			tmp_blk = blocks__GetByID(id);
			if ((tmp_blk != NULL) & (tmp_blk->pos_end > tmp_blk->pos_start)) {
				blck = (char *)malloc(tmp_blk->pos_end - tmp_blk->pos_start + 1);
				memcpy(blck, core.editor.mem + tmp_blk->pos_start, tmp_blk->pos_end - tmp_blk->pos_start + 1);
				math__md4_hash_string((unsigned char *)blck, tmp_blk->pos_end - tmp_blk->pos_start + 1, hash);
				lua_pushstring(L, hash);
				return 1;
			} else {
				bvim_lua_error_raise
				    (L, "You need select valid block before MD4 hash calculation!");
			}
		} else if (lua_type(L, 1) == LUA_TSTRING) {
			blck = (char *)malloc(strlen((char *)lua_tostring(L, 1)));
			blck = (char *)lua_tostring(L, 1);
			math__md4_hash_string((unsigned char *)blck, strlen(blck), hash);
			lua_pushstring(L, hash);
			return 1;
		}
	} else {
		bvim_lua_error_raise(L, "Error in lua md4_hash function! Wrong format!");
	}
	return 0;
}

/* Calculate MD5 hash of buffer */
/* lua: md5_hash(block_number) */
/* lua: md5_hash("string") */
static int bvim_md5_hash(lua_State * L)
{
	unsigned int id = 0;
	char *blck = NULL;
	struct block_item *tmp_blk;
	char hash[65];
	hash[0] = '\0';
	if (lua_gettop(L) == 1) {
		if (lua_type(L, 1) == LUA_TNUMBER) {
			id = (unsigned int)lua_tonumber(L, 1);
			tmp_blk = blocks__GetByID(id);
			if ((tmp_blk != NULL) & (tmp_blk->pos_end > tmp_blk->pos_start)) {
				blck = (char *)malloc(tmp_blk->pos_end - tmp_blk->pos_start + 1);
				memcpy(blck, core.editor.mem + tmp_blk->pos_start, tmp_blk->pos_end - tmp_blk->pos_start + 1);
				math__md5_hash_string((unsigned char *)blck, tmp_blk->pos_end - tmp_blk->pos_start + 1, hash);
				lua_pushstring(L, hash);
				return 1;
			} else {
				bvim_lua_error_raise
				    (L, "You need select valid block before MD5 hash calculation!");
			}
		} else if (lua_type(L, 1) == LUA_TSTRING) {
			blck = (char *)malloc(strlen((char *)lua_tostring(L, 1)));
			blck = (char *)lua_tostring(L, 1);
			math__md5_hash_string((unsigned char *)blck, strlen(blck), hash);
			lua_pushstring(L, hash);
			return 1;
		}
	} else {
		bvim_lua_error_raise(L, "Error in lua md5_hash function! Wrong format!");
	}
	return 0;
}

/* Calculate SHA1 hash of buffer */
/* lua: sha1_hash(block_number) */
/* lua: sha1_hash("string") */
static int bvim_sha1_hash(lua_State * L)
{
	unsigned int id = 0;
	char *blck = NULL;
	struct block_item *tmp_blk;
	char hash[65];
	hash[0] = '\0';
	if (lua_gettop(L) == 1) {
		if (lua_type(L, 1) == LUA_TNUMBER) {
			id = (unsigned int)lua_tonumber(L, 1);
			tmp_blk = blocks__GetByID(id);
			if ((tmp_blk != NULL) & (tmp_blk->pos_end > tmp_blk->pos_start)) {
				blck = (char *)malloc(tmp_blk->pos_end - tmp_blk->pos_start + 1);
				memcpy(blck, core.editor.mem + tmp_blk->pos_start, tmp_blk->pos_end - tmp_blk->pos_start + 1);
				math__sha1_hash_string((unsigned char *)blck, tmp_blk->pos_end - tmp_blk->pos_start + 1, hash);
				lua_pushstring(L, hash);
				return 1;
			} else {
				bvim_lua_error_raise
				    (L, "You need select valid block before SHA1 hash calculation!");
			}
		} else if (lua_type(L, 1) == LUA_TSTRING) {
			blck = (char *)malloc(strlen((char *)lua_tostring(L, 1)));
			blck = (char *)lua_tostring(L, 1);
			math__sha1_hash_string((unsigned char *)blck, strlen(blck), hash);
			lua_pushstring(L, hash);
			return 1;
		}
	} else {
		bvim_lua_error_raise(L, "Error in lua sha1_hash function! Wrong format!");
	}
	return 0;
}

/* Calculate SHA256 hash of buffer */
/* lua: sha256_hash(block_number) */
/* lua: sha256_hash("string") */
static int bvim_sha256_hash(lua_State * L)
{
	unsigned int id = 0;
	char *blck = NULL;
	struct block_item *tmp_blk;
	char hash[65];
	hash[0] = '\0';
	if (lua_gettop(L) == 1) {
		if (lua_type(L, 1) == LUA_TNUMBER) {
			id = (unsigned int)lua_tonumber(L, 1);
			tmp_blk = blocks__GetByID(id);
			if ((tmp_blk != NULL) & (tmp_blk->pos_end > tmp_blk->pos_start)) {
				blck = (char *)malloc(tmp_blk->pos_end - tmp_blk->pos_start + 1);
				memcpy(blck, core.editor.mem + tmp_blk->pos_start, tmp_blk->pos_end - tmp_blk->pos_start + 1);
				math__sha256_hash_string((unsigned char *)blck, tmp_blk->pos_end - tmp_blk->pos_start + 1, hash);
				lua_pushstring(L, hash);
				return 1;
			} else {
				bvim_lua_error_raise
				    (L, "You need select valid block before SHA256 hash calculation!");
			}
		} else if (lua_type(L, 1) == LUA_TSTRING) {
			blck = (char *)malloc(strlen((char *)lua_tostring(L, 1)));
			blck = (char *)lua_tostring(L, 1);
			math__sha256_hash_string((unsigned char *)blck, strlen(blck), hash);
			lua_pushstring(L, hash);
			return 1;
		}
	} else {
		bvim_lua_error_raise
		    (L, "Error in lua sha256_hash function! Wrong format!");
	}
	return 0;
}

/* Calculate SHA512 hash of buffer */
/* lua: sha512_hash(block_number) */
/* lua: sha512_hash("string") */
static int bvim_sha512_hash(lua_State * L)
{
	unsigned int id = 0;
	char *blck = NULL;
	struct block_item *tmp_blk;
	char hash[129];
	hash[0] = '\0';
	if (lua_gettop(L) == 1) {
		if (lua_type(L, 1) == LUA_TNUMBER) {
			id = (unsigned int)lua_tonumber(L, 1);
			tmp_blk = blocks__GetByID(id);
			if ((tmp_blk != NULL) & (tmp_blk->pos_end > tmp_blk->pos_start)) {
				blck = (char *)malloc(tmp_blk->pos_end - tmp_blk->pos_start + 1);
				memcpy(blck, core.editor.mem + tmp_blk->pos_start, tmp_blk->pos_end - tmp_blk->pos_start + 1);
				math__sha512_hash_string((unsigned char *)blck, tmp_blk->pos_end - tmp_blk->pos_start + 1, hash);
				lua_pushstring(L, hash);
				return 1;
			} else {
				bvim_lua_error_raise
				    (L, "You need select valid block before SHA512 hash calculation!");
			}
		} else if (lua_type(L, 1) == LUA_TSTRING) {
			blck = (char *)malloc(strlen((char *)lua_tostring(L, 1)));
			blck = (char *)lua_tostring(L, 1);
			math__sha512_hash_string((unsigned char *)blck, strlen(blck), hash);
			lua_pushstring(L, hash);
			return 1;
		}
	} else {
		bvim_lua_error_raise
		    (L, "Error in lua sha512_hash function! Wrong format!");
	}
	return 0;
}

/* Calculate RIPEMD160 hash of buffer */
/* lua: ripemd160_hash(block_number) */
/* lua: ripemd160_hash("string") */
static int bvim_ripemd160_hash(lua_State * L)
{
	unsigned int id = 0;
	char *blck = NULL;
	struct block_item *tmp_blk;
	char hash[65];
	hash[0] = '\0';
	if (lua_gettop(L) == 1) {
		if (lua_type(L, 1) == LUA_TNUMBER) {
			id = (unsigned int)lua_tonumber(L, 1);
			tmp_blk = blocks__GetByID(id);
			if ((tmp_blk != NULL) & (tmp_blk->pos_end > tmp_blk->pos_start)) {
				blck = (char *)malloc(tmp_blk->pos_end - tmp_blk->pos_start + 1);
				memcpy(blck, core.editor.mem + tmp_blk->pos_start, tmp_blk->pos_end - tmp_blk->pos_start + 1);
				math__ripemd160_hash_string((unsigned char *)blck, tmp_blk->pos_end - tmp_blk->pos_start + 1, hash);
				lua_pushstring(L, hash);
				return 1;
			} else {
				bvim_lua_error_raise
				    (L, "You need select valid block before RIPEMD160 hash calculation!");
			}
		} else if (lua_type(L, 1) == LUA_TSTRING) {
			blck = (char *)malloc(strlen((char *)lua_tostring(L, 1)));
			blck = (char *)lua_tostring(L, 1);
			math__ripemd160_hash_string((unsigned char *)blck, strlen(blck), hash);
			lua_pushstring(L, hash);
			return 1;
		}
	} else {
		bvim_lua_error_raise
		    (L, "Error in lua ripemd160_hash function! Wrong format!");
	}
	return 0;
}

/* Search byte sequence in the buffer */
/* Return address found */
/* lua: search_bytes(bytes) */
/* or search_bytes(bytes, return_all) where return_all = [0|1] */
static int bvim_search_bytes(lua_State * L)
{
	PTR result;
	PTR start_addr = NULL;
	char *bytes;
	char msgbuf[256];
	int i = 0;

	msgbuf[0] = '\0';
	start_addr = core.editor.mem;
	if (lua_gettop(L) == 1) {
		bytes = (char *)lua_tostring(L, -1);
		result =
		    searching('\\', bytes, start_addr, core.editor.maxpos - 1,
			      FALSE | S_GLOBAL);
		/*
		   sprintf(msgbuf, "Found [%s] at 0x%08x position", bytes, (long)(result - core.editor.mem));
		   wmsg(msgbuf, 3, strlen(msgbuf) + 4);
		 */
		lua_pushnumber(L, (long)(result - core.editor.mem));
		return 1;
	} else if (lua_gettop(L) == 2) {
		bytes = (char *)lua_tostring(L, 1);
		result = start_addr;
		if ((int)lua_tonumber(L, 2)) {
			while (result < core.editor.maxpos) {
				result =
				    searching('\\', bytes, start_addr,
					      core.editor.maxpos - 1, FALSE | S_GLOBAL);
				lua_pushnumber(L, (long)(result - core.editor.mem));
				i++;
			}
		}
		return i;
	}
	return 0;
}

/* Replace byte sequence to another in the buffer */
/* Return number of sustitutions */
/* lua: replace_bytes(original, target) */
static int bvim_replace_bytes(lua_State * L)
{
	int result = 0;
	PTR start_addr = NULL;
	char *bytes;
	char *target;
	char bytesbuf[256];
	bytesbuf[0] = '\0';
	start_addr = core.editor.mem;
	if (lua_gettop(L) == 2) {
		bytes = (char *)lua_tostring(L, 1);
		target = (char *)lua_tostring(L, 2);
		sprintf(bytesbuf, "/%s/%s/", bytes, target);
		result =
		    do_substitution('\\', bytesbuf, start_addr, core.editor.maxpos - 1);
		lua_pushnumber(L, result);
	}
	return 1;
}

/* Execute bvim cmd */
/* lua: exec(command) */
static int bvim_exec(lua_State * L)
{
	char *cmdline;
	if (lua_gettop(L) != 0) {
		cmdline = (char *)lua_tostring(L, -1);
		docmdline(cmdline);
	}
	return 0;
}

/* Display error */
/* lua: display_error(message) */
static int bvim_display_error(lua_State * L)
{
	char *message;
	if (lua_gettop(L) != 0) {
		message = (char *)lua_tostring(L, -1);
		bvim_error(state.mode, message);
	}
	return 0;
}

/* Display message in the status line */
/* lua: status_line_msg(message) */
static int bvim_status_line_msg(lua_State * L)
{
	char *message;
	if (lua_gettop(L) != 0) {
		message = (char *)lua_tostring(L, -1);
		bvim_info(state.mode, message);
	}
	return 0;
}

/* Display message in the window */
/* lua: msg_window(message) */
static int bvim_msg_window(lua_State * L)
{
	char *message;
	int height, width;
	int n = lua_gettop(L);
	if (n == 1) {
		message = (char *)lua_tostring(L, -1);
		ui__MsgWin_Show(message, 3, strlen(message) + 4);
	} else if (n == 3) {
		message = (char *)lua_tostring(L, 1);
		height = (int)lua_tonumber(L, 2);
		width = (int)lua_tonumber(L, 3);
		ui__MsgWin_Show(message, height, width);
	}
	return 0;
}

/* Display tools window */
/* lua: tools_window() */
static int bvim_tools_window(lua_State * L)
{
	int n = lua_gettop(L);
	if (n == 1) {
		ui__ToolWin_Show((int)lua_tonumber(L, -1));
	}
	return 0;
}

/* Display text in tools window */
/* lua: print_tools_window() */
static int bvim_print_tools_window(lua_State * L)
{
	int n = lua_gettop(L);
	if (n == 1) {
		ui__ToolWin_Print((char *)lua_tostring(L, -1), 1);
	} else if (n == 2) {
		ui__ToolWin_Print((char *)lua_tostring(L, 1),
				  (int)lua_tonumber(L, 2));
	}
	return 0;
}

/* Print color line in the any place of window */
/* lua: print(x, y, palette, string) */
static int bvim_print(lua_State * L)
{
	char *string;
	unsigned int x, y, palette;
	int n = lua_gettop(L);
	if (n == 4) {
		string = (char *)lua_tostring(L, 4);
		x = (unsigned int)lua_tonumber(L, 1);
		y = (unsigned int)lua_tonumber(L, 2);
		palette = (unsigned int)lua_tonumber(L, 3);
		if (palette > 16)
			palette = 1;
		printcolorline(x, y, palette, string);
	} else {
		bvim_lua_error_raise(L, "Wrong format of lua print() function!");
	}
	return 0;
}

/* Undo */
static int bvim_undo(lua_State * L)
{
	do_undo();
	return 0;
}

/* Redo */
static int bvim_redo(lua_State * L)
{
	return 0;
}

/* Set any bvim parameter (analog of :set param cmd) */
static int bvim_set(lua_State * L)
{
	return 0;
}

static int bvim_scrolldown(lua_State * L)
{
	int lines;
	if (lua_gettop(L) > 0) {
		lines = (int)lua_tonumber(L, 1);
	} else
		lines = 1;
	scrolldown(lines);
	return 0;
}

static int bvim_scrollup(lua_State * L)
{
	int lines;
	if (lua_gettop(L) > 0) {
		lines = (int)lua_tonumber(L, 1);
	} else
		lines = 1;
	scrollup(lines);
	return 0;
}

/* Insert count of bytes at position */
static int bvim_insert(lua_State * L)
{
	return 0;
}

/* Remove count of bytes from position */
static int bvim_remove(lua_State * L)
{
	return 0;
}

/* Get current cursor position */
static int bvim_cursor(lua_State * L)
{
	return 0;
}

/* Adds marker to show in address */
static int bvim_set_marker(lua_State * L)
{
	long address;
	int i = 0;
	if (lua_gettop(L) > 0) {
		address = (long)lua_tonumber(L, 1);
		while ((markers[i].address != 0) & (i < MARK_COUNT))
			i++;
		markers[i].address = address;
		markers[i].marker = '+';
		if (state.mode != BVI_MODE_REPL)
			ui__Screen_Repaint();
	}
	return 0;
}

/* Remove marker */
static int bvim_unset_marker(lua_State * L)
{
	long address;
	int i = 0;
	if (lua_gettop(L) > 0) {
		address = (long)lua_tonumber(L, 1);
		while (i < MARK_COUNT) {
			if (markers[i].address == address)
				break;
		}
		if (markers[i].address == address) {
			markers[i].address = 0;
			markers[i].marker = ' ';
			if (state.mode != BVI_MODE_REPL)
				ui__Screen_Repaint();
		} else
			bvim_lua_error_raise(L, "Cant found mark on this address!");
	}
	return 0;
}

/* Display arbitrary address on the screen */
static int bvim_setpage(lua_State * L)
{
	int address;
	if (lua_gettop(L) > 0) {
		address = (int)lua_tonumber(L, 1);
	} else
		address = 0;
	setpage(core.editor.mem + address);
	return 0;
}

/* ================ LUA REPL ================ */

// TODO: add socket server output mode, write simple client
// TODO: add buffering output, store all output in buffer
//
// insert io__BufferAdd, io__RecordInsert, io__RecordGet
static int bvim_repl_print(lua_State *L)
{
	iorecord_t rec;
	const char *s;

	int n = lua_gettop(L);
	int i;
	lua_getglobal(L, "tostring");
	for (i = 1; i <= n; i++) {
		lua_pushvalue(L, -1); // function to be called
		lua_pushvalue(L, i); // value to print
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1); // get result
		if (s == NULL)
			//return lua_Lerror(L, LUA_QL("tostring") " must return a string to" LUA_QL("print"));
			return -1;

		// Change this to REPL output !!!
		rec.input = "print(...)";
		rec.output = s;
		io__RecordInsert(BVI_BUFFER_REPL, rec);

		if (state.mode == BVI_MODE_REPL) {
			if (i > 1) ui__REPLWin_print("\t");
			ui__REPLWin_print(s);
		}

		lua_pop(L, 1); // pop result
	}

	// Change this to REPL output !!!
	//ui__REPLWin_print("\n");
	return 0;
}

static int bvim_repl_clear(lua_State *L)
{
	ui__REPLWin_clear();
	return 0;
}

/* -------------------------------------------------------------------------------------
 *
 *  Initialization of Lua scripting support and loading plugins ...
 *
 * -------------------------------------------------------------------------------------
 */

/* =============== Lua functions list/buffer abstractions =============== */

/* Preallocate some memory for future lua functions allocation */
int InitLuaFunctionsList(int N)
{
	int i = 0;
	core.luaF_list = (luaF_link)malloc((N + 1)*(sizeof *(core.luaF_list)));
	for (i = 0; i < N + 1; i++) {
		core.luaF_list[i].next = &(core.luaF_list[i + 1]);
	}
	core.luaF_list[N].next = NULL;
	return 0;
}

int LuaFunctionAdd(struct luaF_item i)
{
	luaF_link t = NULL;
	t = (luaF_link)malloc(sizeof(*t));
	if (t != NULL) {
		t->item = i;
		if (core.luaF_list != NULL) {
			t->next = core.luaF_list->next;
			core.luaF_list->next = t;
		} else {
			core.luaF_list = t;
			core.luaF_list->next = NULL;
		}
	} else {
		return -1;
	}
	return 0;
}

void LuaFunctionInsertNext(luaF_link x, luaF_link t)
{
	t->next = x->next;
	x->next = t;
}

void LuaFunctionFree(luaF_link x)
{
	LuaFunctionInsertNext(core.luaF_list, x);
}

luaF_link LuaFunctionDeleteNext(luaF_link x)
{
	luaF_link t =  NULL;
	if (x != NULL) {
		t = x->next;
		x->next = t->next;
	}
	return t;
}

luaF_link LuaFunctionNext(luaF_link x)
{
	return x->next;
}

struct luaF_item LuaFunctionGet(luaF_link x)
{
	return x->item;
}

/* ============= Lua functions list interface ================ */

/* Iterator of any functions on functions list,
 * where result - expected result for function
 * All blocks are unique */
int luaF_Iterator(int (*(func))(), int result)
{
	luaF_link t;
	t = core.luaF_list;
	while (t != NULL)
	{
		if ((*(func))(&(t->item)) == result) {
			return 0;
		}
		t = t->next;
	}
	return -1;
}

int luaF_Add(struct luaF_item b) {
	
	LuaFunctionAdd(b);
	lua_register(lstate, b.name, b.handler.func);
	return 0;
}

int luaF_DelByID(int id) {
	return 0;
}

int luaF_DelByName(char* name) {
	return 0;
}

struct luaF_item* luaF_GetByID(unsigned int id) {
	luaF_link t;
	
	t = core.luaF_list;
	
	while (t != NULL)
	{
		if (t->item.id == id) return &(t->item);
		t = t->next;
	}
	return NULL;
}

struct luaF_item* luaF_GetByName(char* name) {
	luaF_link t;

	t = core.luaF_list;

	while (t != NULL)
	{
		if (!strcmp(t->item.name, name)) return &(t->item);
		t = t->next;
	}
	return NULL;
}

int luaF_Init()
{
	//InitBlocksList(10);
	return 0;
}

int luaF_Destroy()
{
	luaF_link t;
	t = core.luaF_list;
	while (t != NULL)
	{
		free(t);
		t = t->next;
	}
	return 0;
}

void bvim_lua_init()
{
	
	struct luaL_reg std_methods[] = {
		{"print", bvim_repl_print},
		{"clear", bvim_repl_clear},
		{NULL, NULL}
	};

	struct luaL_reg bvim_methods[] = {
		{"command_add", bvim_command_add},
		{"command_del", bvim_command_del},
		{"save", bvim_save},
		{"load", bvim_load},
		{"file", bvim_file},
		{"exec", bvim_exec},
		{"plugin_load", bvim_plugin_load},
		{"plugin_list", bvim_plugin_list},
		{"plugin_info", bvim_plugin_info},
		{"block_select", bvim_block_select},
		{"block_fold", bvim_block_fold},
		{"block_read", bvim_block_read},
		{"block_and", bvim_block_and},
		{"block_or", bvim_block_or},
		{"block_xor", bvim_block_xor},
		{"block_lshift", bvim_block_lshift},
		{"block_rshift", bvim_block_rshift},
		{"block_lrotate", bvim_block_lrotate},
		{"block_rrotate", bvim_block_rrotate},
		{"search_bytes", bvim_search_bytes},
		{"replace_bytes", bvim_replace_bytes},
		{"display_error", bvim_display_error},
		{"display_status_msg", bvim_status_line_msg},
		{"msg_window", bvim_msg_window},
		{"tools_window", bvim_tools_window},
		{"print_tools_window", bvim_print_tools_window},
		{"print", bvim_print},
		{"set", bvim_set},
		{"undo", bvim_undo},
		{"redo", bvim_redo},
		{"insert", bvim_insert},
		{"remove", bvim_remove},
		{"cursor", bvim_cursor},
		{"scrolldown", bvim_scrolldown},
		{"scrollup", bvim_scrollup},
		{"set_marker", bvim_set_marker},
		{"unset_marker", bvim_unset_marker},
		{"setpage", bvim_setpage},
		{NULL, NULL}
	};

	struct luaL_reg hash_methods[] = {
		{"crc16", bvim_crc16},
		{"crc32", bvim_crc32},
		{"md4", bvim_md4_hash},
		{"md5", bvim_md5_hash},
		{"sha1", bvim_sha1_hash},
		{"sha256", bvim_sha256_hash},
		{"sha512", bvim_sha512_hash},
		{"ripemd160", bvim_ripemd160_hash},
		{NULL, NULL}
	};

	struct luaL_reg stat_methods[] = {
		{"entropy", bvim_entropy},
		{NULL, NULL}
	};

	io__BufferAdd2(BVI_BUFFER_REPL);
	lstate = lua_open();
	luaL_openlibs(lstate);
	lua_pushvalue(lstate, LUA_GLOBALSINDEX);
	luaL_register(lstate, 0, std_methods);
	luaL_register(lstate, "bvim", bvim_methods);
	luaL_register(lstate, "hash", hash_methods);
	luaL_register(lstate, "stat", stat_methods);
	lua_atpanic(lstate, bvim_lua_error_handler);
	
	/* export some important constants */
	lua_setConst(lstate, BVI_VISUAL_SELECTION_ID);
	lua_setConst(lstate, BVI_MODE_CMD);
	lua_setConst(lstate, BVI_MODE_EDIT);
	lua_setConst(lstate, BVI_MODE_VISUAL);
	lua_setConst(lstate, BVI_MODE_REPL);
	luaF_Init();
	/* loading init.lua script */
	bvim_run_lua_script("init");
}

// TODO: Add support of full path, multiple plugins directories
int bvim_run_lua_script(char *name)
{
	char filename[256];
	int err;
	filename[0] = '\0';
	strcat(filename, LUA_DEFAULT_SCRIPT_PATH);
	strcat(filename, "/");
	strcat(filename, name);
	strcat(filename, ".lua");
	err = luaL_loadfile(lstate, filename);
	if (err) {
		bvim_error(BVI_MODE_EDIT, "Can't open lua script: %s", lua_tostring(lstate, -1));
		lua_pop(lstate, 1);
	} else {
		lua_pcall(lstate, 0, LUA_MULTRET, 0);
	}
	return 0;
}

int bvim_run_lua_string(char *string)
{
	int err = luaL_loadstring(lstate, string);
	if (err) {
		bvim_error(BVI_MODE_EDIT, "Can't run lua command: %s", lua_tostring(lstate, -1));
		lua_pop(lstate, 1);
	} else {
		lua_pcall(lstate, 0, LUA_MULTRET, 0);
	}
	return 0;
}

char* bvim_repl_init()
{
	char* replbuf = malloc(4096);
	return replbuf;
}

int bvim_repl_read()
{
	return 0;
}

int bvim_repl_eval(char *line)
{
	lua_pushcfunction(lstate, bvim_lua_error_handler);
	int err = luaL_loadstring(lstate, line);
	if (err) {
		bvim_lua_error_raise(lstate, "Lua error: %s", lua_tostring(lstate, -1));
		lua_pop(lstate, 1);
	} else {
		lua_pcall(lstate, 0, LUA_MULTRET, 0);
		lua_pop(lstate, 1);
	}

	return 0;
}

void bvim_lua_finish()
{
	luaF_Destroy();
	lua_close(lstate);
	io__BufferDestroy(BVI_BUFFER_REPL);
}
