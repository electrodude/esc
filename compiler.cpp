#include <cstdlib>
#include <cstdio>

#include <vector>

#include "parserlib.hpp"

#include "parser.hpp"

#include "compiler.hpp"

#define COMPILER_DEBUG 2

#define COMPILER_DUMP_AST 1

void compile_file(char* path)
{
#if COMPILER_DEBUG
	printf("compile %s\n", path);
#endif

	// load file
	size_t slen = 65536;
	size_t sused = 0;
	char* s = (char*)malloc(slen);
	FILE* f = fopen(path, "r");
	size_t n_read = 0;
	do
	{
		n_read = fread(&s[sused], slen - sused, 1, f);

		//printf("read %d, used %d, total %d\n", n_read, sused, slen);

		sused += n_read;

		if (slen < sused - 256)
		{
			s = (char*)realloc(s, slen *= 2);
		}
	} while (n_read);
	fclose(f);

	// parser
#if COMPILER_DEBUG >= 2
	printf("parse\n");
#endif
	std::vector<Block*>* blocks = NULL;
	try
	{
		blocks = parser(s);
	}
	catch (CompilerError* err)
	{
		err->print();
	}
	catch (char const* err)
	{
		printf("Error: %s\n", err);
	}

	if (blocks == NULL)
	{
		return;
	}
#if COMPILER_DEBUG >= 2
	printf("%ld blocks\n", blocks->size());
#endif

	free(s);

#if COMPILER_DUMP_AST
	printf("AST:\n");

	for (std::vector<Block*>::iterator it = blocks->begin(); it != blocks->end(); ++it)
	{
		Block* currblock = *it;
		printf("\nblock \"%s\"", currblock->def->name);
		if (currblock->haserrors)
		{
			printf(" - has errors");
		}
		printf("\n");
		std::vector<Line*>& lines = currblock->lines;
		for (std::vector<Line*>::iterator it2 = lines.begin(); it2 != lines.end(); ++it2)
		{
			Line* l = *it2;
			printf("%d ", l->indentdepth);
			l->operand->print();
			printf("\n");

		}
	}
#endif

	//classify_symbols(blocks);
	
	// macro expansion
	// TODO
	


}
