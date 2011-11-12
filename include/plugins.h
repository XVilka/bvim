/* Exported types:
 * int command__handler(char, int, char**); -  for commands handlers
 * int handler__func() - for key handlers
 * bvi_add_lua_function - not yet implemented
 */

typedef struct plugin *plugin_link;

struct plugin_ {
	char* name;
	char* author;
	char* license;
	struct {
		short major;
		short minor;
	} version;
	char* description;
	void* module;
	/*
	 struct {
		commands_list cmd_lst;
		keys_list key_lst;
		lua_list lua_lst;
	} exports;
	*/
};

typedef struct plugin_ plugin_t;

struct plugin {
	plugin_t item;
	plugin_link next;
};

/* Plugin API:
 *
 * plugin_t plugin_register() function - fill plugin info
 *
 */

/* ---------------------------------------------------------
 *               for using only in bvi
 * ---------------------------------------------------------
 */

int plugins__Init();
plugin_t* plugins__GetByName(char* name);
int plugins__Destroy();

int plugin__Load(char *path);
int plugin__Unload(plugin_t plg);
