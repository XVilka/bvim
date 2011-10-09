/* COMM.C - Routines to parse and execute "command line" commands, such as
 * search or colon commands.
 *
 * 1996-01-16 created;
 * 1998-03-14 V 1.0.1
 * 1999-02-03 V 1.1.0
 * 1999-06-22 V 1.2.0 beta
 * 1999-09-10 V 1.2.0 final
 * 2000-03-03 V 1.3.0 beta
 * 2000-07-15 V 1.3.0 final
 * 2001-10-10 V 1.3.1  
 * 2003-07-04 V 1.3.2
 *
 * NOTE: Edit this file with tabstop=4 !
 *
 * Copyright 1996-2003 by Gerhard Buergmann
 * gerhard@puon.at
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

#include "bvi.h"
#include "keys.h"
#include "set.h"
#include "ui.h"

#ifdef HAVE_LUA_H
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "bscript.h"
#endif

#ifdef HAVE_UNISTD_H
#	include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
#	include <fcntl.h>
#endif

#if defined(__MSDOS__) && !defined(DJGPP)
#include <io.h>
#	include <dir.h>
#define WRITE (O_WRONLY|O_CREAT|O_TRUNC|O_BINARY)
#define APPEND (O_WRONLY|O_APPEND|O_BINARY)
#else
#define WRITE (O_WRONLY|O_CREAT|O_TRUNC)
#define APPEND (O_WRONLY|O_APPEND)
#endif

#include "plugins.h"

#define	CMDSZ	256		/* size of the command buffer */
#define MAXNAME	10		/* size of a : command name */

#define NO_ADDR			1
#define NO_ARG			8
#define ONE_ARG			16
#define	MAX_ONE_ARG		32
#define ONE_FILE		64
#define	MAX_ONE_FILE	128

static char *altfile = NULL;	/* alternate file */
static struct stat buf;
static int c_argc = 0;
static char *c_argv[9];

char *nowrtmsg = "No write@since last change (:%s! overrides)";
char *morefiles = "more files@to edit";
char *ambigous = "Ambigous|Too many file names";
char *ambvalue = "Ambigous|Too many values";
char *extra = "Extra chars|Extra characters at end of command";
char *noaddr = "No address allowed@on this command";
char *noval = "No value@for binary operation";
int repl_count = -1;
int addr_flag;

PTR start_addr;
PTR end_addr;

extern core_t core;
extern state_t state;

extern char *name;		/* actual filename */
extern char **files;		/* used for "next" and "rewind" */
extern int numfiles, curfile;
extern int errno;
extern struct BLOCK_ data_block[BLK_COUNT];

static char oldbuf[CMDSZ];		/** for :!! command **/

/* Record commands history */
void record_cmd(cmdline)
char *cmdline;
{
	
}

/*
 * docmdline() - handle a colon command
 *
 * Handles a colon command received interactively by getcmdln() or from
 * the environment variable "BVIINIT" (or eventually .bvirc).
 */
void docmdline(cmdline)
char *cmdline;
{
	char buff[CMDSZ];
	char cmdbuf[CMDSZ];
	char cmdname[MAXNAME];
	char luacmdbuf[CMDSZ];
	struct key *key_tmp;
	char *cmd;
	char *p;
	size_t len;
	int n, ok;
	int force = 0;
	int saveflag;
	int shresult;
	int map_toggle = 0;

	if (cmdline == NULL)
		return;
	if (*cmdline == '"')
		return;			/** comment **/
	if (strlen(cmdline) > CMDSZ - 2) {
		ui__ErrorMsg("Command too long");
		return;
	}
	strcpy(buff, cmdline);
	cmd = buff;

	/* With no address given, we start at the beginning of the file and
	 * go to the end of the file (not line like in vi).
	 */
	addr_flag = 0;
	start_addr = mem;
	end_addr = maxpos - 1;
	SKIP_WHITE if (*cmd == '%') {
		cmd++;
		addr_flag = 2;
	} else {
		if ((start_addr = calc_addr(&cmd, mem)) == NULL) {
			return;
		}
		if (*cmd == ',') {
			cmd++;
			addr_flag = 1;
			SKIP_WHITE
			    if ((end_addr =
				 calc_addr(&cmd, maxpos - 1)) == NULL) {
				return;
			}
		} else {
			if (addr_flag) {
				/* if we have only one address */
				end_addr = start_addr;
			}
		}
	}
	SKIP_WHITE if (start_addr > (end_addr + 1)) {
		ui__ErrorMsg("Addr1 > addr2|First address exceeds second");
		return;
	}
	if ((end_addr + 1) > maxpos) {
		ui__ErrorMsg("Not that many bytes@in buffer");
		return;
	}
	if (start_addr < mem) {
		sprintf(string, "Negative address@- first byte is %ld",
			P(P_OF));
		ui__ErrorMsg(string);
		return;
	}

	SKIP_WHITE
/**** End of address range calculation ****/
	    if (*cmd == '\0') {
		setpage(end_addr);
		return;
	}
	strcpy(cmdbuf, cmd);	/* save the unmodified command */

	if (*cmd == '!') {
		if (*(cmdbuf + 1) == '\0') {
			ui__ErrorMsg("Incomplete shell escape command@- use 'shell' to get a shell");
			return;
		}
		if (*(cmdbuf + 1) == '!') {
			if (oldbuf[0] != '\0') {
				strcpy(cmdbuf + 1, oldbuf);
				msg(oldbuf);
			} else {
				ui__ErrorMsg("No previous command@to substitute for !");
				return;
			}
		} else
			sprintf(oldbuf, "\n%s\n", cmdbuf + 1);

		if (P(P_AW)) {
			save(name, mem, maxpos, WRITE);
			edits = 0;
		}
		if (edits)
			msg("[No write]|[No write since last change]");
		savetty();
		endwin();
		shresult = system(cmdbuf + 1);
		if (shresult == SIGQUIT)
			fprintf(stderr, "[Shell execution error]");
		fprintf(stderr, "[Hit return to continue]");
		getchar();
		doupdate();
		ui__Screen_Repaint();
		clearstr();
		return;
	}

	n = 0;
	while (*cmd >= 'a' && *cmd <= 'z') {
		cmdname[n++] = *cmd++;
		if (n == MAXNAME) {
			sprintf(string, "What?|%s: Not an editor command",
				cmdbuf);
			ui__ErrorMsg(string);
			return;
		}
	}
	cmdname[n] = '\0';
	if (*cmd == '!') {
		force = 1;
		cmd++;
	}
	len = strlen(cmdname);
	SKIP_WHITE if (!strncmp("substitute", cmdname, len) && CMDLNG(10, 1)) {
		repl_count =
		    do_substitution(*cmd, cmd + 1, start_addr, end_addr);
		if (repl_count == -1) {
			ui__ErrorMsg("No previous substitute re|No previous substitute regular expression");
			return;	/* No prev subst */
		}
		ui__Screen_Repaint();
		if (!repl_count) {
			ui__ErrorMsg("Fail|Substitute pattern matching failed");
		} else if (repl_count > 1) {
			sprintf(string, "%d subs|%d substitutions", repl_count,
				repl_count);
			msg(string);
		}
		return;
	} else if (!strncmp("global", cmdname, len) && CMDLNG(6, 1)) {
		current = start_addr - 1;
		repl_count = 0;
		addch('\n');
		while ((current = searching(*cmd, cmd + 1, current,
					    end_addr,
					    FALSE | S_GLOBAL)) != 0L) {
			addch('\n');
			ui__Line_Print(current, core.screen.maxy - 1);
			repl_count++;
			if (repl_count == LINES - 2) {
				if (wait_return(FALSE))
					return;
				repl_count = 0;
			}
		}
		if (repl_count) {
			wait_return(TRUE);
		} else {
			ui__Screen_Repaint();
			ui__ErrorMsg(notfound);
		}
		return;
	} else if (cmdname[0] == 't') {
		if (!addr_flag)
			start_addr = current;
		cmd = cmdname + 1L;
		SKIP_WHITE do_mark(*cmd, start_addr);
		return;
	}

	if ((!strncmp("write", cmdname, len) && CMDLNG(5, 1))
	    || !strcmp("wq", cmdname)) {
		if (*cmd == '>') {
			if (*(++cmd) == '>') {
				cmd++;
				saveflag = APPEND;
			} else {
				ui__ErrorMsg("Write forms are 'w' and 'w>>'");
				return;
			}
		} else if (*cmd == '!') {
			ui__ErrorMsg("Not yet implemented");
			return;
		} else {
			saveflag = WRITE;
		}
		SKIP_WHITE c_argc = 0;
		if ((c_argv[c_argc] = strtok(cmd, " \t")) != NULL)
			c_argc++;
		while ((c_argv[c_argc] = strtok(NULL, " \t")) != NULL)
			c_argc++;

		if (c_argc > 1) {
			ui__ErrorMsg(ambigous);
			return;
		}
		if (c_argc == 1) {
			/* change '%' to Filename */
			while ((p = strchr(c_argv[0], '%')) != NULL
			       && *(p - 1) != '\\') {
				if (name == NULL) {
					ui__ErrorMsg("No filename@to substitute for %");
					return;
				}
				*p = '\0';
				strcpy(oldbuf, c_argv[0]);
				strcat(oldbuf, name);
				strcat(oldbuf, p + 1);
				c_argv[0] = oldbuf;
			}
		}
		if (name == NULL && c_argc != 0)
			name = strdup(c_argv[0]);
		if (force) {
			if (c_argc == 0)
				ok = save(name, start_addr, end_addr, saveflag);
			else
				ok = save(c_argv[0], start_addr, end_addr,
					  saveflag);
		} else {
			if (c_argc == 0) {
				if (P(P_RO)) {
					sprintf(string,
						"\"%s\" File is read only",
						name);
					ui__ErrorMsg(string);
					return;
				} else
					ok = save(name, start_addr, end_addr,
						  saveflag);
			} else {
				if (!stat(c_argv[0], &buf)) {
					if (saveflag == WRITE) {
						sprintf(string,
							"File exists@- use \"%s! %s\" to overwrite",
							cmdname, c_argv[0]);
						ui__ErrorMsg(string);
						return;
					} else {	/* APPEND */
/* We can only append to a regular file! */
						if (S_ISREG(buf.st_mode)) {
							if (filemode == PARTIAL)
								filemode =
								    REGULAR;
						} else if (S_ISBLK(buf.st_mode)) {
							ui__ErrorMsg("Cannot append to a block special file");
							return;
						}
						ok = save(c_argv[0], start_addr,
							  end_addr, saveflag);
					}
				} else {
					if (saveflag == APPEND) {
						ui__ErrorMsg("No such file");
						return;
					} else {	/* WRITE */
/* If we write the block of a partial file to a new file, it will
 * become a regular file!
 */
						if (filemode == PARTIAL)
							filemode = REGULAR;
						ok = save(c_argv[0], start_addr,
							  end_addr, saveflag);
					}
				}
			}
		}
		if (!strcmp("wq", cmdname)) {
			if (ok)
				quit();
		}
		return;
	}

	c_argc = 0;
	cmd = strtok(cmdbuf, " \t");
	while ((c_argv[c_argc] = strtok(NULL, " \t")) != NULL)
		c_argc++;
	if (!strncmp("map", cmdname, len) && CMDLNG(3, 2)) {
		if (c_argc == 0) {
			ui__ErrorMsg("Error: empty mapping definition!");
		} else if (c_argc == 1) {
			if (strncmp(c_argv[0], "all", 3)) {
				ui__ErrorMsg("Map what?");
			} else {
				keys__KeyMaps_Show();
			}
		} else {
			luacmdbuf[0] = '\0';
			map_toggle = 0;
			for (n = 1; n < c_argc; n++) {
				strcat(luacmdbuf, " ");
				strcat(luacmdbuf, c_argv[n]);
			}
			key_tmp = keys__KeyString_Parse(c_argv[0]);
			if (!keys__Key_Map(key_tmp, luacmdbuf))
				ui__ErrorMsg("Error: can't set new key mapping!");
			return;
		}
	} else if (!strncmp("set", cmdname, len) && CMDLNG(3, 2)) {
		if (chk_comm(NO_ADDR))
			return;
		if (c_argc == 0) {
			doset(NULL);
		} else {
			for (n = 0; n < c_argc; n++) {
				if (doset(c_argv[n]))
					return;
			}
		}
	} else if (!strncmp("block", cmdname, len) && CMDLNG(5, 2)) {
		if (c_argc == 0) {
			return;
		} else if (c_argc == 4) {
			n = atoi(c_argv[0]);
			if (n >= BLK_COUNT) {
				ui__ErrorMsg("Too big block number!");
				return;
			}
			if (atoi(c_argv[1]) < atoi(c_argv[2])) {
				data_block[n].pos_start = atoi(c_argv[1]);
				data_block[n].pos_end = atoi(c_argv[2]);
				if ((atoi(c_argv[3]) < 0) | (atoi(c_argv[3]) > 6))
					data_block[n].palette = C_HX;
				else
					data_block[n].palette = atoi(c_argv[3]);
				data_block[n].hl_toggle = 1;
				ui__Screen_Repaint();
			} else {
				ui__ErrorMsg("Wrong block start and end values!");
				return;
			}
		} else {
			ui__ErrorMsg("Wrong :block command format!");
			return;
		}
	} else if (!strncmp("lua", cmdname, len) && CMDLNG(3, 2)) {
		if (c_argc == 0) {
			ui__ErrorMsg("Error: empty lua command!");
		} else {
			luacmdbuf[0] = '\0';
			for (n = 0; n < c_argc; n++) {
				strcat(luacmdbuf, " ");
				strcat(luacmdbuf, c_argv[n]);
			}
			bvi_run_lua_string(luacmdbuf);
		}
		return;
	} else if (!strncmp("args", cmdname, len) && CMDLNG(4, 2)) {
		if (chk_comm(NO_ADDR | NO_ARG))
			return;
		string[0] = '\0';
		for (n = 0; n < numfiles; n++) {
			if (n == curfile)
				strcat(string, "[");
			strcat(string, files[n]);
			if (n == curfile)
				strcat(string, "]");
			strcat(string, " ");
		}
		msg(string);
	} else if (!strncmp("source", cmdname, len) && CMDLNG(6, 2)) {
		if (chk_comm(NO_ADDR | ONE_FILE))
			return;
		if (read_rc(c_argv[0]))
			sysemsg(c_argv[0]);
		refresh();
	} else if (!strncmp("run", cmdname, len) && CMDLNG(3, 2)) {
		if (c_argc == 0) {
			ui__ErrorMsg("Error: empty plugin name!");
		} else {
			bvi_run_lua_script(c_argv[0]);
		}
		return;
	} else if (!strncmp("cd", cmdname, len) && CMDLNG(2, 2)) {
		if (chk_comm(NO_ADDR | ONE_FILE))
			return;
		if (!force) {
			if (edits) {
				if (P(P_AW)) {
					save(name, mem, maxpos, WRITE);
					edits = 0;
				} else {
					sprintf(string, nowrtmsg, "cd");
					ui__ErrorMsg(string);
					return;
				}
			}
		}
		if (c_argc == 0) {
			sprintf(string, "%c", DELIM);
			c_argv[0] = string;
		}
		if (chdir(c_argv[0]))
			sysemsg(c_argv[0]);
	} else if (!strncmp("edit", cmdname, len) && CMDLNG(4, 1)) {
		/*
		 * The command ":e#" gets expanded to something like ":efile", so
		 * detect that case here.
		 */
		if (*cmd == 'e' && c_argc == 0) {
			if (cmd[1] == '!')
				(void)doecmd(&cmd[2], TRUE);
			else
				(void)doecmd(&cmd[1], FALSE);
			return;
		}
		if (chk_comm(NO_ADDR | MAX_ONE_FILE))
			return;
		doecmd(c_argv[0], force);
	} else if (!strncmp("file", cmdname, len) && CMDLNG(4, 1)) {
		if (chk_comm(NO_ADDR | MAX_ONE_FILE))
			return;
		if (c_argc != 0)
			name = strdup(c_argv[0]);
		fileinfo(name);
	} else if (!strncmp("read", cmdname, len) && CMDLNG(4, 1)) {
		if (chk_comm(NO_ADDR | MAX_ONE_FILE))
			return;
		/* read reads a file at EOF */
		if (c_argc == 0)
			c_argv[0] = name;
		if (!addfile(c_argv[0])) {
			edits = U_TRUNC;
			undosize = filesize;
			ui__Screen_Repaint();
		}
	} else if (!strncmp("xit", cmdname, len) && CMDLNG(3, 1)) {
		if (chk_comm(NO_ADDR | NO_ARG))
			return;
		do_exit();
	} else if (!strncmp("next", cmdname, len) && CMDLNG(4, 1)) {
		if (chk_comm(NO_ADDR | NO_ARG))
			return;
		if (!force) {
			if (edits) {
				if (P(P_AW)) {
					save(name, mem, maxpos, WRITE);
					edits = 0;
				} else {
					sprintf(string, nowrtmsg, "next");
					ui__ErrorMsg(string);
					return;
				}
			}
		}
		if ((curfile + 1) < numfiles) {
			if (force)
				stuffin(":e! ");
			else
				stuffin(":e ");
			stuffin(files[++curfile]);
			stuffin("\n");
		} else
			ui__ErrorMsg("No more files@to edit!");
	} else if (!strncmp("rewind", cmdname, len) && CMDLNG(6, 3)) {
		if (chk_comm(NO_ADDR | NO_ARG))
			return;
		if (numfiles <= 1)	/* nothing to rewind */
			return;
		if (!force) {
			if (edits) {
				if (P(P_AW)) {
					save(name, mem, maxpos, WRITE);
					edits = 0;
				} else {
					sprintf(string, nowrtmsg, "rewind");
					ui__ErrorMsg(string);
					return;
				}
			}
		}
		curfile = 0;
		if (force)
			stuffin(":e! ");
		else
			stuffin(":e ");
		stuffin(files[0]);
		stuffin("\n");
	} else if (!strncmp("append", cmdname, len) && CMDLNG(6, 1)) {
		if (chk_comm(NO_ADDR | MAX_ONE_ARG))
			return;
		do_ins_chg(start_addr, c_argc == 1 ? c_argv[0] : "a", U_APPEND);
	} else if (!strncmp("change", cmdname, len) && CMDLNG(6, 1)) {
		if (chk_comm(MAX_ONE_ARG))
			return;
		if (!addr_flag)
			start_addr = current;
		do_ins_chg(start_addr, c_argc == 1 ? c_argv[0] : "a", U_EDIT);
	} else if (!strncmp("mark", cmdname, len) && CMDLNG(4, 2)) {
		if (c_argc == 0) {
			ui__ErrorMsg("Mark what?");
			return;
		}
		if (c_argc > 1 || (strlen(c_argv[0]) > 1)) {
			ui__ErrorMsg(extra);
			return;
		}
		if (!addr_flag)
			start_addr = current;
		do_mark(c_argv[0][0], start_addr);
	} else if (!strncmp("yank", cmdname, len) && CMDLNG(4, 1)) {
		if ((yanked = yd_addr()) == 0L)
			return;
		if ((yanked = alloc_buf(yanked, &yank_buf)) == 0L)
			return;
		memcpy(yank_buf, start_addr, yanked);
		sprintf(string, "%lu bytes", (long)yanked);
		msg(string);
	} else if (!strncmp("overwrite", cmdname, len) && CMDLNG(9, 1)) {
		if (chk_comm(NO_ARG))
			return;
		if (!addr_flag)
			start_addr = current;
		do_over(start_addr, yanked, yank_buf);
	} else if (!strncmp("undo", cmdname, len) && CMDLNG(4, 1)) {
		if (chk_comm(NO_ADDR | NO_ARG))
			return;
		do_undo();
	} else if (!strncmp("version", cmdname, len) && CMDLNG(7, 2)) {
		if (chk_comm(NO_ADDR | NO_ARG))
			return;
		sprintf(string, "bvi version %s %s", VERSION, copyright);
		msg(string);
	} else if (!strncmp("shell", cmdname, len) && CMDLNG(5, 2)) {
		if (chk_comm(NO_ADDR | NO_ARG))
			return;
		do_shell();
		clear();
		ui__Screen_Repaint();
		clearstr();
	} else if (!strncmp("quit", cmdname, len) && CMDLNG(4, 1)) {
		if (chk_comm(NO_ADDR | NO_ARG))
			return;
		if (!force) {
			if (edits) {
				sprintf(string, nowrtmsg, "quit");
				ui__ErrorMsg(string);
			} else {
				if ((curfile + 1) < numfiles) {
					sprintf(string, "%d %s",
						numfiles - curfile - 1,
						morefiles);
					ui__ErrorMsg(string);
				} else
					quit();
			}
		} else
			quit();
	} else if (!strncmp("sleft", cmdname, len) && CMDLNG(5, 2)) {
		if (c_argc == 1) {
			do_logic(LSHIFT, c_argv[0]);
		} else if (c_argc == 0) {
			do_logic(LSHIFT, "1");
		} else if (c_argc == 2) {
			do_logic_block(LSHIFT, c_argv[0], atoi(c_argv[1]));
		} else {
			ui__ErrorMsg(ambvalue);
			return;
		}
	} else if (!strncmp("sright", cmdname, len) && CMDLNG(6, 2)) {
		if (c_argc == 1) {
			do_logic(RSHIFT, c_argv[0]);
		} else if (c_argc == 0) {
			do_logic(RSHIFT, "1");
		} else if (c_argc == 2) {
			do_logic_block(RSHIFT, c_argv[0], atoi(c_argv[1]));
		} else {
			ui__ErrorMsg(ambvalue);
			return;
		}
	} else if (!strncmp("rleft", cmdname, len) && CMDLNG(5, 2)) {
		if (c_argc == 1) {
			do_logic(LROTATE, c_argv[0]);
		} else if (c_argc == 0) {
			do_logic(LROTATE, "1");
		} else if (c_argc == 2) {
			do_logic_block(LROTATE, c_argv[0], atoi(c_argv[1]));
		} else {
			ui__ErrorMsg(ambvalue);
			return;
		}
	} else if (!strncmp("rright", cmdname, len) && CMDLNG(6, 2)) {
		if (c_argc == 1) {
			do_logic(RROTATE, c_argv[0]);
		} else if (c_argc == 0) {
			do_logic(RROTATE, "1");
		} else if (c_argc == 2) {
			do_logic_block(RROTATE, c_argv[0], atoi(c_argv[1]));
		} else {
			ui__ErrorMsg(ambvalue);
			return;
		}
	} else if (!strcmp("and", cmdname)) {
		if (c_argc == 1) {
			do_logic(AND, c_argv[0]);
		} else if (c_argc == 0) {
			ui__ErrorMsg(noval);
			return;
		} else if (c_argc == 2) {
			do_logic_block(AND, c_argv[0], atoi(c_argv[1]));
		} else {
			ui__ErrorMsg(ambvalue);
			return;
		}
	} else if (!strcmp("or", cmdname)) {
		if (c_argc == 1) {
			do_logic(OR, c_argv[0]);
		} else if (c_argc == 0) {
			ui__ErrorMsg(noval);
			return;
		} else if (c_argc == 2) {
			do_logic_block(OR, c_argv[0], atoi(c_argv[1]));
		} else {
			ui__ErrorMsg(ambvalue);
			return;
		}
	} else if (!strcmp("xor", cmdname)) {
		if (c_argc == 1) {
			do_logic(XOR, c_argv[0]);
		} else if (c_argc == 0) {
			ui__ErrorMsg(noval);
			return;
		} else if (c_argc == 2) {
			do_logic_block(XOR, c_argv[0], atoi(c_argv[1]));
		}
		else {
			ui__ErrorMsg(ambvalue);
			return;
		}
	} else if (!strcmp("neg", cmdname)) {
		if (c_argc != 0) {
			ui__ErrorMsg(extra);
			return;
		}
		do_logic(NEG, "255");
	} else if (!strcmp("not", cmdname)) {
		if (c_argc != 0) {
			ui__ErrorMsg(extra);
			return;
		}
		do_logic(NOT, "255");
	} else {
		sprintf(string, "What?|%s: Not an editor command", cmd);
		if P
			(P_MM) {
			if (!strncmp("delete", cmdname, len) && CMDLNG(6, 1)) {
				if ((undo_count = yd_addr()) == 0L)
					return;
				do_delete(undo_count, start_addr);
				sprintf(string, "%lu bytes", (long)undo_count);
				msg(string);
			} else if (!strncmp("insert", cmdname, len)
				   && CMDLNG(6, 1)) {
				if (chk_comm(MAX_ONE_ARG))
					return;
				if (!addr_flag)
					start_addr = current;
				do_ins_chg(start_addr - 1,
					   c_argc == 1 ? c_argv[0] : "a",
					   U_INSERT);
			} else if (!strncmp("put", cmdname, len)
				   && CMDLNG(3, 2)) {
				if (chk_comm(NO_ARG))
					return;
				if (!addr_flag)
					start_addr = current;
				do_put(start_addr, yanked, yank_buf);
			} else {
				ui__ErrorMsg(string);
			}
		} else {
			if (!strncmp("delete", cmdname, len) && CMDLNG(6, 1)) {
				movebyte();
			} else if (!strncmp("insert", cmdname, len)
				   && CMDLNG(6, 1)) {
				movebyte();
			} else if (!strncmp("put", cmdname, len)
				   && CMDLNG(3, 2)) {
				movebyte();
			} else {
				ui__ErrorMsg(string);
			}
		}
	}
}

/* calculate address range for :yank and :delete command */
off_t yd_addr()
{
	off_t count = 0;

	if (c_argc == 0) {
		switch (addr_flag) {
		case 0:
			start_addr = current;
		case 1:
			count = 1;
			break;
		case 2:
			count = end_addr - start_addr + 1;
			break;
		}
	} else if (c_argc == 1) {
		count = atoi(c_argv[0]);
		switch (addr_flag) {
		case 0:
			start_addr = current;
		case 1:
			end_addr = start_addr + count - 1;
			break;
		case 2:
			start_addr = end_addr;
			end_addr = start_addr + count - 1;
			break;
		}
	} else {
		ui__ErrorMsg(ambvalue);
		return 0;
	}
	return count;
}

void do_exit()
{
	if (edits) {
		if (!save(name, mem, maxpos - 1L, WRITE))
			return;
	}
	if ((curfile + 1) < numfiles) {
		sprintf(string, "%d %s", numfiles - curfile - 1, morefiles);
		ui__ErrorMsg(string);
	} else {
#ifdef HAVE_LUA_H
		bvi_lua_finish();
#endif
		keys__Destroy();
		quit();
	}
}

int doecmd(arg, force)
char *arg;
int force;
{
	char *tmp;

	if (*arg == '\0')
		arg = NULL;
	if (!force && edits) {
		sprintf(string, nowrtmsg, "edit");
		ui__ErrorMsg(string);
		/*
		   if (altfile)
		   free(altfile);
		   altfile = strdup(arg);
		 */
		return FALSE;
	}
	if (arg != NULL) {
		if (name != NULL && !strcmp(arg, name)) {
			if (!edits || (edits && !force))
				return TRUE;
		}
		if (name != NULL && !strcmp(arg, "#")) {
			if (altfile == NULL) {
				ui__ErrorMsg("No alternate filename@to substitute for #");
				return FALSE;
			}
			tmp = name;
			name = altfile;
			altfile = tmp;
		} else {
			if (altfile) {
				free(altfile);
			}
			altfile = name;
			name = strdup(arg);
		}
	}
	if (name == NULL) {
		ui__ErrorMsg("No file|No current filename");
		return FALSE;
	}

	edits = 0;
	filesize = load(name);
	if (arg == NULL) {
		setpage(current < maxpos ? current : maxpos - 1L);
	}
	return TRUE;
}

/* If flag == TRUE we do a ui__Screen_Repaint
 *
 */
int wait_return(flag)
int flag;
{
	int c;

	signal(SIGINT, jmpproc);
	clearstr();
	attrset(A_REVERSE);
	mvaddstr(core.screen.maxy, 0, "[Hit return to continue]");
	attrset(A_NORMAL);
	refresh();
	c = getch();
	if (flag) {
		clear();
		ui__Screen_Repaint();
	}
	clearstr();
	signal(SIGINT, SIG_IGN);
	if (c != CR && c != NL && c != ' ' && c != ':' && c != KEY_ENTER)
		return 1;
	return 0;
}

int chk_comm(flag)
int flag;
{
	if ((flag & NO_ADDR) && (addr_flag > 0)) {
		ui__ErrorMsg(noaddr);
		return 1;
	}
	if ((flag & NO_ARG) && (c_argc > 0)) {
		ui__ErrorMsg(extra);
		return 1;
	}
	if ((flag & MAX_ONE_ARG) && (c_argc > 1)) {
		ui__ErrorMsg(ambvalue);
		return 1;
	}
	if ((flag & ONE_FILE) && (c_argc == 0)) {
		ui__ErrorMsg("Missing filename");
		return 1;
	}
	if ((flag & MAX_ONE_FILE) && (c_argc > 1)) {
		ui__ErrorMsg(ambigous);
		return 1;
	}
	return 0;
}
