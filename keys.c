#include "bvi.h"
#include "keys.h"

struct key keys[] = {
	{ '^', "Caret", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_HEX }},
	{ '$', "Dollar", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_ASCII }},
	{ '\t', "Tab", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__toggle }},
	{ '~', "Tilda", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__tilda }},
	{ KEY_HOME, "Home", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_home }},
	{ 'H', "H", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_home }},
	{ 'M', "M", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__M }},
	{ KEY_LL, "LL", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__L }},
	{ 'L',"L", "",  1, BVI_HANDLER_INTERNAL, { .func = handler__L }},
	{ BVI_CTRL('H'), "Ctrl-H", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_left }},
	{ KEY_BACKSPACE, "Backspace", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_left }},
	{ KEY_LEFT, "Left", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_left }},
	{ 'h', "h", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_left }},
	{ ' ', "Space", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_right }},
	{ KEY_RIGHT, "Right", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_right }},
	{ 'l', "l", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_right }},
	{ '-', "Minus", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_up }},
	{ KEY_UP, "Up", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_up }},
	{ 'k', "k", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_up }},
	{ '+', "Plus", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_EOL }},
	{ CR, "Enter", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_EOL }},
	{ 'j', "j", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_down }},
	{ BVI_CTRL('J'), "Ctrl-J", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_down }},
	{ BVI_CTRL('N'), "Ctrl-N", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_down }},
	{ KEY_DOWN, "Down", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_down }},
	{ '|', "|", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__line }},
	{ 'S', "S", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__toolwin_toggle }},
	{ ':', ":", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__cmdstring }},
	{ BVI_CTRL('B'), "Ctrl-B", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__previous_page }},
	{ KEY_PPAGE, "PageUp", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__previous_page }},
	{ BVI_CTRL('D'), "Ctrl-D", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__scrolldown }},
	{ BVI_CTRL('U'), "Ctrl-U", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__scrollup }},
	{ BVI_CTRL('E'), "Ctrl-E", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__linescroll_down }},
	{ BVI_CTRL('F'), "Ctrl-F", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__nextpage }},
	{ KEY_NPAGE, "PageDown", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__nextpage }},
	{ BVI_CTRL('G'), "Ctrl-G", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__fileinfo }},
	{ BVI_CTRL('L'), "Ctrl-L", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__screen_redraw }},
	{ BVI_CTRL('Y'), "Ctrl-Y", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__linescroll_up }},
	{ 'A', "A", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__append_mode }},
	{ 'B', "B", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__backsearch }},
	{ 'b', "b", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__backsearch }},
	{ 'e', "e", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__setpage }},
	{ ',', ",", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__doft1 }},
	{ ';', ";", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__doft2 }},
	{ 'F', "F", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__doft3 }},
	{ 'f', "f", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__doft3 }},
	{ 't', "t", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__doft3 }},
	{ 'T', "T", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__doft3 }},
	{ 'G', "G", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto1 }},
	{ 'g', "g", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto2 }},
	{ '?', "?", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__search_string }},
	{ '/', "/", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__search_string }},
	{ '#', "#", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__search_string }},
	{ '\\', "\\", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__search_string }},
	{ 'n', "n", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__search_next }},
	{ 'N', "N", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__search_next }},
	{ 'm', "m", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__mark }},
	{ '\'', "'", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_mark }},
	{ '`', "`", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__goto_mark }},
	{ 'D', "D", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__trunc }},
	{ 'o', "o", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__overwrite }},
	{ 'P', "P", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__paste }},
	{ 'r', "r", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__redo }},
	{ 'R', "R", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__redo }},
	{ 'u', "u", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__undo }},
	{ 'U', "U", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__undo }},
	{ 'W', "W", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__wordsearch }},
	{ 'w', "w", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__wordsearch }},
	{ 'y', "y", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__yank }},
	{ 'z', "z", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__doz }},
	{ 'Z', "Z", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__exit }},
	{ '.', ".", "", 1, BVI_HANDLER_INTERNAL, { .func = handler__stuffin }},
	{ 0, NULL, NULL, 0, 0, { NULL }}
};

/*
struct keymap keys[] = {
	{ "F1",			BVI_KEY_F1				},
	{ "F2",			BVI_KEY_F2				},
	{ "F3",			BVI_KEY_F3				},
	{ "F4",			BVI_KEY_F4				},
	{ "F5",			BVI_KEY_F5				},
	{ "F6",			BVI_KEY_F6				},
	{ "F7",			BVI_KEY_F7				},
	{ "F8",			BVI_KEY_F8				},
	{ "F9",			BVI_KEY_F9				},
	{ "F10",		BVI_KEY_F10				},
	{ "F11",		BVI_KEY_F11				},
	{ "F12",		BVI_KEY_F12				},
	{ "Space",		BVI_KEY_SPACE			},
	{ "Tab",		BVI_KEY_TAB				},
	{ "Home",		BVI_KEY_HOME			},
	{ "End",		BVI_KEY_END				},
	{ "PgUp",		BVI_KEY_PGUP			},
	{ "PgDown",		BVI_KEY_PGDOWN			},
	{ "Up",			BVI_KEY_UP				},
	{ "Down",		BVI_KEY_DOWN			},
	{ "A",			BVI_KEY_A_BIG			},
	{ "B",			BVI_KEY_B_BIG			},
	{ "C",			BVI_KEY_C_BIG			},
	{ "D",			BVI_KEY_D_BIG			},
	{ "E",			BVI_KEY_E_BIG			},
	{ "F",			BVI_KEY_F_BIG			},
	{ "G",			BVI_KEY_G_BIG			},
	{ "H",			BVI_KEY_H_BIG			},
	{ "I",			BVI_KEY_I_BIG			},
	{ "J",			BVI_KEY_J_BIG			},
	{ "K",			BVI_KEY_K_BIG			},
	{ "L",			BVI_KEY_L_BIG			},
	{ "M",			BVI_KEY_M_BIG			},
	{ "N",			BVI_KEY_N_BIG			},
	{ "O",			BVI_KEY_O_BIG			},
	{ "P",			BVI_KEY_P_BIG			},
	{ "Q",			BVI_KEY_Q_BIG			},
	{ "R",			BVI_KEY_R_BIG			},
	{ "S",			BVI_KEY_S_BIG			},
	{ "T",			BVI_KEY_T_BIG			},
	{ "U",			BVI_KEY_U_BIG			},
	{ "V",			BVI_KEY_V_BIG			},
	{ "W",			BVI_KEY_W_BIG			},
	{ "X",			BVI_KEY_X_BIG			},
	{ "Y",			BVI_KEY_Y_BIG			},
	{ "Z",			BVI_KEY_Z_BIG			},
	{ "a",			BVI_KEY_A_LITTLE		},
	{ "z",			BVI_KEY_Z_LITTLE		},
	{ "0",			BVI_KEY_0				},
	{ "1",			BVI_KEY_1				},	
	{ "2",			BVI_KEY_2				},
	{ "3",			BVI_KEY_3				},
	{ "4",			BVI_KEY_4				},
	{ "5",			BVI_KEY_5				},
	{ "6",			BVI_KEY_6				},
	{ "7",			BVI_KEY_7				},
	{ "8",			BVI_KEY_8				},
	{ "9",			BVI_KEY_9				},
	{ ",",			BVI_KEY_COMMA			},
	{ "Backspace",	BVI_KEY_BACKSPACE		},
	{ "Del",		BVI_KEY_DELETE			},
	{ "Ins",		BVI_KEY_INSERT			},
	{ "^",			BVI_KEY_CARET			},
	{ "~",			BVI_KEY_TILDA			},
	{ "$",			BVI_KEY_DOLLAR			},
	{ "%",			BVI_KEY_PERCENT			},
	{ "@",			BVI_KEY_AT				},
	{ "#",			BVI_KEY_DIEZ			},
	{ "&",			BVI_KEY_AMPERSAND		},
	{ "*",			BVI_KEY_ASTERISK		},
	{ "-",			BVI_KEY_MINUS			},
	{ "+",			BVI_KEY_PLUS			},
	{ "", 0 }
};
*/

struct KEYMAP_ KEYMAP[32];

void keys__Init() {
	/*
	int i;
	for (i = 0; i < 32; i++)
		KEYMAP[i].keycode = 0;
	*/
}

struct key* keys__KeyString_Parse(char *key_string) {
	struct key *k;
	k = (struct key*)malloc(sizeof(struct key*));
	/*
	for (n = 0; n < 32; n++) {
		if (KEYMAP[n].keycode == atoi(key_string)) {
			map_toggle = 1;
			ui__ErrorMsg("Already mapped key. Try tomorrow, please!");
			break;
		}
	}
	if (map_toggle == 0) {
		for (n = 0; n < 32; n++) {
			if (KEYMAP[n].keycode == 0) {
				KEYMAP[n].keycode = atoi(c_argv[0]);
				strcpy(KEYMAP[n].cmd, luacmdbuf);
				map_toggle = 1;
				break;
			}
		}
		if (map_toggle == 0)
			ui__ErrorMsg("There is no empty slots for mapping!");
	}
	*/
	return k;
}

int keys__Key_Map(struct key *map_key, char* cmd) {
	return 0;
}

void keys__KeyMaps_Show(void) {
	char dispbuf[256];
	char luacmdbuf[256];
	int n;

	dispbuf[0] = '\0';
	for (n = 0; n < 32; n++) {
		if (KEYMAP[n].keycode != 0) {
			luacmdbuf[0] = '\0';
			sprintf(luacmdbuf, "map %d %s\n", KEYMAP[n].keycode, KEYMAP[n].cmd);
			strcat(dispbuf, luacmdbuf);
		}
	}
	msg(dispbuf);
	wait_return(TRUE);
}

int keys__Key_Pressed(int ch, struct key* k) {
	int i = 0;
	while (keys[i].id != 0) {
		if ((ch == keys[i].id) & (keys[i].enabled == 1)) {
			if ((keys[i].handler_type == BVI_HANDLER_INTERNAL) & (keys[i].handler.func != NULL)) {
				(*(keys[i].handler.func))();
			}
		}
		i++;
	}
	return 0;
}
