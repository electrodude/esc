#include "registry.hpp"

CompilerRegistry registry;

CompilerRegistry::CompilerRegistry()
{

}


CompilerRegistry::~CompilerRegistry()
{

}

void CompilerRegistry::operator()(BlockFactory* factory)
{
	blocks[factory->getName()] = factory;
}

BlockFactory* CompilerRegistry::getBlock(std::string id)
{
	return blocks[id];
}

std::map<std::string, BlockFactory*> CompilerRegistry::getDefaultBlocks()
{
	// TODO: don't set these every time
	defaultblocks["con"] = getBlock("con");
	defaultblocks["obj"] = getBlock("obj");
	defaultblocks["var"] = getBlock("var");
	defaultblocks["pub"] = getBlock("pub");
	defaultblocks["pri"] = getBlock("pri");
	defaultblocks["dat"] = getBlock("dat");

	return defaultblocks;
}

