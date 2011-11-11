#define lua_setConst(state, name) { lua_pushnumber(state, name); lua_setglobal(state, #name ); }

typedef struct lua_io *lua_io_link;

struct lua_io_item {
	char* lua_input;
	char* lua_output;
	char* lua_error;
};

struct lua_io { 
	struct lua_io_item item;
	lua_io_link next;
};

void bvi_lua_init(void);
void bvi_lua_finish(void);
int bvi_run_lua_string(char *string);
int bvi_run_lua_script(char *name);
int bvi_repl_read();
int bvi_repl_eval(char* line);
