#pragma once

#include <cstdio>

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

class module
{
public:
	module(const char* path);

	virtual ~module();

	virtual CompilerModule* getModule() const;

protected:
	library* lib;

	CompilerModule* mod;
};


