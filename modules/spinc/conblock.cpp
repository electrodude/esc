#include <iostream>
#include <string>

#include "spinc.hpp"

CONBlock::CONBlock(std::string code)
{
	std::cout << "CON block: " << code << '\n';
}
	

CONBlockFactory::CONBlockFactory()
{
	printf("CON block registered!\n");
}

Block* CONBlockFactory::newBlock(std::string code) const
{
	return new CONBlock(code);
}

std::string CONBlockFactory::getName() const
{
	return "con";
}

