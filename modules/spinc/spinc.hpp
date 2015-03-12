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

	virtual Block* newBlock(std::string code) const;

	virtual std::string getName() const;
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

	virtual Block* newBlock(std::string code) const;

	virtual std::string getName() const;
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

	virtual Block* newBlock(std::string code) const;

	virtual std::string getName() const;
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

	virtual Block* newBlock(std::string code) const;

	virtual std::string getName() const;
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

	virtual Block* newBlock(std::string code) const;

	virtual std::string getName() const;
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

	virtual Block* newBlock(std::string code) const;

	virtual std::string getName() const;
};



class SpinCompiler : public CompilerModule
{
public:
	SpinCompiler(CompilerRegistry& registry);

	virtual ~SpinCompiler()	{}
};

