#include "loader.hpp"

typedef CompilerModule* (*loadfunc_t)();

typedef void (*unloadfunc_t)(CompilerModule*);

void chkerr()
{
	char* err = dlerror();
	if (err != NULL)
	{
		printf("dl error: %s\n", err);
		throw err;
	}
}

module::module(const char* path)
{
	lib = dlopen(path, RTLD_LAZY | RTLD_GLOBAL);	// TODO: find a way to make this not global
	chkerr();
	loadfunc_t loadfunc = (loadfunc_t)dlsym(lib, "loadModule");
	chkerr();
	mod = loadfunc();
}

module::~module()
{
	unloadfunc_t unloadfunc = (unloadfunc_t)dlsym(lib, "unloadModule");
	chkerr();
	unloadfunc(mod);
	dlclose(lib);
	chkerr();
}

CompilerModule* module::getModule() const
{
	return mod;
}
