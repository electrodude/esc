#include <map>
#include <string>
#include <iostream>

#include <cstdlib>

#include "registry.hpp"
#include "loader.hpp"

#include "compiler.hpp"

#include "main.hpp"

int main(int argc, char** argv)
{
	CompilerModule* mod = ModuleLoader::load("./modules/spinc.so");

	Compiler c;

	if (argc <= 1)
	{
		std::cout << "Usage: blah blah blah\n";

		exit(0);
	}

	std::string path(argv[1]);

	c.compileFile(path);
}
