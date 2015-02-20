#include "spinc.hpp"

class PUBBlock : public Block
{
public:
	PUBBlock(const char* code)
	{

	}

	void foo() {};
};

class SpinCompiler : public CompilerModule
{
public:
	SpinCompiler()
	{

	}

	~SpinCompiler()
	{

	}

	virtual Block* newBlock(int id, const char* code)
	{
		switch (id)
		{
			case 0: return new PUBBlock(code);
		}
		return NULL;
	}
};

extern "C" CompilerModule* loadModule()
{
	return new SpinCompiler();
}

extern "C" void unloadModule(CompilerModule* mod)
{
	delete mod;
}
