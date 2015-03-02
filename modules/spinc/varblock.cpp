#include <iostream>
#include <string>

#include "spinc.hpp"

VARBlock::VARBlock(std::string code)
{
	std::cout << "VAR block: " << code << '\n';
}


VARBlockFactory::VARBlockFactory()
{
	printf("VAR block registered!\n");
}

Block* VARBlockFactory::newBlock(std::string code)
{
	return new VARBlock(code);
}

std::string VARBlockFactory::getName() const
{
	return "var";
}

