#pragma once

#include <cstdio>
#include <map>
#include <string>

#ifdef __linux__
#include <dlfcn.h>
typedef void* library;
typedef void* function;
#elif _WIN32
#include <windows.h>
#define dlopen(path, flags) LoadLibrary(path)
typedef HMODULE library;
#endif

#include "module.hpp"
#include "registry.hpp"

extern std::map<std::string, CompilerModule*> modules;

class ModuleLoader
{
public:
	ModuleLoader(std::string path);

	static CompilerModule* load(std::string path);

	virtual ~ModuleLoader();

	virtual CompilerModule* getModule() const;

protected:
	library lib;

	CompilerModule* mod;
};


