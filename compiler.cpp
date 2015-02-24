#include <fstream>
#include <iostream>

#include <string>
#include <algorithm>

#include "compiler.hpp"

SpinObject::SpinObject(std::string code)
{
	blocktypes["con"] = registry.getBlock("con");
	blocktypes["obj"] = registry.getBlock("obj");
	blocktypes["var"] = registry.getBlock("var");
	blocktypes["pub"] = registry.getBlock("pub");
	blocktypes["pri"] = registry.getBlock("pri");
	blocktypes["dat"] = registry.getBlock("dat");
	blocktypes["pub"] = registry.getBlock("pub");

	BlockFactory* currBlock = blocktypes["con"];

	std::string::iterator start = code.begin();
	std::string::iterator end = start;
	while (end != code.end())
	{
		//std::cout << *end;
		if (*end == '\n' && *(end+1) != '\n')
		{
			std::string::iterator end2 = end;

			end2++;

			while ((*end2 != ' ' && *end2 != '\n') && end2 != code.end())
			{
				//std::cout << "block char: " << *end2 << '\n';
				end2++;
			}

			std::string blockname = std::string(end+1, end2);

			std::transform(blockname.begin(), blockname.end(), blockname.begin(), ::tolower);

			//std::cout << "potential block: " << blockname << '\n';

			BlockFactory* nextBlock = blocktypes[blockname];

			if (nextBlock != NULL)
			{
				if (currBlock != NULL) // in case CON isn't defined
				{
					currBlock->newBlock(std::string(start, end-1));
				}
				else
				{
					std::cout << "Block: " << std::string(start, end-1) << '\n';
				}


				end = end2;
				start = end+1;

				currBlock = nextBlock;
				std::cout << "Next block type: " << blockname << '\n';
			}
		}

		end++;
	}

	currBlock->newBlock(std::string(start+1, end-1));	
}

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

	SpinObject* obj = new SpinObject(s.str());

	objects[path] = obj;

	return obj;
}
