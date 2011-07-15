/* Include the Lua API header files. */
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "bvi.h"
#include "set.h"
#include "util.h"
#include "bscript.h"

lua_State *lstate;

extern struct BLOCK_ data_block[BLK_COUNT];

/* Save current buffer into file */
/* lua: save(filename, [start, end, flags]) */
static int bvi_save(lua_State * L)
{
	/* save(filename, start, end, flags); */
	return 0;
}

/* Load file into current buffer */
/* lua: load(filename, [mode]) */
static int bvi_load(lua_State * L)
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
static int bvi_file(lua_State * L)
{
	char *filename = "dummy";
	lua_pushstring(L, filename);
	return 1;
}

/* Select block in the buffer */
/* lua: block_select(block_number, start, end, pattern) */
static int bvi_block_select(lua_State *L)
{
	unsigned int n = 0;
	if (lua_gettop(L) == 4) {
		n = (unsigned int)lua_tonumber(L, 1);
		if (n < BLK_COUNT) {
			data_block[n].pos_start = (unsigned long)lua_tonumber(L, 2);
			data_block[n].pos_end = (unsigned long)lua_tonumber(L, 3);
			data_block[n].palette = (unsigned int)lua_tonumber(L, 4);
			data_block[n].hl_toggle = 1;
			repaint();
		} else {
			emsg("Wrong block number! Too big!");
		}
	} else {
		emsg("Error in lua block_select function! Wrong format!");
	}
	return 0;
}

/* Read block in the buffer */
/* lua: block_read(block_number) */
static int bvi_block_read(lua_State *L)
{
	unsigned int n = 0;
	char* block = NULL;
	if (lua_gettop(L) == 1) {
		n = (unsigned int)lua_tonumber(L, 1);
		if (data_block[n].pos_end > data_block[n].pos_start) {
			/*block = (void *)lua_newuserdata(L, data_block[n].pos_end - data_block[n].pos_start);*/
			block = (char*)malloc(data_block[n].pos_end - data_block[n].pos_start + 2);
			strncpy(block, mem + data_block[n].pos_start, data_block[n].pos_end - data_block[n].pos_start);
			strcat(block, '\0');
			lua_pushstring(L, block);
		} else {
			emsg("You need select valid block before read!");
		}
	} else {
		emsg("Error in lua block_read function! Wrong format!");
	}
	return 1;
}

/* Calculate SHA1 hash of buffer */
/* lua: sha1_hash(block_number) */
static int bvi_sha1_hash(lua_State *L)
{
	unsigned int n = 0;
	char* block = NULL;
	char hash[65];
	hash[0] = '\0';
	if (lua_gettop(L) == 1) {
		n = (unsigned int)lua_tonumber(L, 1);
		if (data_block[n].pos_end > data_block[n].pos_start) {
			block = (char *)malloc(data_block[n].pos_end - data_block[n].pos_start);
			memcpy(block, *(&mem + data_block[n].pos_start), data_block[n].pos_end - data_block[n].pos_start);
			sha1_hash_string(block, hash);
			lua_pushstring(L, hash);
		} else {
			emsg("You need select valid block before SHA1 hash calculation!");
		}
	} else {
		emsg("Error in lua sha1_hash function! Wrong format!");
	}
	return 1;
}

/* Search byte sequence in the buffer */
/* Return address found */
/* lua: search_bytes(bytes) */
static int bvi_search_bytes(lua_State *L)
{
	PTR result;
	char* bytes;
	char msgbuf[256];
	msgbuf[0] = '\0';
	if (lua_gettop(L) == 1) {
		bytes = (char *)lua_tostring(L, -1);
		result = searching('\\', bytes, 0, maxpos - 1, FALSE | S_GLOBAL);
		sprintf(msgbuf, "Found [%s] at %ld position", bytes, (long)result);
		wmsg(msgbuf, 3, strlen(msgbuf) + 4);
		lua_pushstring(L, result);
	}
	return 1;
}

/* Replace byte sequence to another in the buffer */
/* Return number of sustitutions */
/* lua: replace_bytes(original, target) */
static int bvi_replace_bytes(lua_State *L)
{
	int result = 0;
	char* bytes;
	char* target;
	char bytesbuf[256];
	bytesbuf[0] = '\0';
	if (lua_gettop(L) == 2) {
		bytes = (char *)lua_tostring(L, 1);
		target = (char *)lua_tostring(L, 2);
		sprintf(bytesbuf, "/%s/%s/", bytes, target);
		result = do_substitution('\\', bytesbuf, 0, maxpos - 1);
		lua_pushnumber(L, result);
	}
	return 1;
}

/* Execute bvi cmd */
/* lua: exec(command) */
static int bvi_exec(lua_State * L)
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
static int bvi_display_error(lua_State * L)
{
	char *message;
	if (lua_gettop(L) != 0) {
		message = (char *)lua_tostring(L, -1);
		emsg(message);
	}
	return 0;
}

/* Display message in the status line */
/* lua: status_line_msg(message) */
static int bvi_status_line_msg(lua_State * L)
{
	char *message;
	if (lua_gettop(L) != 0) {
		message = (char *)lua_tostring(L, -1);
		msg(message);
	}
	return 0;
}

/* Display message in the window */
/* lua: msg_window(message) */
static int bvi_msg_window(lua_State * L)
{
	char *message;
	int height, width;
	int n = lua_gettop(L);
	if (n == 1) {
		message = (char *)lua_tostring(L, -1);
		wmsg(message, 3, strlen(message) + 4);
	} else if (n == 3) {
		message = (char *)lua_tostring(L, 1);
		height = (int)lua_tonumber(L, 2);
		width = (int)lua_tonumber(L, 3);
		wmsg(message, height, width);
	}
	return 0;
}

/* Print color line in the any place of window */
/* lua: print(x, y, palette, string) */
static int bvi_print(lua_State * L)
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
		emsg("Wrong format of lua print() function!");
	}
	return 0;
}

/* Undo */
static int bvi_undo(lua_State * L)
{
	do_undo();
	return 0;
}

/* Redo */
static int bvi_redo(lua_State * L)
{
	return 0;
}

/* Set any bvi parameter (analog of :set param cmd) */
static int bvi_set(lua_State * L)
{
	return 0;
}

static int bvi_scrolldown(lua_State * L)
{
	int lines;
	if (lua_gettop(L) > 0) {
		lines = (int)lua_tonumber(L, 1);
	} else
		lines = 1;
	scrolldown(lines);
	return 0;
}

static int bvi_scrollup(lua_State * L)
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
static int bvi_insert(lua_State * L)
{
	return 0;
}

/* Remove count of bytes from position */
static int bvi_remove(lua_State * L)
{
	return 0;
}

/* Get current cursor position */
static int bvi_cursor(lua_State * L)
{
	return 0;
}

/* Display arbitrary address on the screen */
static int bvi_setpage(lua_State * L)
{
	int address;
	if (lua_gettop(L) > 0) {
		address = (int)lua_tonumber(L, 1);
	} else
		address = 0;
	setpage(address);
	return 0;
}

/* -------------------------------------------------------------------------------------
 *
 *  Initialization of Lua scripting support and loading plugins ...
 *
 * -------------------------------------------------------------------------------------
 */

void bvi_lua_init()
{
	struct luaL_reg bvi_methods[] = {
		{"save", bvi_save},
		{"load", bvi_load},
		{"file", bvi_file},
		{"exec", bvi_exec},
		{"block_select", bvi_block_select},
		{"block_read", bvi_block_read},
		{"sha1_hash", bvi_sha1_hash},
		{"search_bytes", bvi_search_bytes},
		{"replace_bytes", bvi_replace_bytes},
		{"display_error", bvi_display_error},
		{"display_status_msg", bvi_status_line_msg},
		{"msg_window", bvi_msg_window},
		{"print", bvi_print},
		{"set", bvi_set},
		{"undo", bvi_undo},
		{"redo", bvi_redo},
		{"insert", bvi_insert},
		{"remove", bvi_remove},
		{"cursor", bvi_cursor},
		{"scrolldown", bvi_scrolldown},
		{"scrollup", bvi_scrollup},
		{"setpage", bvi_setpage},
		{NULL, NULL}
	};
	lstate = lua_open();
	luaL_openlibs(lstate);
	luaL_register(lstate, "bvi", bvi_methods);
}

int bvi_run_lua_script(char *name)
{
	char filename[256];
	int err;
	filename[0] = '\0';
	strcat(filename, "plugins/");
	strcat(filename, name);
	strcat(filename, ".lua");
	err = luaL_loadfile(lstate, filename);
	if (err) {
		emsg("Error: cant open lua script file!");
		lua_pop(lstate, 1);
	} else {
		lua_pcall(lstate, 0, LUA_MULTRET, 0);
	}
	return 0;
}

int bvi_run_lua_string(char *string)
{
	int err = luaL_loadstring(lstate, string);
	if (err) {
		emsg("Error in lua script!");
		lua_pop(lstate, 1);
	} else {
		lua_pcall(lstate, 0, LUA_MULTRET, 0);
	}
	return 0;
}

void bvi_lua_finish()
{
	lua_close(lstate);
}
