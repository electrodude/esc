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
		n_read = fread(&s[sused], 1, slen - sused, f);

		//printf("read %d, used %d, total %d\n", n_read, sused, slen);

		sused += n_read;

		// If changed in the future, make sure there's still room for
		//  the extra newline and null terminator at the end.
		if (slen < sused - 256)
		{
			s = (char*)realloc(s, slen *= 2);
		}
	} while (n_read);

	if (!feof(f))
	{
		printf("ferror: %d\n", ferror(f));
		exit(1);
	}

	fclose(f);

#if COMPILER_DEBUG >= 3
	printf("file length: %ld\n", sused);
#endif

	s[sused-1] = '\n'; // append final newline (arguably a hack)
	s[sused] = '\0';   // there is room for this extra null terminator, see
	                   //  fread loop

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
