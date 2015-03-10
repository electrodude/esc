#include <fstream>
#include <iostream>

#include <string>
#include <algorithm>

#include "settings.hpp"

#include "compiler.hpp"


%%{
	machine blocksplitter;

	action endblock
	{
		blockend = p;
	}

	action idblock
	{
		blockidstart = p;
	}

	action beginblock
	{
		std::string blockname = std::string(blockidstart, p-1);

		std::transform(blockname.begin(), blockname.end(), blockname.begin(), ::tolower);

		//std::cout << "potential block: " << blockname << '\n';

		BlockFactory* nextBlock = blocktypes[blockname];

		if (nextBlock != NULL)
		{
			if (currBlock != NULL)
			{
				
				currBlock->newBlock(std::string(blockstart-1, blockend));
			}
			else
			{
				std::cout << "warning: dropping unknown block: " << std::string(blockstart, p) << '\n';
			}

			currBlock = nextBlock;

			blockstart = p;
		}
	}

	action finalblock
	{
		if (currBlock != NULL)
		{
			currBlock->newBlock(std::string(blockstart-1, blockend-1));
		}
		else
		{
			std::cout << "warning: dropping unknown block: " << std::string(blockstart, p) << '\n';
		}
	}


	blockstart = alpha+ >idblock ^alpha %beginblock;

	blockline = (^'\n')*;


	main := (blockstart? >endblock blockline '\n')* blockstart? >endblock blockline 0;


}%%


%% write data;

SpinObject::SpinObject(std::string code)
{
	blocktypes = registry.getDefaultBlocks();


	BlockFactory* currBlock = blocktypes["con"];

	std::string::iterator p = code.begin();
	std::string::iterator blockend = p;
	std::string::iterator blockidstart = p;
	std::string::iterator blockstart = p;
	std::string::iterator pe = code.end();

	int cs;
	
	int commentdepth = 0;

	int maxCommentLength = settings.maxCommentLength;


	%% write init;
	%% write exec;

	if (currBlock != NULL)
	{
		currBlock->newBlock(std::string(blockstart-1, blockend-1));
	}
	else
	{
		std::cout << "warning: dropping unknown block: " << std::string(blockstart, p) << '\n';
	}


}

