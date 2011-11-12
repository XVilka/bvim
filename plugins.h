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

int plugins__Init();
plugin_t* plugins__GetByName(char* name);
int plugins__Destroy();

int plugin__Load(char *path);
int plugin__Unload(plugin_t plg);
