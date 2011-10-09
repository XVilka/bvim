#include <stdio.h>
#include "bvi.h"
#include "set.h"
#include "ui.h"

extern core_t core;
extern state_t state;
extern struct BLOCK_ data_block[BLK_COUNT];
extern struct MARKERS_ markers[MARK_COUNT];

char tmpbuf[10];
char linbuf[256];

struct color colors[] = {	/* RGB definitions and default value, if have no support of 256 colors */
	{"background", "bg", 50, 50, 50, COLOR_BLACK},
	{"addresses", "addr", 335, 506, 700, COLOR_BLUE},
	{"hex", "hex", 600, 600, 600, COLOR_MAGENTA},
	{"data", "data", 0, 800, 400, COLOR_GREEN},
	{"error", "err", 999, 350, 0, COLOR_RED},
	{"status", "stat", 255, 255, 255, COLOR_WHITE},
	{"command", "comm", 255, 255, 255, COLOR_WHITE},
	{"window", "win", 0, 800, 900, COLOR_YELLOW},
	{"addrbg", "addrbg", 0, 0, 0, COLOR_CYAN},
	{"", "", 0, 0, 0, 0}	/* end marker */
};

struct {
	short r;
	short g;
	short b;
} original_colors[8];

struct {
	short f;
	short b;
} original_colorpairs[8];

void ui__Init() {
	/****** Initialisation of curses ******/
	initscr();
	if (has_colors() != FALSE) {
		start_color();
		ui__Colors_Save();
		ui__Colors_Set();
	}
	attrset(A_NORMAL);
	ui__MainWin_Resize(LINES);	
}

void ui__MainWin_Resize(int lines_count) {
	core.screen.maxy = lines_count;
	if (params[P_LI].flags & P_CHANGED)
		core.screen.maxy = P(P_LI);
	state.scrolly = core.screen.maxy / 2;
	P(P_SS) = state.scrolly;
	P(P_LI) = core.screen.maxy;
	core.screen.maxy--;
	
	keypad(stdscr, TRUE);
	scrollok(stdscr, TRUE);
	nonl();
	cbreak();
	noecho();

	core.params.COLUMNS_ADDRESS = 10;
	strcpy(addr_form, "%08lX%c:");

	core.params.COLUMNS_DATA = ((COLS - core.params.COLUMNS_ADDRESS - 1) / 16) * 4;
	P(P_CM) = core.params.COLUMNS_DATA;
	core.screen.maxx = core.params.COLUMNS_DATA * 4 + core.params.COLUMNS_ADDRESS + 1;
	core.params.COLUMNS_HEX = core.params.COLUMNS_DATA * 3;
	status = core.params.COLUMNS_HEX + core.params.COLUMNS_DATA - 17;
	state.screen = core.params.COLUMNS_DATA * (core.screen.maxy - 1);

	ui__Screen_New();
}

/* ==================== Tools window ======================= */
WINDOW *tools_win = NULL;

/* Check if tools window already exist */
short ui__ToolWin_Exist() {
	if (tools_win == NULL)
		return 0;
	else
		return 1;
}

/* Show tools window with <lines_count> height */
int ui__ToolWin_Show(int lines_count) {
	if (tools_win == NULL) {
		lines_count = LINES - lines_count - 2;
		ui__MainWin_Resize(lines_count);
		refresh();
		attron(COLOR_PAIR(C_WN + 1));
		tools_win = newwin(LINES - lines_count + 2, core.screen.maxx + 1, lines_count, 0);
		box(tools_win, 0, 0);
		wrefresh(tools_win);
		attroff(COLOR_PAIR(C_WN + 1));
		return 0;
	} else {
		ui__ToolWin_Hide();
		ui__ToolWin_Show(lines_count);
		return 0;
	}
}

/* Print string in tools window in <line> line */
int ui__ToolWin_Print(char* str, int line) {
	if (tools_win != NULL) {
		attron(COLOR_PAIR(C_WN + 1));
		mvwaddstr(tools_win, line, 1, str);
		wrefresh(tools_win);
		attroff(COLOR_PAIR(C_WN + 1));
		return 0;
	} else {
		ui__ErrorMsg("print_tools_window: tools window not exist!\n");
		return -1;
	}
}

int ui__ToolWin_ScrollUp(int lines) {
	return 0;
}

int ui__ToolWin_ScrollDown(int lines) {
	return 0;
}

/* Hides tools window */
int ui__ToolWin_Hide() {
	if (tools_win != NULL) {
		ui__MainWin_Resize(LINES);
		attron(COLOR_PAIR(C_WN + 1));
		wborder(tools_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
		wrefresh(tools_win);
		delwin(tools_win);
		attroff(COLOR_PAIR(C_WN + 1));
		ui__Screen_Repaint();
		tools_win = NULL;
		return 0;
	} else {
		ui__ErrorMsg("hide_tools_window: tools window not exist!\n");
		return -1;
	}
}

/* ========================== Colors handling ============================ */
void ui__Colors_Save()
{
	int i;
	for (i = 0; colors[i].fullname[0] != '\0'; i++) {
		color_content(colors[i].short_value, &original_colors[i].r, &original_colors[i].g, &original_colors[i].b);
	}
	for (i = 1; i < 8; i++) {
		pair_content(i, &original_colorpairs[i].f, &original_colorpairs[i].b);
	}
}

void ui__Colors_Load()
{
	int i;
	for (i = 0; colors[i].fullname[0] != '\0'; i++) {
		init_color(colors[i].short_value, original_colors[i].r, original_colors[i].g, original_colors[i].b);
	}
	for (i = 1; i < 8; i++) {
		init_pair(i, original_colorpairs[i].f, original_colorpairs[i].b);
	}
}

void ui__Colors_Set()
{
	int i;
	if (can_change_color()) {
		for (i = 0; colors[i].fullname[0] != '\0'; i++) {
			if (init_color
			    (colors[i].short_value, C_r(i), C_g(i),
			     C_b(i)) == ERR)
				fprintf(stderr, "Failed to set [%d] color!\n",
					i);
			if (C_s(i) <= 7) {
				init_pair(i + 1, C_s(i), C_s(0));
			} else {
				colors[i].short_value = COLOR_WHITE;
				init_pair(i + 1, C_s(i), C_s(0));
			}
		}
		init_pair(C_AD + 1, C_s(C_AD), COLOR_CYAN);
	} else {		/* if have no support of changing colors */
		for (i = 0; colors[i].fullname[0] != '\0'; i++) {
			if (C_s(i) <= 7) {
				init_pair(i + 1, C_s(i), C_s(0));
			} else {
				colors[i].short_value = COLOR_WHITE;
				init_pair(i + 1, C_s(i), C_s(0));
			}
		}
	}
}

int ui__Color_Set(char* arg)
{
	int i;
	char *s;
	for (i = 0; colors[i].fullname[0] != '\0'; i++) {
			s = colors[i].fullname;
			if (strncmp(arg, s, strlen(s)) == 0)
				break;
			s = colors[i].shortname;
			if (strncmp(arg, s, strlen(s)) == 0)
				break;
	}
	if (i == 0) {
		ui__ErrorMsg("Wrong color name!");
		return -1;
	} else {
		colors[i].r = atoi(substr(arg, strlen(s) + 1, 3));
		colors[i].g = atoi(substr(arg, strlen(s) + 5, 3));
		colors[i].b = atoi(substr(arg, strlen(s) + 9, 3));
		/*set_palette();*/
		ui__Colors_Set();
		ui__Screen_Repaint();
	}
	return 0;
}

/* ========================== lines print interface ====================== */

void printcolorline(int y, int x, int palette, char *string)
{
	palette++;
	attron(COLOR_PAIR(palette));
	mvaddstr(y, x, string);
	attroff(COLOR_PAIR(palette));
}

void printcolorline_hexhl(int y, int x, int base_palette, char *string, highlight_table *hl, unsigned int hl_tbl_size)
{
	unsigned int i;
	printcolorline(y, x, base_palette, string);
	for (i = 0; i < hl_tbl_size; i++) {
		if ((hl[i].hex_start < hl[i].hex_end) & (hl[i].toggle == 1)) {
			attron(COLOR_PAIR(hl[i].palette) | A_STANDOUT | A_BOLD);
			mvaddstr(y, x + hl[i].hex_start, substr(string, hl[i].hex_start, hl[i].hex_end - hl[i].hex_start));
			attroff(COLOR_PAIR(hl[i].palette) | A_STANDOUT | A_BOLD);
		}
	}
}

void printcolorline_dathl(int y, int x, int base_palette, char *string, highlight_table *hl, unsigned int hl_tbl_size)
{
	unsigned int i;
	printcolorline(y, x, base_palette, string);
	for (i = 0; i < hl_tbl_size; i++) {
		if ((hl[i].dat_start < hl[i].dat_end) & (hl[i].toggle == 1)) {
			attron(COLOR_PAIR(hl[i].palette) | A_STANDOUT | A_BOLD);
			mvaddstr(y, x + hl[i].dat_start, substr(string, hl[i].dat_start, hl[i].dat_end - hl[i].dat_start));
			attroff(COLOR_PAIR(hl[i].palette) | A_STANDOUT | A_BOLD);
		}
	}
}

void ui__Line_Print(mempos, scpos)
PTR mempos;
int scpos;
{
	char hl_msg[256];
	highlight_table hl[BLK_COUNT];
	unsigned int i = 0;
	unsigned int n = 0;
	unsigned int k = 0;
	unsigned int print_pos;
	int nxtpos = 0;
	unsigned char Zeichen;
	long address;
	char marker = ' ';

	hl_msg[0] = '\0';
	
	if (mempos > maxpos) {
		strcpy(linbuf, "~         ");
	} else {
		address = (long)(mempos - mem + P(P_OF));
		while (markers[k].address != 0) {
			if (markers[k].address == address) {
				marker = markers[k].marker;
				break;
			}
			k++;
		}
		sprintf(linbuf, addr_form, address, marker);
		*string = '\0';
	}
	strcat(linbuf, " ");
	/* load color from C(C_AD) */
	printcolorline(scpos, 0, C_AD, linbuf);
	nxtpos = 11;
	*linbuf = '\0';

	/* handle highlighted blocks */
	for (i = 0; i < BLK_COUNT; i++) {
		if (data_block[i].hl_toggle == 1) {
			hl[n].toggle = 1;
			hl[n].hex_start = 0;
			hl[n].dat_start = 0;
			hl[n].palette = data_block[i].palette;
			if (hl[n].flg == 1) {
				hl[n].hex_end = core.params.COLUMNS_DATA * 3;
				hl[n].dat_end = core.params.COLUMNS_DATA;
			} else {
				hl[n].hex_end = 0;
				hl[n].dat_end = 0;
			}
			for (print_pos = 0; print_pos < core.params.COLUMNS_DATA * 3; print_pos += 3) {
				if (((long)(mempos - mem + (print_pos / 3)) == data_block[i].pos_start) & (hl[n].flg != 1)) {
					hl[n].hex_start = print_pos;
					hl[n].dat_start = print_pos / 3;
					hl[n].hex_end = core.params.COLUMNS_DATA * 3;
					hl[n].dat_end = core.params.COLUMNS_DATA;
					hl[n].flg = 1;
				} else if (((long)(mempos - mem + (print_pos / 3)) < data_block[i].pos_end) & 
					((long)(mempos - mem + (print_pos / 3)) > data_block[i].pos_start) & (hl[n].flg != 1)) {
					hl[n].hex_start = print_pos;
					hl[n].dat_start = print_pos / 3;
					hl[n].hex_end = core.params.COLUMNS_DATA * 3;
					hl[n].dat_end = core.params.COLUMNS_DATA;
					hl[n].flg = 1;
				} else if (((long)(mempos - mem + (print_pos / 3)) == data_block[i].pos_end) & (hl[n].flg == 1)) {
					hl[n].hex_end = print_pos + 2;
					hl[n].dat_end = print_pos / 3 + 1;
					hl[n].flg = 0;
				} else if (((long)(mempos - mem + (print_pos / 3)) > data_block[i].pos_end)) {
					hl[n].flg = 0;
				}
			}
			n++;
		}
	}
	
	for (print_pos = 0; print_pos < core.params.COLUMNS_DATA; print_pos++) {
		if (mempos + print_pos >= maxpos) {
			sprintf(tmpbuf, "   ");
			Zeichen = ' ';
		} else {
			Zeichen = *(mempos + print_pos);
			sprintf(tmpbuf, "%02X ", Zeichen);
		}
		strcat(linbuf, tmpbuf);
		if (isprint(Zeichen))
			*(string + print_pos) = Zeichen;
		else
			*(string + print_pos) = '.';
	}
	*(string + core.params.COLUMNS_DATA) = '\0';

	/* load color from C(C_HX) */
	strcat(linbuf, "|");
	printcolorline_hexhl(scpos, nxtpos, C_HX, linbuf, hl, n + 1);
	
	/* strcat(linbuf, string); */
	nxtpos += strlen(linbuf);
	/* load color from C(C_DT) */
	printcolorline_dathl(scpos, nxtpos, C_DT, string, hl, n + 1);
}


/* TODO: add hex-data folding feature */

/* displays a line on screen
 * at state.pagepos + line y
 */
int ui__lineout()
{
	off_t Address;
	
	Address = state.pagepos - mem + y * core.params.COLUMNS_DATA;
	ui__Line_Print(mem + Address, y);
	move(y, x);
	/*if (k != 0) 
		y = k;
	*/
	return (0);
}

void ui__Screen_New()
{
	state.screen = core.params.COLUMNS_DATA * (core.screen.maxy - 1);
	clear();
	ui__Screen_Repaint();
}

int ui__line(int line, int address)
{
	ui__Line_Print(mem + address, line);
	move(line, x);
	return (0);
}

void ui__Screen_Repaint()
{
/***** redraw screen *********************/
	off_t Address;
	int save_y;
	int fold_start = 0;
	int fold_end = 0;
	int i = 0;

	save_y = y;
	for (y = 0; y < core.screen.maxy; y++) {
		Address = state.pagepos - mem + y * core.params.COLUMNS_DATA;
		for (i = 0; i < BLK_COUNT; i++) {
			if ((data_block[i].folding == 1) & (data_block[i].pos_start < Address) & (data_block[i].pos_end > Address)) {
				fold_start = data_block[i].pos_start;
				fold_end = data_block[i].pos_end;
				state.pagepos += (fold_end - fold_start) / core.params.COLUMNS_DATA;
				/* Address = state.pagepos - mem + y * core.params.COLUMNS_DATA; */
				break;
			}
		}
		ui__line(y, Address);
	}
	y = save_y;
}

void clearstr()
{
	int n;

	move(core.screen.maxy, 0);
	for (n = 0; n < core.screen.maxx; n++)
		addch(' ');
	move(core.screen.maxy, 0);
}

/**** displays an error message *****/
void ui__ErrorMsg(s)
char *s;
{
	int cnt;

	if (P(P_EB))
		beep();
	clearstr();
	attron(COLOR_PAIR(C_ER + 1));
	/* attrset(A_REVERSE); */
	cnt = outmsg(s);
	/* attrset(A_NORMAL); */
	attroff(COLOR_PAIR(C_ER + 1));
	if (cnt >= (core.screen.maxx - 25)) {	/* 25 = status */
		addch('\n');
		wait_return(TRUE);
	}
}

/*** System error message *****/
void sysemsg(s)
char *s;
{
	char string[256];

#ifdef HAVE_STRERROR
	sprintf(string, "%s: %s", s, strerror(errno));
#else
	sprintf(string, "%s: %s", s, sys_errlist[errno]);
#endif

	ui__ErrorMsg(string);
}

/*** displays mode if showmode set *****/
void smsg(s)
char *s;
{
	if (P(P_MO)) {
		msg(s);
		setcur();
	}
}

/*** display window ***/
void ui__MsgWin_Show(s, height, width)
char *s;
int height;
int width;
{
	WINDOW *msg_win;
	int starty = (LINES - height) / 2;	/* Calculating for a center placement */
	int startx = (COLS - width) / 2;	/* of the window                */
	int ch;
	refresh();
	attron(COLOR_PAIR(C_WN + 1));
	msg_win = newwin(height, width, starty, startx);
	box(msg_win, 0, 0);
	wrefresh(msg_win);
	mvwaddstr(msg_win, 1, 2, s);
	wrefresh(msg_win);
	while ((ch = getch()) == -1) {
	}
	wborder(msg_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(msg_win);
	delwin(msg_win);
	attroff(COLOR_PAIR(C_WN + 1));
	ui__Screen_Repaint();
}

/************* displays s on status line *****************/
void msg(s)
char *s;
{
	clearstr();
	if (outmsg(s) >= (core.screen.maxx - 25)) {	/* 25 = status */
		addch('\n');
		wait_return(TRUE);
	}
}

int outmsg(s)
char *s;
{
	char *poi;
	int cnt = 0;

	move(core.screen.maxy, 0);
	poi = strchr(s, '|');

	if (P(P_TE)) {
		poi = s;
		while (*poi != '\0' && *poi != '@' && *poi != '|') {
			addch(*poi++);
			cnt++;
		}
	} else {
		if (poi)
			poi++;
		else
			poi = s;
		while (*poi) {
			if (*poi == '@')
				addch(' ');
			else
				addch(*poi);
			poi++;
			cnt++;
		}
	}
	return cnt;
}


