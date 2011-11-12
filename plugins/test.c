#include <stdlib.h>
#include "../plugins.h"

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

	return plg;
}
