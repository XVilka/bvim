/* Bvim - BVi IMproved, binary analysis framework
 *
 * Copyright 1996-2004 by Gerhard Buergmann <gerhard@puon.at>
 * Copyright 2011 by Anton Kochkov <anton.kochkov@gmail.com>
 *
 * This file is part of Bvim.
 *
 * Bvim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bvim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bvim.  If not, see <http://www.gnu.org/licenses/>.
 */ 

#include <unistd.h>
#include <dlfcn.h>
#include "bvim.h"
#include "bscript.h"
#include "keys.h"
#include "commands.h"
#include "blocks.h"
#include "plugins.h"

//extern core_t core;
//extern state_t state;
//extern api_t api;

static plugin_link plugins;

// TODO: Add plugin dependency support

/* -------------------------------------------------------------------------
 *                       Internals
 * -------------------------------------------------------------------------
 */

// TODO: implement parsing C structures as plugin
// TODO: implement parsing C++ objects as plugin
//
int PluginAdd(core_t *core, plugin_t i)
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

int PluginDel(core_t *core, plugin_t i)
{
	return 0;
}

/* --------------------------------------------------------------------------
 *                      Exported plugins API
 * --------------------------------------------------------------------------
 */


int plugins__Init(core_t *core)
{
	core->api.error = bvim_error;
	core->api.info = bvim_info;
	core->api.debug = bvim_debug;
	core->api.blk.iterator = blocks__Iterator;
	core->api.blk.add = blocks__Add;
	core->api.blk.del_by_id = blocks__DelByID;
	core->api.blk.del_by_name = blocks__DelByName;
	core->api.blk.get_by_id = blocks__GetByID;
	core->api.blk.get_by_name = blocks__GetByName;

	return 0;
}

plugin_t* plugins__GetByName(core_t *core, char* name) {
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

int plugins__Destroy(core_t *core)
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

int plugin__Load(core_t *core, char* path)
{
	int i = 0;
	char *msg;
	void *module;
	plugin_t plg;

	plugin_t (*plugin_register)();
	int (*plugin_init)(core_t *);

	/* Check if file exist and valid */
	if (access(path, R_OK) != 0) {
		bvim_error(core->curbuf->state.mode, "Can't read/find plugin *.so file");
		return -1;
	}
	module = dlopen(path, RTLD_NOW);
	if (!module) {
		msg = dlerror();
		if (msg != NULL) {
			dlclose(module);
			bvim_error(core->curbuf->state.mode, "plugin load error: %s", msg);
			return -1;
		}
	}
	// load plugin_register() symbol and call it
	plugin_register = dlsym(module, "plugin_register");
	msg = dlerror();
	if (msg != NULL) {
		dlclose(module);
		bvim_error(core->curbuf->state.mode, "plugin load error: can't find plugin_register() function; %s", msg);
		return -1;
	}
	if (plugin_register != NULL) {
		plg = plugin_register();
		plg.module = module;
		PluginAdd(core, plg);
		// Add commands, key handlers, etc to lists
		plugin_init = dlsym(module, "plugin_init");
		msg = dlerror();
		if (msg != NULL) {
			dlclose(plg.module);
			bvim_error(core->curbuf->state.mode, "plugin init error: can't find plugin_init() function: %s", msg);
			return -1;
		}
		if (plugin_init != NULL) {
			plugin_init(core, &state);
			if (plg.exports.keys != NULL) {
				i = 0;
				while (plg.exports.keys[i].id != 0)
				{
					plg.exports.keys[i].handler.func = dlsym(plg.module, plg.exports.keys[i].handler.func_name);
					plg.exports.keys[i].handler_type = BVI_HANDLER_INTERNAL; // was external before loading
					keys__Key_Map(core, &(plg.exports.keys[i]));
					i++;
				}
			}
			if (plg.exports.cmds != NULL) {
				i = 0;
				while (plg.exports.cmds[i].id != 0)
				{
					plg.exports.cmds[i].handler.func = dlsym(plg.module, plg.exports.cmds[i].handler.func_name);
					plg.exports.cmds[i].handler_type = BVI_HANDLER_INTERNAL; // was external before loading
					commands__Cmd_Add(core, &(plg.exports.cmds[i]));
					i++;
				}
			}
			if (plg.exports.luaF_list != NULL) {
				i = 0;
				while (plg.exports.luaF_list[i].id != 0)
				{
					plg.exports.luaF_list[i].handler.func = dlsym(plg.module, plg.exports.luaF_list[i].handler.func_name);
					plg.exports.luaF_list[i].handler_type = BVI_HANDLER_INTERNAL; // was external before loading
					luaF_Add(core, plg.exports.luaF_list[i]);
					i++;
				}
			}

		} else {
			dlclose(module);
			bvim_error(core->curbuf->state.mode, "plugin init error: wrong plugin_init() function");
			return -1;
		}
	} else {
		dlclose(module);
		bvim_error(core->curbuf->state.mode, "plugin load error: wrong plugin_register() function");
		return -1;
	}
	return 0;
}

// Unload plugin, remove from list
int plugin__Unload(core_t *core, plugin_t *plg)
{
	int i = 0;
	char* msg = NULL;
	int (*plugin_unregister)();

	plugin_unregister = dlsym(plg->module, "plugin_unregister");
	msg = dlerror();
	if (msg != NULL) {
		dlclose(plg->module);
		bvim_error(core->curbuf->state.mode, "plugin unload error: can't find plugin_unregister() function");
		return -1;
	}
	/* Unregistering commands, hotkeys, lua functions */
	if (plugin_unregister != NULL) {
		plugin_unregister();
		if (plg->exports.keys != NULL) {
				i = 0;
				while (plg->exports.keys[i].id != 0)
				{
					keys__Key_Unmap(core, &(plg->exports.keys[i]));
					i++;
				}
			}
			if (plg->exports.cmds != NULL) {
				i = 0;
				while (plg->exports.cmds[i].id != 0)
				{
					commands__Cmd_Del(core, plg->exports.cmds[i].name);
					i++;
				}
			}
			if (plg->exports.luaF_list != NULL) {
				i = 0;
				while (plg->exports.luaF_list[i].id != 0)
				{
					luaF_DelByID(core, plg->exports.luaF_list[i].id);
					i++;
				}
			}

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


