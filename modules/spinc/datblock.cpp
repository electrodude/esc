#include <iostream>
#include <string>

#include "spinc.hpp"

DATBlock::DATBlock(std::string code)
{
	std::cout << "DAT block: " << code << '\n';
}


DATBlockFactory::DATBlockFactory()
{
	printf("DAT block registered!\n");
}

Block* DATBlockFactory::newBlock(std::string code)
{
	return new DATBlock(code);
}

std::string DATBlockFactory::getName() const
{
	return "dat";
}

