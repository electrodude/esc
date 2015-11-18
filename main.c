#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "parserlib.h"

// main

int main(int argc, char** argv)
{
	parser_init();

#include "opcodes.c"

// load file
	size_t slen = 65536;
	size_t sused = 0;
	char* s = malloc(slen);
	FILE* f = fopen(argv[1], "r");
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

	stack/*of stack of line*/* blocks = parser(s);

	printf("\nAST:\n");

	for (int i=0; i < blocks->top; i++)
	{
		printf("\nblock\n");
		stack/* of line */* lines = blocks->base[i];
		for (int j=0; j < lines->top; j++)
		{
			line* l = lines->base[j];
			printf("%d ", l->indentdepth);
			operand_print(l->operand);
			printf("\n");

		}
	}
}
