#include "loader.hpp"

typedef CompilerModule* (*loadfunc_t)(CompilerRegistry*);

typedef void (*unloadfunc_t)(CompilerModule*);

std::map<std::string, CompilerModule*> modules;

CompilerModule* ModuleLoader::load(std::string path)
{
	char* cpath = realpath(path.c_str(), NULL);
	path = std::string(cpath);
	free(cpath);
	CompilerModule* mod = (new ModuleLoader(path))->getModule();

	modules[path] = mod;

	return mod;
}

void chkerr()
{
	char* err = dlerror();
	if (err != NULL)
	{
		printf("dl error: %s\n", err);
		throw err;
	}
}

ModuleLoader::ModuleLoader(std::string path)
{
	lib = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);	// TODO: find a way to make this not global
	chkerr();
	loadfunc_t loadfunc = (loadfunc_t)dlsym(lib, "loadModule");
	chkerr();
	mod = loadfunc(&registry);

	mod->loader = this;
}

ModuleLoader::~ModuleLoader()
{
	unloadfunc_t unloadfunc = (unloadfunc_t)dlsym(lib, "unloadModule");
	chkerr();
	unloadfunc(mod);
	dlclose(lib);
	chkerr();
}

CompilerModule* ModuleLoader::getModule() const
{
	return mod;
}
