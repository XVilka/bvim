#define LUA_DEFAULT_SCRIPT_PATH "scripts"

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

void bvim_lua_init(void);
void bvim_lua_finish(void);
int bvim_run_lua_string(char *string);
int bvim_run_lua_script(char *name);
int bvim_repl_read();
int bvim_repl_eval(char* line);

int luaF_Add(struct luaF_item b);
int luaF_Iterator(int (*(func))(), int result);
int luaF_DelByName(char *name);
int luaF_DelByID(int id);
struct luaF_item* luaF_GetByName(char* name);
struct luaF_item* luaF_GetByID(unsigned int id);
