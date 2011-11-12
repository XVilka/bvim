#include <dlfcn.h>
#include "bvi.h"
#include "keys.h"
#include "commands.h"
#include "blocks.h"
#include "plugins.h"

extern core_t core;
extern state_t state;

static plugin_link plugins;

/* -------------------------------------------------------------------------
 *                       Internals
 * -------------------------------------------------------------------------
 */

// TODO: implement parsing C structures as plugin
// TODO: implement parsing C++ objects as plugin
//
int PluginAdd(plugin_t i)
{
	plugin_link t = NULL;
	t = (plugin_link)malloc(sizeof(*t));
	if (t != NULL) {
		t->item = i;
		if (plugins != NULL) {
			t->next = plugins->next;
			plugins->next = t;
		} else {
			plugins = t;
			plugins->next = NULL;
		}
	} else {
		return -1;
	}
	return 0;
}

int PluginDel(plugin_t i)
{
	return 0;
}

/* --------------------------------------------------------------------------
 *                      Exported plugins API
 * --------------------------------------------------------------------------
 */


int plugins__Init()
{
	return 0;
}

plugin_t* plugins__GetByName(char* name) {
	plugin_link t;

	t = plugins;

	while (t != NULL)
	{
		FILE *fp = fopen("debugging.log", "a");
		fprintf(fp, "plugins__GetByName(%s): t != NULL\n\t\tt->item.name = %s\n", name, name);
		fclose(fp);

		if (!strcmp(t->item.name, name)) return &(t->item);
		t = t->next;
	}
	return NULL;
}

int plugins__Destroy()
{
	plugin_link t;

	t = plugins;
	while (t != NULL)
	{
		free(t);
		t = t->next;
	}
	return 0;
}

int plugin__Load(char* path)
{
	int i = 0;
	char *msg;
	void *module;
	plugin_t plg;

	plugin_t (*plugin_register)();
	int (*plugin_init)(core_t *, state_t *);

	module = dlopen(path, RTLD_NOW);
	if (!module) {
		msg = dlerror();
		if (msg != NULL) {
			dlclose(module);
			bvi_error(state.mode, "plugin load error: %s", msg);
			return -1;
		}
	}
	// load plugin_register() symbol and call it
	plugin_register = dlsym(module, "plugin_register");
	msg = dlerror();
	if (msg != NULL) {
		dlclose(module);
		bvi_error(state.mode, "plugin load error: can't find plugin_register() function; %s", msg);
		return -1;
	}
	if (plugin_register != NULL) {
		plg = plugin_register();
		plg.module = module;
		PluginAdd(plg);
		// Add commands, key handlers, etc to lists
		plugin_init = dlsym(module, "plugin_init");
		msg = dlerror();
		if (msg != NULL) {
			dlclose(plg.module);
			bvi_error(state.mode, "plugin init error: can't find plugin_init() function: %s", msg);
			return -1;
		}
		if (plugin_init != NULL) {
			plugin_init(&core, &state);
			if (plg.exports.keys != NULL) {
				i = 0;
				while (plg.exports.keys[i].id != 0)
				{
					plg.exports.keys[i].handler.func = dlsym(plg.module, plg.exports.keys[i].handler.func_name);
					plg.exports.keys[i].handler_type = BVI_HANDLER_INTERNAL; // was external before loading
					keys__Key_Map(&(plg.exports.keys[i]));
					i++;
				}
			}
			if (plg.exports.cmds != NULL) {
				i = 0;
				while (plg.exports.cmds[i].id != 0)
				{
					plg.exports.cmds[i].handler.func = dlsym(plg.module, plg.exports.cmds[i].handler.func_name);
					plg.exports.cmds[i].handler_type = BVI_HANDLER_INTERNAL; // was external before loading
					commands__Cmd_Add(&(plg.exports.cmds[i]));
					i++;
				}
			}
		} else {
			dlclose(module);
			bvi_error(state.mode, "plugin init error: wrong plugin_init() function");
			return -1;
		}
	} else {
		dlclose(module);
		bvi_error(state.mode, "plugin load error: wrong plugin_register() function");
		return -1;
	}
	return 0;
}

// Unload plugin, remove from list
int plugin__Unload(plugin_t plg)
{
	char* msg = NULL;
	int (*plugin_unregister)();

	plugin_unregister = dlsym(plg.module, "plugin_unregister");
	msg = dlerror();
	if (msg != NULL) {
		dlclose(plg.module);
		bvi_error(state.mode, "plugin unload error: can't find plugin_unregister() function");
		return -1;
	}
	if (plugin_unregister != NULL) {
		plugin_unregister();
	}
	dlclose(plg.module);
	PluginDel(plg);
	return 0;
}


/* -----------------------------------------------------------------------------------
 *                        Working with commands, blocks,
 *                        lua functions, core structures, etc.
 * -----------------------------------------------------------------------------------
 */


