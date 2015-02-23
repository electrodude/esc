#include <map>
#include <string>

#include <cstdlib>

#include "loader.hpp"

#include "main.hpp"


CompilerRegistry registry;

std::map<std::string, CompilerModule*> modules;

CompilerModule* loadModule(std::string path)
{
	char* cpath = realpath(path.c_str(), NULL);
	path = std::string(cpath);
	free(cpath);
	CompilerModule* mod = (new ModuleLoader(path, &registry))->getModule();

	modules[path] = mod;

	return mod;
}

int main(int argc, char** argv)
{
	CompilerModule* mod = loadModule("modules/spinc.so");
}
