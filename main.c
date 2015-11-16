#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "parserlib.h"

#define LIBDEBUG 0

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
	stack/*of line*/* lines = parser(s);
}
