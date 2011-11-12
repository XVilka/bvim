#include <stdlib.h>
#include "plugins.h"
#include "data.h"

/*
 * load error handler from core.error
 * load info handler from core.info
 * load debug handler from core.debug
 */

plugin_t plugin_register()
{
	plugin_t plg;
	struct command cmds[] = {
		{ 35, "test", "do nothing", 1, BVI_HANDLER_EXTERNAL, { .func_name="plg_command_test" }, 4, 1},
		{ 0, NULL, NULL, 0, 0, { NULL }, 0, 0}
	};

	plg.name = "test";
	plg.author = "Anton Kochkov";
	plg.license = "GPLv2";
	plg.version.major = 0;
	plg.version.minor = 1;
	plg.description = "Just test plugin, for example purposes";
	plg.module = NULL;
	plg.exports.keys = NULL;
	plg.exports.cmds = cmds;
	return plg;
}

int plugin_init(core_t *core, state_t *state)
{
	return 0;
}

/* ---------------------------------------------------------------
 *                 exported commands
 * ---------------------------------------------------------------
 */

int plg_command_test(char flags, int c_argc, char** c_argv)
{
	// do nothing
	return 0;
}
