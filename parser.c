#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#include "parser.h"
#include "parserlib.h"

#define PARSERDEBUG 0



static stack* vstack;

static stack* ostack;

static char* tok2stra(char* base, char* start, char* end)
{
	size_t baselen = strlen(base);

	char* s = malloc(baselen + end - start + 1);

	strncpy(s, base, baselen);

	strncpy(&s[baselen], start, end - start);

	s[baselen + end-start] = 0;

	return s;
}

static char* tok2str(char* start, char* end)
{
	char* s = malloc(end - start + 1);

	strncpy(s, start, end - start);

	s[end-start] = 0;

	return s;
}

static void printstacks(void)
{
	printf("vstack: ");
	for (int i=0; i < vstack->top; i++)
	{
		operand* val = vstack->base[i];

		operand_print(val);

		printf("\t");
	}
	printf("\n");
	printf("ostack: ");
	for (int i=0; i < ostack->top; i++)
	{
		operator* op = ostack->base[i];

		printf("\"%s\" ", op->name);
	}
	printf("\n");
}

static void fold(operator* nextop)
{
#if PARSERDEBUG
	if (nextop != NULL)
	{
		printf("fold: nextop = \"%s\" (%d%d%d)\n", nextop->name, nextop->leftarg, nextop->rightarg, nextop->bracket);
	}
	else
	{
		printf("fold: nextop = NULL\n");
	}
#endif

	if (nextop != NULL && nextop->bracket && !nextop->leftarg)
	{
		// don't fold if left bracket

#if PARERDEBUG
		printf("fold: left bracket, no fold\n");
#endif

		stack_push(ostack, nextop);

#if PARSERDEBUG >= 2
		printstacks();
#endif

		return;
	}

	operator* topop;

	int bracket = nextop != NULL ? nextop->bracket : 0;

	while (topop = stack_peek(ostack), topop != NULL && (nextop == NULL || nextop->precedence >= topop->precedence) && ((nextop != NULL && topop->bracket == nextop->bracket) || !topop->bracket))
	{
		operator* op = stack_pop(ostack);
#if PARSERDEBUG
		printf("fold: op \"%s\" (%d%d%d)\n", op->name, op->leftarg, op->rightarg, op->bracket);
#endif

		if (op != NULL && op->bracket && op->rightarg && !op->leftarg)
		{
#if PARERDEBUG
			printf("fold bracket break\n");
#endif
			bracket = 0;
			break;
		}

		operand* rhs = NULL;
		if (op->rightarg)
		{
			rhs = stack_pop(vstack);
			if (rhs == NULL)
			{
				printf("Error: rhs of '%s' (%d%d%d) == NULL!\n", op->name, op->leftarg, op->rightarg, op->bracket);
				exit(1);
			}
		}

		operand* lhs = NULL;
		if (op->leftarg)
		{
			lhs = stack_pop(vstack);
			if (lhs == NULL)
			{
				printf("Error: lhs of '%s' (%d%d%d) == NULL!\n", op->name, op->leftarg, op->rightarg, op->bracket);
				exit(1);
			}
		}

		stack_push(vstack, binop_new(op, lhs, rhs));

		if (op != NULL && op->bracket && op->rightarg)
		{
#if PARERDEBUG
			printf("fold function break\n");
#endif
			bracket = 0;
			break;
		}
	}

	operator* tos = stack_peek(ostack);

	if (nextop == NULL && tos != NULL && (tos->bracket && tos->rightarg))
	{
		printf("Error: unmatched left parentheses\n");
		exit(1);
	}

	if (nextop != NULL && bracket && nextop->leftarg && !nextop->rightarg)
	{
		if (tos == NULL)
		{
			printf("Error: unmatched right parentheses! nextop = \"%s\" (%d%d%d), tos = ", nextop->name, nextop->leftarg, nextop->rightarg, nextop->bracket);
			if (tos != NULL)
			{
				printf("\"%s\" (%d%d%d)\n", tos->name, tos->leftarg, tos->rightarg, tos->bracket);
			}
			else
			{
				printf("NULL\n");
			}
			exit(1);
		}
	}

	if (nextop != NULL && !(nextop->bracket && !nextop->rightarg))
	{
		stack_push(ostack, nextop);
	}

#if PARSERDEBUG >= 2
	printstacks();
#endif
}

static unsigned int lineno = 0;

static void eatblockcomment(char** pp)
{
#if PARSERDEBUG >= 2
	printf("block comment on line %d: ", lineno);
#endif

	int level = 0;

	char* p = *pp;

	while (level > 0 || *p == '{')
	{
		switch(*p++)
		{
			case 0   : goto end;
			case '{' : level++; break;
			case '}' : --level; break;
			case '\n': lineno++;
		}
	}

end:
#if PARSERDEBUG >= 2
	printf("ate %ld chars\n", p - *pp);
#endif

	*pp = p;
}

static char islabelchar[256];

stack* parser(char* p)
{
	char* ts = p;

	optabentry* currop;

	int pc = 5;

	lineno = 1;
	char* linestart = p;

#if PARSERDEBUG >= 2
	printf("Line %d\n", lineno);
#endif

	char* lastgloballabel = "!begin";


	const char* currblock = symbol_get("con")->name;

	const char* datblockname = symbol_get("dat")->name;


	stack* blocks = stack_new();

	stack* lines = stack_new();

	stack_push(blocks, lines);

	line* currline = malloc(sizeof(line));

	goto line;

newline:
#if PARSERDEBUG
	printf("\\n\n");
#endif

	lineno++;
	linestart = p;

#if PARSERDEBUG >= 2
	printf("Line %d\n", lineno);
#endif

	if (stack_peek(vstack) != NULL)
	{
		currline->operand = stack_pop(vstack);

#if PARSERDEBUG >= 2
		operand_print(currline->operand);
		printf("\n");
#endif

		stack_push(lines, currline);

		currline = malloc(sizeof(line));
	}

	goto line;

line:
#if PARSERDEBUG >= 3
	//printf("line\n", *p);
	printf("line: '%c'\n", *p);
#endif
	switch (*p)
	{
		case 0   : goto end;
		case '\'': p++; goto comment;
		case '{' : eatblockcomment(&p); goto line;
		case '\n':
		case '\r': lineno++; linestart=p+1;
		case ' ' :
		case '\t': p++; goto line;
	}

	goto expr_entry;

comment:
#if PARSERDEBUG >= 3
	printf("comment: '%c'\n", *p);
#endif
	switch (*p)
	{
		case 0   : goto end;
		case '\n':
		case '\r': p++; goto newline;
	}

	p++; goto comment;

expr_entry:
#if PARSERDEBUG
	//printf("opcode\n");
#endif
	{
		operator* stray = stack_peek(ostack);
		if (stray != NULL)
		{
			printf("Error at %d:%ld: stray operator on operator stack: %s\n", lineno, p-linestart, stray->name);
			goto error;
		}
	}
	{
		operand* stray = stack_peek(vstack);
		if (stray != NULL)
		{
			printf("Error at %d:%ld: stray value on value stack: ", lineno, p-linestart);
			operand_print(stray);
			printf("\n");
			goto error;
		}
	}

expr:
#if PARSERDEBUG >= 2
	printf("expr: '%c'\n", *p);
#endif
	currop = preoperators;

	switch (*p)
	{
		case 0   : goto end;
		case ' ' :
		case '\t': p++; goto expr;
		case '{' : eatblockcomment(&p); goto expr;
		case '"' : goto string;
	}

	currop = currop[tolower(*p)].next;

	if (*p >= '0' && *p <= '9') goto decnum;
	if (islabelchar[*p] >= 3) goto ident;

	if ((*p == '\n' || *p == '\r') && stack_peek(vstack) == NULL)
	{
		goto newline;
	}


	if (currop != NULL)
	{
		p++;

		while (*p && currop[tolower(*p)].next != NULL)
		{
#if PARSERDEBUG >= 3
			printf("prefix operator char: '%c'\n", *p);
#endif
			currop = currop[tolower(*p)].next;

			p++;
		}

		operator* op = currop[0].op;

		if (op == NULL)
		{
			goto error;
		}


#if PARSERDEBUG >= 2
		printf("prefix op operator: \"%s\"\n", op->name);
#endif
		fold(op);

		goto expr;
	}

	goto error;

ident:
	ts = p;

	while (*p && (islabelchar[*p] >= 2 || currop != NULL))
	{
#if PARSERDEBUG >= 3
		printf("ident: '%c'\n", *p);
#endif

		if (currop != NULL)
		{
			currop = currop[tolower(*p)].next;
		}

		p++;
	}

	if (currop != NULL && currop[0].op != NULL)
	{
		operator* op = currop[0].op;

#if PARSERDEBUG >= 2
		printf("prefix ident operator: \"%s\"\n", op->name);
#endif
		fold(op);

		goto expr;
	}

	char* base = "";

	if (*ts == ':')
	{
		base = lastgloballabel;
	}

	char* s = tok2stra(base, ts, p);

#if PARSERDEBUG
	printf("ident: \"%s\"\n", s);
#endif
	operand* val = ident_new(&s);

	symbol* sym = val->val.ident;

	int push = 1;

	if (stack_peek(vstack) == NULL)
	{
		if (currblock == datblockname)
		{
			// DAT blocks have local labels, which we need to scope now
			if (sym->type == SYM_UNKNOWN)
			{
				if (*s != ':' && *s != '.')
				{
					lastgloballabel = s;
				}

				sym->type = SYM_LABEL;
			}
			else if (sym->type == SYM_LABEL)
			{
				printf("Duplicate symbol \"%s\" at %d:%ld\n", s, lineno, p-linestart);
				//goto error;
			}
		}

		if (sym->type == SYM_BLOCK)
		{
#if PARSERDEBUG
			printf("\nBlock \"%s\"\n", s);
#endif

			lines = stack_new();

			stack_push(blocks, lines);

			currblock = s;

			symbols = sym->data.block->symbols;
			operators = sym->data.block->operators;
			preoperators = sym->data.block->preoperators;

			push = 0;
		}
	}
	else
	{
		if (sym->type == SYM_BLOCK)
		{
			printf("Warning: block name not first token on line!\n");

			goto error;
		}
	}

	if (push)
	{
		stack_push(vstack, val);

		goto operator;
	}
	else
	{
		goto line;
	}


string:
	ts = p;

string_mid:
#if PARSERDEBUG >= 2
	printf("string: '%c'\n", *p);
#endif

	switch (*p)
	{
		case 0   : goto error;
		case '"' : break;
		default  : p++; goto string_mid;
	}

	stack_push(vstack, string_new(tok2str(ts, p)));
	p++;


	goto operator;



	int nbase;
decnum:
	nbase = 10;
	goto num;

hexnum:
	nbase = 16;
	goto num;

binnum:
	if (*p == '%')
	{
		nbase = 4;
	}
	else
	{
		nbase = 2;
	}
num:
	ts = p;

	plong n = 0;
num_l:
#if PARSERDEBUG >= 3
	printf("num: '%c'\n", *p);
#endif
	switch (*p)
	{
		case 0   : goto error;
	}

	plong digit = 0;
	if (*p >= '0' && *p <= '9')
	{
		digit = *p - '0';
	}
	else if (*p >= 'A' && *p <= 'Z')
	{
		digit = *p + 10 - 'A';
	}
	else if (*p >= 'a' && *p <= 'z')
	{
		digit = *p + 10 - 'z';
	}
	else if (*p == '_')
	{
		// eat underscores
	}
	else if (*p == '.')
	{
		// TODO: floats

		printf("Can't do floats yet!\n");
	}
	else
	{
		goto num_done;
	}

	n = n * nbase + digit;
	p++;

	goto num_l;

num_done:

	stack_push(vstack, int_new(n));

	goto operator;

here_or_hex:
#if PARSERDEBUG
	printf("here or hex: '%c'\n", *p);
#endif
	p++;
	if (isxdigit(*p)) goto hexnum;

	stack_push(vstack, ref_new(currline));
	goto operator;

operator:
	currop = operators;

#if PARSERDEBUG >= 3
	printf("operator char: '%c'\n", *p);
#endif
	switch (*p)
	{
		case 0   : goto end;
		case ' ' :
		case '\t': p++; goto operator;
		case '\n':
		case '\r': fold(NULL); p++; goto newline;
		case '\'': fold(NULL); p++; goto comment;
		case '{' : eatblockcomment(&p); goto operator;
	}

	ts = p;

operator_mid:

	while (*p && currop[tolower(*p)].next != NULL)
	{
#if PARSERDEBUG >= 3
		printf("operator char: '%c'\n", *p);
#endif
		currop = currop[tolower(*p)].next;

		p++;
	}

	operator* op = currop[0].op;

	if (op == NULL)
	{
		fold(operators[0].op);
		p = ts; // backtracking! Blech!
		goto ident;
	}


#if PARSERDEBUG >= 2
	printf("operator: \"%s\"\n", op->name);
#endif

	if (!op->leftarg)
	{
		fold(operators[0].op);
	}

	fold(op);

	if (op->rightarg)
	{
		goto expr;
	}
	else
	{
		goto operator;
	}


error:
	printf("Parse error at %d:%ld: '%c' (%02X)\n", lineno, p-linestart, *p, *p);

#if 0
	while (*p != 0 && *p != '\n' && *p != '\r') p++;

	while (vstack->top > 0)
	{
		operand* o = stack_pop(vstack);
		printf("dropping operand ");
		operand_print(o);
		printf("\n");
	}

	while (ostack->top > 0)
	{
		operator* o = stack_pop(ostack);
		if (o != NULL)
		{
			printf("dropping operator '%s'\n", o->name);
		}
		else
		{
			printf("dropping operator NULL\n");
		}
	}

	printf("continuing on next line\n\n");

	goto line;
#endif


end:

	return blocks;
}


static inline void islabelchar_r(size_t start, size_t end, char c)
{
	while (start <= end)
	{
		islabelchar[start++] = c;
	}
}

void parser_init(void)
{
	islabelchar_r(0,   127,   0);
	islabelchar_r('A', 'Z',   3);
	islabelchar_r('a', 'z',   3);
	islabelchar       ['_'] = 3;
	islabelchar       [':'] = 3;
	islabelchar       ['.'] = 3;
	islabelchar_r('0', '9',   2);
	islabelchar_r(128, 255,   2); // UTF-8 support

	vstack = stack_new();
	ostack = stack_new();

	symbols = malloc(sizeof(symtabentry)*256);
	operators = malloc(sizeof(optabentry)*256);
	preoperators = malloc(sizeof(optabentry)*256);
}
