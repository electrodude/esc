#pragma once

#include "module.hpp"

class CONBlock : public Block
{
public:
	CONBlock(std::string code);
	
	virtual void foo() {}
};

class CONBlockFactory : public BlockFactory
{
public:
	CONBlockFactory();

	virtual Block* newBlock(std::string code);

	virtual std::string name();
};

class OBJBlock : public Block
{
public:
	OBJBlock(std::string code);

	virtual void foo() {}
};

class OBJBlockFactory : public BlockFactory
{
public:
	OBJBlockFactory();

	virtual Block* newBlock(std::string code);

	virtual std::string name();
};

class VARBlock : public Block
{
public:
	VARBlock(std::string code);

	virtual void foo() {}
};

class VARBlockFactory : public BlockFactory
{
public:
	VARBlockFactory();

	virtual Block* newBlock(std::string code);

	virtual std::string name();
};


class PUBBlock : public Block
{
public:
	PUBBlock(std::string code);

	virtual void foo() {}
};

class PUBBlockFactory : public BlockFactory
{
public:
	PUBBlockFactory();

	virtual Block* newBlock(std::string code);

	virtual std::string name();
};

class PRIBlock : public Block
{
public:
	PRIBlock(std::string code);

	virtual void foo() {}
};

class PRIBlockFactory : public BlockFactory
{
public:
	PRIBlockFactory();

	virtual Block* newBlock(std::string code);

	virtual std::string name();
};

class DATBlock : public Block
{
public:
	DATBlock(std::string code);

	virtual void foo() {}
};

class DATBlockFactory : public BlockFactory
{
public:
	DATBlockFactory();

	virtual Block* newBlock(std::string code);

	virtual std::string name();
};



class SpinCompiler : public CompilerModule
{
public:
	SpinCompiler(CompilerRegistry& registry);

	virtual ~SpinCompiler()	{}
};

