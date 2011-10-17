#define FLAG_FORCE 1

struct command {
	int id;
	char *name;
	char *description;
	short enabled;
	short handler_type;
	union {
		char *lua_cmd;
		char *int_cmd;
		int (*func) (char, int, char **);
	} handler;
	int size1;
	int size2;
};

struct command_array {
	struct command *arr;
	int items;
	int allocated;
};

int command__help(char, int, char**);
int command__map(char, int, char**);
int command__unmap(char, int, char**);
int command__set(char, int, char**);
int command__block(char, int, char**);
int command__lua(char, int, char**);
int command__args(char, int, char**);
int command__source(char, int, char**);
int command__run(char, int, char**);
int command__cd(char, int, char**);
int command__edit(char, int, char**);
int command__file(char, int, char**);
int command__read(char, int, char**);
int command__xit(char, int, char**);
int command__next(char, int, char**);
int command__rewind(char, int, char**);
int command__append(char, int, char**);
int command__change(char, int, char**);
int command__mark(char, int, char**);
int command__yank(char, int, char**);
int command__overwrite(char, int, char**);
int command__undo(char, int, char**);
int command__version(char, int, char**);
int command__shell(char, int, char**);
int command__quit(char, int, char**);
int command__sleft(char, int, char**);
int command__sright(char, int, char**);
int command__rleft(char, int, char**);
int command__rright(char, int, char**);
int command__and(char, int, char**);
int command__or(char, int, char**);
int command__xor(char, int, char**);
int command__neg(char, int, char**);
int command__not(char, int, char**);

void commands__Init();
void commands__Destroy();
int commands__Cmd_Add(struct command*);
int commands__Cmd_Del(char *name);

