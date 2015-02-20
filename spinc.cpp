#include "spinc.hpp"

class PUBBlock : public Block
{
public:
	PUBBlock(std::string)
	{

	}

	virtual void foo() {};
};

class PUBBlockFactory : public BlockFactory
{
public:
	PUBBlockFactory()
	{
		printf("PUB block registered!\n");
	}

	virtual Block* newBlock(std::string code)
	{
		return new PUBBlock(code);
	}
};

class SpinCompiler : public CompilerModule
{
public:
	SpinCompiler(CompilerRegistry& registry) : CompilerModule(registry)
	{
		printf("Spin compiler module loading!\n");
		registry(new PUBBlockFactory());
	}

	~SpinCompiler()
	{

	}
};

extern "C" CompilerModule* loadModule(CompilerRegistry& registry)
{
	return new SpinCompiler(registry);
}

extern "C" void unloadModule(CompilerModule* mod)
{
	delete mod;
}
