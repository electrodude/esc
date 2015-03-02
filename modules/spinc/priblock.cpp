#include <iostream>
#include <string>

#include "spinc.hpp"

PRIBlock::PRIBlock(std::string code)
{
	std::cout << "PRI block: " << code << '\n';
}


PRIBlockFactory::PRIBlockFactory()
{
	printf("PRI block registered!\n");
}

Block* PRIBlockFactory::newBlock(std::string code)
{
	return new PRIBlock(code);
}

std::string PRIBlockFactory::getName() const
{
	return "pri";
}

