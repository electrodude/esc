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
