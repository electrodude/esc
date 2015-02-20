#pragma once

#include "module.hpp"

class BlockFactory;

class CompilerRegistry
{
public:
	CompilerRegistry();

	virtual ~CompilerRegistry();

	void operator()(BlockFactory* factory);
};
