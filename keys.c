#include "bvi.h"
#include "keys.h"
#include "bscript.h"

static struct keys_array keymap;

int KeyAdd(struct key item)
{
	if (keymap.items == keymap.allocated) {
		if (keymap.allocated == 0)
			keymap.allocated = 3;
		else
			keymap.allocated += 4;

		void *_tmp =
		    realloc(keymap.arr,
			    (keymap.allocated * sizeof(struct key)));

		if (!_tmp) {
			fprintf(stderr, "ERROR: Couldn't realloc memory!\n");
			return (-1);
		}
		keymap.arr = (struct key *)_tmp;
	}
	keymap.arr[keymap.items] = item;
	keymap.items++;

	return keymap.items;
}

int KeyDel(struct key item)
{
	return 0;
}

int KeyDefaults()
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
		 {.func = handler__doft3}},
		{'f', "f", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__doft3}},
		{'t', "t", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__doft3}},
		{'T', "T", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__doft3}},
		{'G', "G", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto1}},
		{'g', "g", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__goto2}},
		{'?', "?", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__search_string}},
		{'/', "/", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__search_string}},
		{'#', "#", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__search_string}},
		{'\\', "\\", "", 1, BVI_HANDLER_INTERNAL,
		 {.func = handler__search_string}},
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
		{0, NULL, NULL, 0, 0, {NULL}} // end marker
	};
	while (keys_def[i].id != 0) {
		KeyAdd(keys_def[i]);
		i++;
	}
	return 0;
}

void keys__Init()
{
	keymap.arr = NULL;
	keymap.items = 0;
	keymap.allocated = 0;
	KeyDefaults();
}

void keys__Destroy()
{
	free(keymap.arr);
}

// TODO: Implement parsing key strings, like <Ctrl-K>
struct key *keys__KeyString_Parse(char *key_string)
{
	struct key *k;
	k = (struct key *)malloc(sizeof(struct key *));
	return k;
}

int keys__Key_Map(struct key *map_key)
{
	KeyAdd(*map_key);
	return 0;
}

int keys__Key_Unmap(struct key *map_key)
{
	KeyDel(*map_key);
	return 0;
}

void keys__KeyMaps_Show(void)
{
	char dispbuf[256];
	char luacmdbuf[256];
	int i = 0;

	dispbuf[0] = '\0';
	while (i < keymap.items) {
		luacmdbuf[0] = '\0';
		sprintf(luacmdbuf, "map %s %s\n", keymap.arr[i].name,
			keymap.arr[i].description);
		strcat(dispbuf, luacmdbuf);
		i++;
	}
	msg(dispbuf);
	wait_return(TRUE);
}

int keys__Key_Pressed(int key_code)
{
	int j = 0;
	while (j < keymap.items) {
		if ((keymap.arr[j].id == key_code) & (keymap.arr[j].enabled ==
						      1)) {
			if ((keymap.arr[j].handler_type ==
			     BVI_HANDLER_INTERNAL) & (keymap.arr[j].handler.
						      func != NULL)) {
				(*(keymap.arr[j].handler.func)) ();
			} else
			    if ((keymap.arr[j].handler_type ==
				 BVI_HANDLER_LUA) & (keymap.arr[j].handler.
						     lua_cmd != NULL)) {
				bvi_run_lua_string(keymap.arr[j].handler.
						   lua_cmd);
			}
		}
		j++;
	}
	return 0;
}
