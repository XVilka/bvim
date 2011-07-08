/* DOSCUR.H - CURSES.H for TURBO C
 *
 * Copyright 1996-2002 by Gerhard Buergmann
 * Gerhard.Buergmann@puon.at
 *
 * 1996-02-28 V 1.0.0
 * 1998-04-12 V 1.0.1
 * 1999-01-14 V 1.1.1
 * 1999-07-01 V 1.2.0
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * See file COPYING for information on distribution conditions.
 */

#include <stdio.h>
#include <conio.h>
#include <dos.h>

#define TRUE            1
#define FALSE           0

#define ESC           27
#define KEY_F0        0x3A00
#define KEY_F(n)      (KEY_F0+(n<<8))
#define KEY_DOWN      0x5000
#define KEY_UP        0x4800
#define KEY_LEFT      0x4B00
#define KEY_RIGHT     0x4D00
#define KEY_NPAGE     0x5100
#define KEY_PPAGE     0x4900
#define KEY_RETURN    0x0d
#define KEY_ENTER     0x0a
#define KEY_BACKSPACE 0x0E08
#define KEY_HOME      0x4700
#define KEY_LL        0x4f00                    /* HOME DOWN, End */
#define KEY_DC        0x5300
#define KEY_IC        0x5200



#define A_NORMAL        3
#define A_STANDOUT      8
#define A_BOLD          8
#define A_REVERSE       127
#define A_BLINK         128
#define A_CHARTEXT      0x00ff
#define A_ATTRIBUTES    0xff00

#define chtype  unsigned int

#define WINDOW  text_info

extern  struct WINDOW scr;

extern  int     stdscr;

extern  int     COLS;
extern  int     LINES;
extern  int     ECHO;
extern  int     NODEL;

void	attrset(int);

#define initscr()               clrscr();window(1,1,COLS,LINES);gettextinfo(&scr)
#define newwin(h,b,y,x) window(x+1,y+1,x+b+1,y+h+1);gettextinfo(&scr)
#define erasechar()             KEY_ERASE
#define beep()                  putch(7)
#define flash()                 putch(7)
#define wclrtoeol(w)    clreol()
#define clrtoeol()              clreol()
#define werase(w)               window(scr.winleft,scr.wintop,scr.winright,scr.winbottom)
#define erase()                 clrscr()
#define wclear(w)               werase(w)
#define clear()                 clrscr()
/*
#define wattrset(w,a)   textattr(a)
#define attrset(a)              textattr(a)
*/
#define standout()              highvideo()
#define standend()              normvideo()
#define wmove(w,y,x)    gotoxy(x+1,y+1)
#define move(y,x)               gotoxy(x+1,y+1)
#define mvwaddch(w,y,x,c)       gotoxy(x+1,y+1);putch(c)
#define mvaddch(y,x,c)  gotoxy(x+1,y+1);putch(c)
#define waddch(w,c)             putch(c)
#define addch(c)                putch(c)
#define mvwaddstr(w,y,x,s)      gotoxy(x+1,y+1);cputs(s)
#define mvaddstr(y,x,s)         gotoxy(x+1,y+1);cputs(s)
#define waddstr(s)              cputs(s)
#define addstr(s)               cputs(s)
#define mvwprintw(w,y,x,s)      gotoxy(x+1,y+1);cprintf(s)
#define mvprintw(y,x,s,a)       gotoxy(x+1,y+1);cprintf(s,a)
#define wprintw(w,s)    cprintf(s)
#define printw                  cprintf
#define getyx(w,y,x)    x=wherex()-1;y=wherey()-1
#define getbegyx(w,y,x) x=scr.winleft;y=scr.wintop;
#define getmaxyx(w,y,x) x=(scr.winright)-(scr.winleft);y=(scr.winbottom)-(scr.wintop)+1
#define winsertln()             insline()
#define insertln()              insline()
#define wdeleteln(w)    delline()
#define deleteln()              delline()
#define mvwgetch(w,y,x) gotoxy(x+1,y+1);ugetch()
#define mvgetch(y,x)    gotoxy(x+1,y+1);ugetch()
#define wgetch(w)               ugetch()
#define mvwgetstr(w,y,x,s)      gotoxy(x+1,y+1);gets(s)
#define mvgetstr(y,x,s)         gotoxy(x+1,y+1);gets(s)
#define wgetstr(w,s)            gets(s)
/*
#define getstr(s)               gets(s)
*/
#define flushinp()              fflush(stdin)
#define mvwscanw(w,y,x,s)       gotoxy(x+1,y+1);cscanf(s)
#define mvscanw(y,x,s)  gotoxy(x+1,y+1);cscanf(s)
#define wscanw(w,s)             wscanw(s)
#define scanw                   cscanf
#define mvinsch(y,x,c)  gotoxy(x+1,y+1);insch(c)

#define nodelay(w,b)    NODEL=b
#define echo()                  ECHO=TRUE
#define noecho()                ECHO=FALSE

#define nl()
#define nonl()
#define keypad(a,b)
#define refresh()
#define wrefresh(w)
#define cbreak()
#define endwin()
#define raw()
#define noraw()
#define notimeout(w,t)
#define idlok(w,b)
#define scrollok(a,b)
#define doupdate()
#define savetty()
#define resetty()

#define ERR     (-1)

