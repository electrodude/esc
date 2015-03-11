#include <fstream>
#include <iostream>

#include <string>
#include <algorithm>

#include "settings.hpp"

#include "compiler.hpp"


%%{
	machine blocksplitter;

	prepush
	{
		if (top >= stacklen)
		{
			stack = (int*)realloc(stack, (stacklen*=2)*sizeof(int));
			//std::cout << "#expand lexer stack to " << stacklen << " entries\n";
		}
	}

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

#	action finalblock
#	{
#		if (currBlock != NULL)
#		{
#			currBlock->newBlock(std::string(blockstart-1, blockend-1));
#		}
#		else
#		{
#			std::cout << "warning: dropping unknown block: " << std::string(blockstart, p) << '\n';
#		}
#	}


	blockstart = alpha+ >idblock space %beginblock;

	include comment "comment.rl";	

#	blockline = ((any - '\n' - "'" - '{') | comment)*;
	blockline = ((^'\n') | comment)*;

	main := (blockstart? >endblock blockline '\n')*;

# failed attempt to handle trailing newlines - see Compiler:compileFile(Path) for where it hackishly adds the trailing newline
#	main := (blockstart? >endblock blockline '\n')* blockstart? >endblock blockline '\n'?;




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
	std::string::iterator eof = pe;

	int cs;

	int stacklen = 1;
	int* stack = (int*)malloc(stacklen*sizeof(int));
	int top = 0;
	
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

