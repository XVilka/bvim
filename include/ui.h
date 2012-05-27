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

void ui__Init(core_t*, buf_t*);
void ui__ErrorMsg(core_t*, buf_t*, char *);
void ui__SystemErrorMsg(core_t*, buf_t *, char *);
void ui__StatusMsg(core_t*, buf_t*, char *);
void ui__MsgWin_Show(core_t*, buf_t*, char *, int width, int height);

int ui__ToolWin_Show(core_t*, buf_t*, int);
int ui__ToolWin_Hide(core_t*, buf_t*);
int ui__ToolWin_Print(core_t*, buf_t*, char *, int);
short ui__ToolWin_Exist(core_t*, buf_t*);

int ui__REPL_Main(core_t*, buf_t*);
int ui__REPLWin_Show(core_t*, buf_t*);
int ui__REPLWin_Hide(core_t*, buf_t*);
int ui__REPLWin_print(core_t*, buf_t*, char*);
int ui__REPLWin_clear(core_t*, buf_t*);
short ui__REPLWin_Exist();

void ui__MainWin_Resize(core_t*, int);
void printcolorline(int, int, int, char *);
void ui__Line_Print(core_t*, buf_t*, PTR, int);
int ui__lineout(core_t*, buf_t*);
void ui__clearstr(core_t*, buf_t*);
void ui__smsg(core_t*, buf_t*, char *s);

void ui__setcur(void);

int ui__Color_Set(core_t *core, buf_t *buf, char *);
void ui__Colors_Set();
void ui__Colors_Load();
void ui__Colors_Save();

void ui__Screen_Repaint(core_t*, buf_t*);
void ui__Screen_New(core_t*, buf_t*);

int ui__BlockHighlightAdd(struct block_item *blk);

int ui__Destroy();
