#pragma once

#include "symtab.hpp"

class Block
{
public:
	//Block(const char* code) = 0;
	

	virtual void foo() = 0;

	Namespace ns;
};

class CompilerModule
{
public:
	CompilerModule() {};

	virtual ~CompilerModule() {};

	virtual Block* newBlock(int id, const char* code) = 0;
};

extern "C" CompilerModule* loadModule();

extern "C" void unloadModule(CompilerModule*);
