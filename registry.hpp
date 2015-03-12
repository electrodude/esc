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

	void operator()(const BlockFactory* factory);

	const BlockFactory* getBlock(std::string id);

	std::map<std::string, const BlockFactory*> getDefaultBlocks();

private:
	std::map<std::string, const BlockFactory*> defaultblocks;

	std::map<std::string, const BlockFactory*> blocks;
};

extern CompilerRegistry registry;
