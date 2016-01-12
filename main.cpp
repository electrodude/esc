#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <strings.h>

#include <getopt.h>

#include "parserlib.hpp"

#include "parser.hpp"

#include "compiler.hpp"

#define OPTDEBUG 0

/*
static inline char* strdup(const char* s)
{
	char* d = malloc(strlen(s) + 1);
	if (d == NULL) return NULL;

	strcpy(d, s);

	return d;
}
*/

static char* outputfile;

static void usage(void)
{
	printf(
"esc %s - the Extensible Spin Compiler\n"
"Copyright 2015 Albert Emanuel Milani\n"
"Usage: esc\n"
"         [ -h ]                  display this help\n"
"         [ -t<width> ]           set tab width in spaces\n"
"         [ -o<path> ]            output file path\n"
"         [ -x<language> ]        compilation language\n"
"         [ -m<processor> ]       target processor\n"
"         [ -I<path> ]            add search path\n"
"         [ -L<path> ]            add search path\n"
"         [ -O<flags> ]           enable optimizer\n"
/*
#if defined(unix) || defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
"         [ -g<extension.so> ]    load extension\n"
#else
# if defined(_WIN32) || defined(_WIN64)
"         [ -g<extension.dll> ]   load extension\n"
# else
"         [ -g<extension> ]       load extension\n"
# endif
#endif
*/
"           <file.spin>           input file\n"
,
	"0.0.1");
	exit(1);
}

// main

Grammar* register_spin(void);

int main(int argc, char** argv)
{
	std::vector<char*> search_dirs;
	search_dirs.push_back(".");

	char* outputfile = NULL;
	char* langname = NULL;
	char* procname = NULL;

	while (1)
	{
		int c = getopt(argc, argv, "ht:o:x:m:I:L:g:O::");

		if (c == -1)
		{
			break;
		}

		switch (c)
		{
			case 't':
			{
				tabwidth = atoi(optarg);
#if OPTDEBUG
				printf("tab width: %d\n", tabwidth);
#endif
				break;
			}

			case 'o':
			{
				if (outputfile != NULL && strcmp(outputfile, optarg))
				{
					puts("Error: conflicting output file arguments!");
					usage();
				}
				outputfile = strdup(optarg);
#if OPTDEBUG
				printf("output file: %s\n", outputfile);
#endif
				break;
			}

			case 'm':
			{
				if (procname != NULL && strcmp(procname, optarg))
				{
					puts("Error: conflicting processor specifications!");
					usage();
				}
				procname = strdup(optarg);
#if OPTDEBUG
				printf("processor: %s\n", procname);
#endif
				break;
			}

			case 'x':
			{
				if (langname != NULL && strcmp(langname, optarg))
				{
					puts("Error: conflicting language specifications!");
					usage();
				}
				langname = strdup(optarg);
#if OPTDEBUG
				printf("language: %s\n", langname);
#endif
				break;
			}

			case 'I':
			case 'L':
			{
				char* path = strdup(optarg);
#if OPTDEBUG
				printf("search dir: %s\n", path);
#endif
				search_dirs.push_back(path);
				break;
			}

			case 'g':
			{
#if OPTDEBUG
				printf("grammar: %s\n", optarg);
#endif
				printf("NYI: -g<extension>\n");
				break;
			}

			case 'O':
			{
#if OPTDEBUG
				printf("optimize: %s\n", optarg);
#endif
				break;
			}

			//case 'h':
			default:
			{
				usage();
				//break;
			}
		}

	}

	if (langname == NULL)
	{
		langname = "spin";
	}

	if (procname == NULL)
	{
		procname = "p8x32a";
	}


	parser_init();

	if (!strcasecmp(langname, "spin"))
	{
		Grammar::reset(register_spin());
	}
	else
	{
		printf("Unknown language: %s\n", langname);
		exit(1);
	}

	if (optind >= argc)
	{
		puts("Error: missing input file");
		usage();
	}

	char* inputfile = argv[optind++];


	if (optind < argc)
	{
		printf("Error: extraneous arguments:");
		while (optind < argc)
		{
			printf(" %s", argv[optind++]);
		}
		puts("");
		usage();
	}

	if (outputfile == NULL)
	{
		size_t inputfile_len = strlen(inputfile);
		outputfile = (char*)malloc(inputfile_len + 7 + 1);
		strcpy(outputfile, inputfile);

		strncpy(&outputfile[inputfile_len], ".binary", 7);
	}


	compile_file(inputfile);
}
