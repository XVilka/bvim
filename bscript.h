void bvi_lua_init(void);
void bvi_lua_finish(void);
int bvi_run_lua_string(char *string);
int bvi_run_lua_script(char *name);
int bvi_repl_read();
int bvi_repl_eval(char* line);
