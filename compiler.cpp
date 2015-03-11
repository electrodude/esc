#include <fstream>
#include <iostream>

#include <string>
#include <algorithm>

#include "settings.hpp"

#include "compiler.hpp"


Compiler::Compiler()
{
	
}

SpinObject* Compiler::compileFile(Path path)
{
	if (SpinObject* obj = objects[path])
	{
		return obj;
	}
	
	std::ifstream f(path);

	std::stringstream s;
	s << f.rdbuf();
	s << '\n'; // hack: instead of properly handling no trailing newline, just add a trailing newline

	SpinObject* obj = new SpinObject(s.str());

	objects[path] = obj;

	return obj;
}
