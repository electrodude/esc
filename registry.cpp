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
	blocks[factory->name()] = factory;
}

BlockFactory* CompilerRegistry::getBlock(std::string id)
{
	return blocks[id];
}
