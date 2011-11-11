#include <dlfcn.h>
#include "bvi.h"
#include "plugins.h"

extern core_t core;
extern state_t state;

// TODO: implement parsing C structures as plugin
// TODO: implement parsing C++ objects as plugin

// Add loaded plugins list
int plugins__Load(char* name)
{
	char *msg;
	void *module;
	module = dlopen(name, RTLD_NOW);
	if (!module) {
		msg = dlerror();
		if (msg != NULL) {
			dlclose(module);
			bvi_error(state.mode, "plugin load error: %s", msg);
		}
	}
	return 0;
}

// Unload plugin, remove from list
int plugins__Unload(char* name)
{
	return 0;
}
