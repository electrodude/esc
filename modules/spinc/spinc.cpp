#include <iostream>
#include <string>

#include "spinc.hpp"

class CONBlock : public Block
{
public:
	CONBlock(std::string code)
	{
		std::cout << "CON block: " << code << '\n';
	}

	virtual void foo() {};
};

class CONBlockFactory : public BlockFactory
{
public:
	CONBlockFactory()
	{
		printf("CON block registered!\n");
	}

	virtual Block* newBlock(std::string code)
	{
		return new CONBlock(code);
	}

	virtual std::string name()
	{
		return "con";
	}
};

class OBJBlock : public Block
{
public:
	OBJBlock(std::string code)
	{
		std::cout << "OBJ block: " << code << '\n';
	}

	virtual void foo() {};
};

class OBJBlockFactory : public BlockFactory
{
public:
	OBJBlockFactory()
	{
		printf("OBJ block registered!\n");
	}

	virtual Block* newBlock(std::string code)
	{
		return new OBJBlock(code);
	}

	virtual std::string name()
	{
		return "obj";
	}
};

class VARBlock : public Block
{
public:
	VARBlock(std::string code)
	{
		std::cout << "VAR block: " << code << '\n';
	}

	virtual void foo() {};
};

class VARBlockFactory : public BlockFactory
{
public:
	VARBlockFactory()
	{
		printf("VAR block registered!\n");
	}

	virtual Block* newBlock(std::string code)
	{
		return new VARBlock(code);
	}

	virtual std::string name()
	{
		return "var";
	}
};


class PUBBlock : public Block
{
public:
	PUBBlock(std::string code)
	{
		std::cout << "PUB block: " << code << '\n';
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

	virtual std::string name()
	{
		return "pub";
	}
};

class PRIBlock : public Block
{
public:
	PRIBlock(std::string code)
	{
		std::cout << "PRI block: " << code << '\n';
	}

	virtual void foo() {};
};

class PRIBlockFactory : public BlockFactory
{
public:
	PRIBlockFactory()
	{
		printf("PRI block registered!\n");
	}

	virtual Block* newBlock(std::string code)
	{
		return new PRIBlock(code);
	}

	virtual std::string name()
	{
		return "pri";
	}
};

class DATBlock : public Block
{
public:
	DATBlock(std::string code)
	{
		std::cout << "DAT block: " << code << '\n';
	}

	virtual void foo() {};
};

class DATBlockFactory : public BlockFactory
{
public:
	DATBlockFactory()
	{
		printf("DAT block registered!\n");
	}

	virtual Block* newBlock(std::string code)
	{
		return new DATBlock(code);
	}

	virtual std::string name()
	{
		return "dat";
	}
};



class SpinCompiler : public CompilerModule
{
public:
	SpinCompiler(CompilerRegistry& registry)
	{
		printf("Spin compiler module loading!\n");
		registry(new CONBlockFactory());
		registry(new OBJBlockFactory());
		registry(new VARBlockFactory());
		registry(new PUBBlockFactory());
		registry(new PRIBlockFactory());
		registry(new DATBlockFactory());

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
