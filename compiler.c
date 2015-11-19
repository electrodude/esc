#include "stack.h"
#include <stdlib.h>

#include <stdio.h>

#include "parserlib.h"

#include "parser.h"

#include "compiler.h"

#define COMPILER_DEBUG 2

#define COMPILER_DUMP_AST 0

void compile_file(char* path)
{
#if COMPILER_DEBUG
	printf("compile %s\n", path);
#endif

	// load file
	size_t slen = 65536;
	size_t sused = 0;
	char* s = malloc(slen);
	FILE* f = fopen(path, "r");
	size_t n_read = 0;
	do
	{
		n_read = fread(&s[sused], slen - sused, 1, f);

		//printf("read %d, used %d, total %d\n", n_read, sused, slen);

		sused += n_read;

		if (slen < sused - 256)
		{
			s = realloc(s, slen *= 2);
		}
	} while (n_read);
	fclose(f);

	// parser
#if COMPILER_DEBUG >= 2
	printf("parse\n");
#endif
	stack/*of block*/ * blocks = parser(s);

	if (blocks == NULL)
	{
		return;
	}
#if COMPILER_DEBUG >= 2
	printf("%d blocks\n", blocks->top);
#endif

	free(s);

#if COMPILER_DUMP_AST
	printf("AST:\n");

	for (int i=0; i < blocks->top; i++)
	{
		block* currblock = blocks->base[i];
		printf("\nblock \"%s\"\n", currblock->def->name);
		stack/* of line */ * lines = currblock->lines;
		for (int j=0; j < lines->top; j++)
		{
			line* l = lines->base[j];
			printf("%d ", l->indentdepth);
			operand_print(l->operand);
			printf("\n");

		}
	}
#endif

	//classify_symbols(blocks);
	
	// macro expansion
	// TODO
	


}
