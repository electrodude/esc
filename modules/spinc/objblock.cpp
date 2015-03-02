#include <iostream>
#include <string>

#include "spinc.hpp"

OBJBlock::OBJBlock(std::string code)
{
	std::cout << "OBJ block: " << code << '\n';
}


OBJBlockFactory::OBJBlockFactory()
{
	printf("OBJ block registered!\n");
}

Block* OBJBlockFactory::newBlock(std::string code)
{
	return new OBJBlock(code);
}

std::string OBJBlockFactory::getName() const
{
	return "obj";
}
