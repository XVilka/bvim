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

typedef struct hl_t *hl_link;

struct hl_item {
	unsigned int id; /* unique id */
	char name[64]; /* for naming highlights */
	unsigned long hex_start;
	unsigned long hex_end;
	unsigned long dat_start;
	unsigned long dat_end;
	int flg;
	unsigned int palette;   /* pallete, which we are using for highlight */
	int toggle; /* is this highlighting enabled? */
};

struct hl_t { 
	struct hl_item item;
	hl_link next;
};


struct repl_t {
	int current_y;
	int current_x;
};

void ui__Init(void);
void ui__ErrorMsg(char *);
void ui__SystemErrorMsg(char *);
void ui__StatusMsg(char *);
void ui__MsgWin_Show(char *, int width, int height);

int ui__ToolWin_Show(int), ui__ToolWin_Hide();
int ui__ToolWin_Print(char *, int);
short ui__ToolWin_Exist();

int ui__REPL_Main();
int ui__REPLWin_Show();
int ui__REPLWin_Hide();
int ui__REPLWin_print(const char*);
int ui__REPLWin_clear();
short ui__REPLWin_Exist();

void ui__MainWin_Resize(int);
void printcolorline(int, int, int, char *);
void ui__Line_Print();
int ui__lineout();

void ui__setcur(void);

int ui__Color_Set(char *);
void ui__Colors_Set();
void ui__Colors_Load();
void ui__Colors_Save();

void ui__Screen_Repaint(void), ui__Screen_New(void);

int ui__BlockHighlightAdd(struct block_item *blk);

int ui__Destroy();
