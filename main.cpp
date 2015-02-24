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

	if (argv == 0)
	{
		std::cout << "Usage: blah blah blah\n";
	}

	//std::string path(argv[1]);
	std::string path("../../solarcar/modbusmaster.spin");

	c.compileFile(path);
}
