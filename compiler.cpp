#include <cstdlib>
#include <cstdio>

#include <vector>

#include "parserlib.hpp"

#include "parser.hpp"

#include "compiler.hpp"

#define COMPILER_DEBUG 0

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


		sused += n_read;

#if COMPILER_DEBUG >= 3
		printf("read %ld, used %ld, total %ld\n", n_read, sused, slen);
#endif

		// If changed in the future, make sure there's still room for
		//  the extra newline and null terminator at the end.
		if (sused > slen - 256)
		{
			s = (char*)realloc(s, slen *= 2);
#if COMPILER_DEBUG >= 3
			printf("realloc to %ld\n", slen);
#endif
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
	Line* lines = NULL;
	try
	{
		lines = parser(s);
	}
	catch (CompilerError* err)
	{
		err->print();
	}
	catch (char const* err)
	{
		printf("Error: %s\n", err);
	}

	if (lines == NULL)
	{
		return;
	}
#if COMPILER_DEBUG >= 2
	printf("%ld blocks\n", lines->children.size());
#endif

	free(s);

#if COMPILER_DUMP_AST
	printf("AST:\n");

	lines->print();
#endif

	//classify_symbols(blocks);

	// macro expansion
	// TODO



}
