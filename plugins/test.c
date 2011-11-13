#include <stdlib.h>
#include "plugins.h"
#include "data.h"

core_t *core;
state_t *state;

void (*plugin_error)(int, char*, ...);
void (*plugin_info)(int, char*, ...);
void (*plugin_debug)(int, char*, ...);

struct command cmds[] = {
	{ 35, "test", "do nothing", 1, BVI_HANDLER_EXTERNAL, { .func_name = "plg_command_test" }, 4, 1},
	{ 0, NULL, NULL, 0, 0, { NULL }, 0, 0}
};

struct key keys[] = {
	{ BVI_CTRL('P'), "Ctrl-P", "", 1, BVI_HANDLER_EXTERNAL, { .func_name = "plg_key_test" }},
	{ 0, NULL, NULL, 0, 0, { NULL }}
};

plugin_t plugin_register()
{
	plugin_t plg;

	plg.name = "test";
	plg.author = "Anton Kochkov";
	plg.license = "GPLv2";
	plg.version.major = 0;
	plg.version.minor = 1;
	plg.description = "Just test plugin, for example purposes";
	plg.module = NULL;
	plg.exports.keys = keys;
	plg.exports.cmds = cmds;
	return plg;
}

int plugin_init(core_t *bvi_core, state_t *bvi_state)
{
	plugin_error = bvi_core->error;
	plugin_info = bvi_core->info;
	plugin_debug = bvi_core->debug;
	core = bvi_core;
	state = bvi_state;
	return 0;
}

/* ---------------------------------------------------------------
 *                 exported commands
 * ---------------------------------------------------------------
 */

int plg_command_test(char flags, int c_argc, char** c_argv)
{
	plugin_info(state->mode, "Command from \"%s\" plugin successfully executed!", "test");
	return 0;
}

int plg_key_test()
{
	plugin_info(state->mode, "Key, defined in \"%s\" plugin, pressed!", "test");
	return 0;
}
