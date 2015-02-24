#pragma once

#include <string>

#include "symtab.hpp"
#include "registry.hpp"

class Block
{
public:
	Block() {};

	virtual ~Block() {};

	virtual void foo() = 0;

	Namespace ns;
};

class BlockFactory
{
public:
	BlockFactory() {};

	virtual ~BlockFactory() {};

	virtual Block* newBlock(std::string code) = 0;

	virtual std::string name() = 0;
};

class CompilerRegistry;
class ModuleLoader;

class CompilerModule
{
public:
//	CompilerModule();

	virtual ~CompilerModule() {};

	ModuleLoader* loader;
};

extern "C" CompilerModule* loadModule(CompilerRegistry& registry);

extern "C" void unloadModule(CompilerModule* module);
