/* Include the Lua API header files. */
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "bvi.h"
#include "set.h"
#include "math.h"
#include "bscript.h"
#include "ui.h"
#include "plugins.h"

lua_State *lstate;

extern struct BLOCK_ data_block[BLK_COUNT];
extern struct MARKERS_ markers[MARK_COUNT];
extern WINDOW *tools_win;
extern PTR mem;

/* -----------------------------------------------------------------------------
 *                       Interface for plugins API
 * -----------------------------------------------------------------------------
 */

static int bvi_add_lua_function()
{
	return 0;
}

static int bvi_remove_lua_function()
{
	return 0;
}

/* -----------------------------------------------------------------------------
 */

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

/* TODO: add function , which returns list of all presented blocks */

/* Select block in the buffer */
/* lua: block_select(block_number, start, end, pattern) */
static int bvi_block_select(lua_State * L)
{
	unsigned int n = 0;
	if (lua_gettop(L) == 4) {
		n = (unsigned int)lua_tonumber(L, 1);
		if (n < BLK_COUNT) {
			data_block[n].pos_start =
			    (unsigned long)lua_tonumber(L, 2);
			data_block[n].pos_end =
			    (unsigned long)lua_tonumber(L, 3);
			data_block[n].palette =
			    (unsigned int)lua_tonumber(L, 4);
			data_block[n].hl_toggle = 1;
			ui__Screen_Repaint();
		} else {
			ui__ErrorMsg("Wrong block number! Too big!");
		}
	} else {
		ui__ErrorMsg
		    ("Error in lua block_select function! Wrong format!");
	}
	return 0;
}

/* Fold block in the buffer */
/* lua: block_fold(block_number) */
static int bvi_block_fold(lua_State * L)
{
	unsigned int n = 0;
	if (lua_gettop(L) == 1) {
		n = (unsigned int)lua_tonumber(L, 1);
		if ((n <
		     BLK_COUNT) & ((data_block[n].pos_end -
				    data_block[n].pos_start) > 0)) {
			data_block[n].folding = 1;
			ui__Screen_Repaint();
		} else {
			ui__ErrorMsg("Wrong block number!");
		}
	} else {
		ui__ErrorMsg("Error in lua block_fold function! Wrong format!");
	}
	return 0;
}

/* Read block in the buffer */
/* lua: block_read(block_number) */
static int bvi_block_read(lua_State * L)
{
	unsigned int n = 0;
	char *block = NULL;
	if (lua_gettop(L) == 1) {
		n = (unsigned int)lua_tonumber(L, 1);
		if (data_block[n].pos_end > data_block[n].pos_start) {
			/*block = (void *)lua_newuserdata(L, data_block[n].pos_end - data_block[n].pos_start); */
			block =
			    (char *)malloc(data_block[n].pos_end -
					   data_block[n].pos_start + 1);
			memcpy(block, start_addr + data_block[n].pos_start,
			       data_block[n].pos_end - data_block[n].pos_start +
			       1);
			lua_pushstring(L, block);
		} else {
			ui__ErrorMsg
			    ("You need select valid block before read!");
		}
	} else {
		ui__ErrorMsg("Error in lua block_read function! Wrong format!");
	}
	return 1;
}

/* AND bit operation on block */
/* lua: block_and(block_number, mask) */
static int bvi_block_and(lua_State * L)
{
	unsigned int n = 0;
	if (lua_gettop(L) == 2) {
		n = (unsigned int)lua_tonumber(L, 1);
		if (data_block[n].pos_end > data_block[n].pos_start) {
			do_logic_block(AND, (char *)lua_tostring(L, 2), n);
		} else {
			ui__ErrorMsg
			    ("You need select valid block before and operation!");
		}
	} else {
		ui__ErrorMsg("Error in lua block_and function! Wrong format!");
	}
	return 0;
}

/* OR bit operation on block */
/* lua: block_or(block_number, mask) */
static int bvi_block_or(lua_State * L)
{
	unsigned int n = 0;
	if (lua_gettop(L) == 2) {
		n = (unsigned int)lua_tonumber(L, 1);
		if (data_block[n].pos_end > data_block[n].pos_start) {
			do_logic_block(OR, (char *)lua_tostring(L, 2), n);
		} else {
			ui__ErrorMsg
			    ("You need select valid block before or operation!");
		}
	} else {
		ui__ErrorMsg("Error in lua block_and function! Wrong format!");
	}
	return 0;
}

/* XOR bit operation on block */
/* lua: block_xor(block_number, mask) */
static int bvi_block_xor(lua_State * L)
{
	unsigned int n = 0;
	if (lua_gettop(L) == 2) {
		n = (unsigned int)lua_tonumber(L, 1);
		if (data_block[n].pos_end > data_block[n].pos_start) {
			do_logic_block(XOR, (char *)lua_tostring(L, 2), n);
		} else {
			ui__ErrorMsg
			    ("You need select valid block before xor operation!");
		}
	} else {
		ui__ErrorMsg("Error in lua block_and function! Wrong format!");
	}
	return 0;
}

/* LSHIFT bit operation on block */
/* lua: block_lshift(block_number, count) */
static int bvi_block_lshift(lua_State * L)
{
	unsigned int n = 0;
	if (lua_gettop(L) == 2) {
		n = (unsigned int)lua_tonumber(L, 1);
		if (data_block[n].pos_end > data_block[n].pos_start) {
			do_logic_block(LSHIFT, (char *)lua_tostring(L, 2), n);
		} else {
			ui__ErrorMsg
			    ("You need select valid block before lshift operation!");
		}
	} else {
		ui__ErrorMsg("Error in lua block_and function! Wrong format!");
	}
	return 0;
}

/* RSHIFT bit operation on block */
/* lua: block_rshift(block_number, count) */
static int bvi_block_rshift(lua_State * L)
{
	unsigned int n = 0;
	if (lua_gettop(L) == 2) {
		n = (unsigned int)lua_tonumber(L, 1);
		if (data_block[n].pos_end > data_block[n].pos_start) {
			do_logic_block(RSHIFT, (char *)lua_tostring(L, 2), n);
		} else {
			ui__ErrorMsg
			    ("You need select valid block before rshift operation!");
		}
	} else {
		ui__ErrorMsg("Error in lua block_and function! Wrong format!");
	}
	return 0;
}

/* LROTATE bit operation on block */
/* lua: block_lrotate(block_number, count) */
static int bvi_block_lrotate(lua_State * L)
{
	unsigned int n = 0;
	if (lua_gettop(L) == 2) {
		n = (unsigned int)lua_tonumber(L, 1);
		if (data_block[n].pos_end > data_block[n].pos_start) {
			do_logic_block(LROTATE, (char *)lua_tostring(L, 2), n);
		} else {
			ui__ErrorMsg
			    ("You need select valid block before lrotate operation!");
		}
	} else {
		ui__ErrorMsg("Error in lua block_and function! Wrong format!");
	}
	return 0;
}

/* RROTATE bit operation on block */
/* lua: block_rrotate(block_number, count) */
static int bvi_block_rrotate(lua_State * L)
{
	unsigned int n = 0;
	if (lua_gettop(L) == 2) {
		n = (unsigned int)lua_tonumber(L, 1);
		if (data_block[n].pos_end > data_block[n].pos_start) {
			do_logic_block(RROTATE, (char *)lua_tostring(L, 2), n);
		} else {
			ui__ErrorMsg
			    ("You need select valid block before rrotate operation!");
		}
	} else {
		ui__ErrorMsg("Error in lua block_and function! Wrong format!");
	}
	return 0;
}

/* Calculate CRC16 checksum of buffer */
/* lua: crc16(block_number) */
/* lua: crc16("string") */
static int bvi_crc16(lua_State * L)
{
	unsigned int n = 0;
	char *block = NULL;
	unsigned int crc = 0;
	if (lua_gettop(L) == 1) {
		if (lua_type(L, 1) == LUA_TNUMBER) {
			n = (unsigned int)lua_tonumber(L, 1);
			if (data_block[n].pos_end > data_block[n].pos_start) {
				block =
				    (char *)malloc(data_block[n].pos_end -
						   data_block[n].pos_start + 1);
				memcpy(block, mem + data_block[n].pos_start,
				       data_block[n].pos_end -
				       data_block[n].pos_start + 1);
				crc =
				    crc16(block,
					  data_block[n].pos_end -
					  data_block[n].pos_start + 1, crc);
				lua_pushnumber(L, crc);
				return 1;
			}
		} else if (lua_type(L, 1) == LUA_TSTRING) {
			block =
			    (char *)malloc(strlen((char *)lua_tostring(L, 1)));
			block = (char *)lua_tostring(L, 1);
			crc = crc16(block, strlen(block), crc);
			lua_pushnumber(L, crc);
			return 1;
		}
	} else {
		ui__ErrorMsg("Error in lua crc16 function! Wrong format!");
	}
	return 0;
}

/* Calculate CRC32 checksum of buffer */
/* lua: crc32(block_number) */
/* lua: crc32("string") */
static int bvi_crc32(lua_State * L)
{
	unsigned int n = 0;
	char *block = NULL;
	unsigned int crc = 0;
	if (lua_gettop(L) == 1) {
		if (lua_type(L, 1) == LUA_TNUMBER) {
			n = (unsigned int)lua_tonumber(L, 1);
			if (data_block[n].pos_end > data_block[n].pos_start) {
				block =
				    (char *)malloc(data_block[n].pos_end -
						   data_block[n].pos_start + 1);
				memcpy(block, mem + data_block[n].pos_start,
				       data_block[n].pos_end -
				       data_block[n].pos_start + 1);
				crc =
				    crc32(block,
					  data_block[n].pos_end -
					  data_block[n].pos_start + 1, crc);
				lua_pushnumber(L, crc);
				return 1;
			}
		} else if (lua_type(L, 1) == LUA_TSTRING) {
			block =
			    (char *)malloc(strlen((char *)lua_tostring(L, 1)));
			block = (char *)lua_tostring(L, 1);
			crc = crc32(block, strlen(block), crc);
			lua_pushnumber(L, crc);
			return 1;
		}
	} else {
		ui__ErrorMsg("Error in lua crc32 function! Wrong format!");
	}
	return 0;
}

/* Calculate MD4 hash of buffer */
/* lua: md4_hash(block_number) */
/* lua: md4_hash("string") */
static int bvi_md4_hash(lua_State * L)
{
	unsigned int n = 0;
	char *block = NULL;
	char hash[65];
	hash[0] = '\0';
	if (lua_gettop(L) == 1) {
		if (lua_type(L, 1) == LUA_TNUMBER) {
			n = (unsigned int)lua_tonumber(L, 1);
			if (data_block[n].pos_end > data_block[n].pos_start) {
				block =
				    (char *)malloc(data_block[n].pos_end -
						   data_block[n].pos_start + 1);
				memcpy(block, mem + data_block[n].pos_start,
				       data_block[n].pos_end -
				       data_block[n].pos_start + 1);
				md4_hash_string((unsigned char *)block,
						data_block[n].pos_end -
						data_block[n].pos_start + 1,
						hash);
				lua_pushstring(L, hash);
				return 1;
			} else {
				ui__ErrorMsg
				    ("You need select valid block before MD4 hash calculation!");
			}
		} else if (lua_type(L, 1) == LUA_TSTRING) {
			block =
			    (char *)malloc(strlen((char *)lua_tostring(L, 1)));
			block = (char *)lua_tostring(L, 1);
			md4_hash_string((unsigned char *)block, strlen(block),
					hash);
			lua_pushstring(L, hash);
			return 1;
		}
	} else {
		ui__ErrorMsg("Error in lua md4_hash function! Wrong format!");
	}
	return 0;
}

/* Calculate MD5 hash of buffer */
/* lua: md5_hash(block_number) */
/* lua: md5_hash("string") */
static int bvi_md5_hash(lua_State * L)
{
	unsigned int n = 0;
	char *block = NULL;
	char hash[65];
	hash[0] = '\0';
	if (lua_gettop(L) == 1) {
		if (lua_type(L, 1) == LUA_TNUMBER) {
			n = (unsigned int)lua_tonumber(L, 1);
			if (data_block[n].pos_end > data_block[n].pos_start) {
				block =
				    (char *)malloc(data_block[n].pos_end -
						   data_block[n].pos_start + 1);
				memcpy(block, mem + data_block[n].pos_start,
				       data_block[n].pos_end -
				       data_block[n].pos_start + 1);
				md5_hash_string((unsigned char *)block,
						data_block[n].pos_end -
						data_block[n].pos_start + 1,
						hash);
				lua_pushstring(L, hash);
				return 1;
			} else {
				ui__ErrorMsg
				    ("You need select valid block before MD5 hash calculation!");
			}
		} else if (lua_type(L, 1) == LUA_TSTRING) {
			block =
			    (char *)malloc(strlen((char *)lua_tostring(L, 1)));
			block = (char *)lua_tostring(L, 1);
			md5_hash_string((unsigned char *)block, strlen(block),
					hash);
			lua_pushstring(L, hash);
			return 1;
		}
	} else {
		ui__ErrorMsg("Error in lua md5_hash function! Wrong format!");
	}
	return 0;
}

/* Calculate SHA1 hash of buffer */
/* lua: sha1_hash(block_number) */
/* lua: sha1_hash("string") */
static int bvi_sha1_hash(lua_State * L)
{
	unsigned int n = 0;
	char *block = NULL;
	char hash[65];
	hash[0] = '\0';
	if (lua_gettop(L) == 1) {
		if (lua_type(L, 1) == LUA_TNUMBER) {
			n = (unsigned int)lua_tonumber(L, 1);
			if (data_block[n].pos_end > data_block[n].pos_start) {
				block =
				    (char *)malloc(data_block[n].pos_end -
						   data_block[n].pos_start + 1);
				memcpy(block, mem + data_block[n].pos_start,
				       data_block[n].pos_end -
				       data_block[n].pos_start + 1);
				sha1_hash_string((unsigned char *)block,
						 data_block[n].pos_end -
						 data_block[n].pos_start + 1,
						 hash);
				lua_pushstring(L, hash);
				return 1;
			} else {
				ui__ErrorMsg
				    ("You need select valid block before SHA1 hash calculation!");
			}
		} else if (lua_type(L, 1) == LUA_TSTRING) {
			block =
			    (char *)malloc(strlen((char *)lua_tostring(L, 1)));
			block = (char *)lua_tostring(L, 1);
			sha1_hash_string((unsigned char *)block, strlen(block),
					 hash);
			lua_pushstring(L, hash);
			return 1;
		}
	} else {
		ui__ErrorMsg("Error in lua sha1_hash function! Wrong format!");
	}
	return 0;
}

/* Calculate SHA256 hash of buffer */
/* lua: sha256_hash(block_number) */
/* lua: sha256_hash("string") */
static int bvi_sha256_hash(lua_State * L)
{
	unsigned int n = 0;
	char *block = NULL;
	char hash[65];
	hash[0] = '\0';
	if (lua_gettop(L) == 1) {
		if (lua_type(L, 1) == LUA_TNUMBER) {
			n = (unsigned int)lua_tonumber(L, 1);
			if (data_block[n].pos_end > data_block[n].pos_start) {
				block =
				    (char *)malloc(data_block[n].pos_end -
						   data_block[n].pos_start + 1);
				memcpy(block, mem + data_block[n].pos_start,
				       data_block[n].pos_end -
				       data_block[n].pos_start + 1);
				sha256_hash_string((unsigned char *)block,
						   data_block[n].pos_end -
						   data_block[n].pos_start + 1,
						   hash);
				lua_pushstring(L, hash);
				return 1;
			} else {
				ui__ErrorMsg
				    ("You need select valid block before SHA256 hash calculation!");
			}
		} else if (lua_type(L, 1) == LUA_TSTRING) {
			block =
			    (char *)malloc(strlen((char *)lua_tostring(L, 1)));
			block = (char *)lua_tostring(L, 1);
			sha256_hash_string((unsigned char *)block,
					   strlen(block), hash);
			lua_pushstring(L, hash);
			return 1;
		}
	} else {
		ui__ErrorMsg
		    ("Error in lua sha256_hash function! Wrong format!");
	}
	return 0;
}

/* Calculate SHA512 hash of buffer */
/* lua: sha512_hash(block_number) */
/* lua: sha512_hash("string") */
static int bvi_sha512_hash(lua_State * L)
{
	unsigned int n = 0;
	char *block = NULL;
	char hash[129];
	hash[0] = '\0';
	if (lua_gettop(L) == 1) {
		if (lua_type(L, 1) == LUA_TNUMBER) {
			n = (unsigned int)lua_tonumber(L, 1);
			if (data_block[n].pos_end > data_block[n].pos_start) {
				block =
				    (char *)malloc(data_block[n].pos_end -
						   data_block[n].pos_start + 1);
				memcpy(block, mem + data_block[n].pos_start,
				       data_block[n].pos_end -
				       data_block[n].pos_start + 1);
				sha512_hash_string((unsigned char *)block,
						   data_block[n].pos_end -
						   data_block[n].pos_start + 1,
						   hash);
				lua_pushstring(L, hash);
				return 1;
			} else {
				ui__ErrorMsg
				    ("You need select valid block before SHA512 hash calculation!");
			}
		} else if (lua_type(L, 1) == LUA_TSTRING) {
			block =
			    (char *)malloc(strlen((char *)lua_tostring(L, 1)));
			block = (char *)lua_tostring(L, 1);
			sha512_hash_string((unsigned char *)block,
					   strlen(block), hash);
			lua_pushstring(L, hash);
			return 1;
		}
	} else {
		ui__ErrorMsg
		    ("Error in lua sha512_hash function! Wrong format!");
	}
	return 0;
}

/* Calculate RIPEMD160 hash of buffer */
/* lua: ripemd160_hash(block_number) */
/* lua: ripemd160_hash("string") */
static int bvi_ripemd160_hash(lua_State * L)
{
	unsigned int n = 0;
	char *block = NULL;
	char hash[65];
	hash[0] = '\0';
	if (lua_gettop(L) == 1) {
		if (lua_type(L, 1) == LUA_TNUMBER) {
			n = (unsigned int)lua_tonumber(L, 1);
			if (data_block[n].pos_end > data_block[n].pos_start) {
				block =
				    (char *)malloc(data_block[n].pos_end -
						   data_block[n].pos_start + 1);
				memcpy(block, mem + data_block[n].pos_start,
				       data_block[n].pos_end -
				       data_block[n].pos_start + 1);
				ripemd160_hash_string((unsigned char *)block,
						      data_block[n].pos_end -
						      data_block[n].pos_start +
						      1, hash);
				lua_pushstring(L, hash);
				return 1;
			} else {
				ui__ErrorMsg
				    ("You need select valid block before RIPEMD160 hash calculation!");
			}
		} else if (lua_type(L, 1) == LUA_TSTRING) {
			block =
			    (char *)malloc(strlen((char *)lua_tostring(L, 1)));
			block = (char *)lua_tostring(L, 1);
			ripemd160_hash_string((unsigned char *)block,
					      strlen(block), hash);
			lua_pushstring(L, hash);
			return 1;
		}
	} else {
		ui__ErrorMsg
		    ("Error in lua ripemd160_hash function! Wrong format!");
	}
	return 0;
}

/* Search byte sequence in the buffer */
/* Return address found */
/* lua: search_bytes(bytes) */
/* or search_bytes(bytes, return_all) where return_all = [0|1] */
static int bvi_search_bytes(lua_State * L)
{
	PTR result;
	PTR start_addr = NULL;
	char *bytes;
	char msgbuf[256];
	int i = 0;

	msgbuf[0] = '\0';
	start_addr = mem;
	if (lua_gettop(L) == 1) {
		bytes = (char *)lua_tostring(L, -1);
		result =
		    searching('\\', bytes, start_addr, maxpos - 1,
			      FALSE | S_GLOBAL);
		/*
		   sprintf(msgbuf, "Found [%s] at 0x%08x position", bytes, (long)(result - mem));
		   wmsg(msgbuf, 3, strlen(msgbuf) + 4);
		 */
		lua_pushnumber(L, (long)(result - mem));
		return 1;
	} else if (lua_gettop(L) == 2) {
		bytes = (char *)lua_tostring(L, 1);
		result = start_addr;
		if ((int)lua_tonumber(L, 2)) {
			while (result < maxpos) {
				result =
				    searching('\\', bytes, start_addr,
					      maxpos - 1, FALSE | S_GLOBAL);
				lua_pushnumber(L, (long)(result - mem));
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
static int bvi_replace_bytes(lua_State * L)
{
	int result = 0;
	PTR start_addr = NULL;
	char *bytes;
	char *target;
	char bytesbuf[256];
	bytesbuf[0] = '\0';
	start_addr = mem;
	if (lua_gettop(L) == 2) {
		bytes = (char *)lua_tostring(L, 1);
		target = (char *)lua_tostring(L, 2);
		sprintf(bytesbuf, "/%s/%s/", bytes, target);
		result =
		    do_substitution('\\', bytesbuf, start_addr, maxpos - 1);
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
		ui__ErrorMsg(message);
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
static int bvi_tools_window(lua_State * L)
{
	int n = lua_gettop(L);
	if (n == 1) {
		ui__ToolWin_Show((int)lua_tonumber(L, -1));
	}
	return 0;
}

/* Display text in tools window */
/* lua: print_tools_window() */
static int bvi_print_tools_window(lua_State * L)
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
		ui__ErrorMsg("Wrong format of lua print() function!");
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

/* Adds marker to show in address */
static int bvi_set_marker(lua_State * L)
{
	long address;
	int i = 0;
	if (lua_gettop(L) > 0) {
		address = (long)lua_tonumber(L, 1);
		while ((markers[i].address != 0) & (i < MARK_COUNT))
			i++;
		markers[i].address = address;
		markers[i].marker = '+';
		ui__Screen_Repaint();
	}
	return 0;
}

/* Remove marker */
static int bvi_unset_marker(lua_State * L)
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
			ui__Screen_Repaint();
		} else
			ui__ErrorMsg("Cant found mark on this address!");
	}
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
		{"block_fold", bvi_block_fold},
		{"block_read", bvi_block_read},
		{"block_and", bvi_block_and},
		{"block_or", bvi_block_or},
		{"block_xor", bvi_block_xor},
		{"block_lshift", bvi_block_lshift},
		{"block_rshift", bvi_block_rshift},
		{"block_lrotate", bvi_block_lrotate},
		{"block_rrotate", bvi_block_rrotate},
		{"crc16", bvi_crc16},
		{"crc32", bvi_crc32},
		{"md4_hash", bvi_md4_hash},
		{"md5_hash", bvi_md5_hash},
		{"sha1_hash", bvi_sha1_hash},
		{"sha256_hash", bvi_sha256_hash},
		{"sha512_hash", bvi_sha512_hash},
		{"ripemd160_hash", bvi_ripemd160_hash},
		{"search_bytes", bvi_search_bytes},
		{"replace_bytes", bvi_replace_bytes},
		{"display_error", bvi_display_error},
		{"display_status_msg", bvi_status_line_msg},
		{"msg_window", bvi_msg_window},
		{"tools_window", bvi_tools_window},
		{"print_tools_window", bvi_print_tools_window},
		{"print", bvi_print},
		{"set", bvi_set},
		{"undo", bvi_undo},
		{"redo", bvi_redo},
		{"insert", bvi_insert},
		{"remove", bvi_remove},
		{"cursor", bvi_cursor},
		{"scrolldown", bvi_scrolldown},
		{"scrollup", bvi_scrollup},
		{"set_marker", bvi_set_marker},
		{"unset_marker", bvi_unset_marker},
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
		ui__ErrorMsg("Error: cant open lua script file!");
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
		ui__ErrorMsg("Error in lua script!");
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
