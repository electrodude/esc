#pragma once

#include <map>
#include <string>
#include <sstream>

#include "misc.hpp"

#include "registry.hpp"
#include "symtab.hpp"

class SpinObject
{
public:
	SpinObject(std::string code);

	std::map<std::string, BlockFactory*> blocktypes;
};

class Compiler
{
public:
	Compiler();

	SpinObject* compileFile(Path path);

	std::map<Path, SpinObject*> objects;
};


