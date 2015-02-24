#pragma once

#include <string>
#include <map>

#include "module.hpp"

class BlockFactory;

class CompilerRegistry
{
public:
	CompilerRegistry();

	virtual ~CompilerRegistry();

	void operator()(BlockFactory* factory);

	BlockFactory* getBlock(std::string id);

	std::map<std::string, BlockFactory*> blocks;
};

extern CompilerRegistry registry;
