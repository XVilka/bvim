/* Exported types:
 * int command__handler(char, int, char**); -  for commands handlers
 * int handler__func() - for key handlers
 * bvim_add_lua_function - not yet implemented
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
	 struct {
		struct key *keys;
		struct command *cmds;
		struct luaF_item *luaF_list;
	} exports;
};

typedef struct plugin_ plugin_t;

struct plugin {
	plugin_t item;
	plugin_link next;
};

/* Plugin API:
 *
 * plugin_t plugin_register() function - fill plugin info
 * plugin_init(plugin_t plugin);
 *
 * Import core, state, blocks, commands and keys structure.
 *
 * plugin_unregister() function - remove plugin info,
 * remove previously exported handlers, keys, commands, lua 
 * functions, etc.
 */

// TODO: Export all data as set of objects
// TODO: and export iterator functions on these sets

/* ---------------------------------------------------------
 *               for using only in bvimm
 * ---------------------------------------------------------
 */

int plugins__Init();
plugin_t* plugins__GetByName(char* name);
int plugins__Destroy();

int plugin__Load(char *path);
int plugin__Unload(plugin_t plg);
