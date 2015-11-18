#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#include <math.h>

#include "parser.h"
#include "parserlib.h"

#define PARSERDEBUG 0


int tabwidth = 8;

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
#if PARSERDEBUG >= 3
	if (nextop != NULL)
	{
		printf("fold: nextop = \"%s\" (%d, %d)\n", nextop->name, nextop->leftarg, nextop->rightarg);
	}
	else
	{
		printf("fold: nextop = NULL\n");
	}
#endif

	if (nextop != NULL && nextop->leftarg == 0 && nextop->rightarg >= 2)
	{
		// don't fold if left bracket

#if PARSERDEBUG >= 3
		printf("fold: left bracket, no fold\n");
#endif

		stack_push(ostack, nextop);

#if PARSERDEBUG >= 3
		printstacks();
#endif

		return;
	}

	operator* topop;

	while (topop = stack_peek(ostack),
	        topop != NULL // stop if stack is empty
	        && (nextop == NULL // continue all the way if nextop == NULL
		    || (nextop->leftarg >= 2 || (topop->rightarg < 2 && nextop->precedence > topop->precedence)) // stop if right bracket or failed precedence test
		   )
	        //&& ((nextop != NULL && topop->rightarg == nextop->leftarg) || topop->rightarg == 0) // unless tos is left bracket, stop if mat
	      )
	{
		topop = stack_pop(ostack);
#if PARSERDEBUG >= 3
		printf("fold: op \"%s\" (%d, %d)\n", topop->name, topop->leftarg, topop->rightarg);
#endif

		if (topop != NULL && nextop != NULL && topop->leftarg == 0 && topop->rightarg >= 2 && nextop->leftarg >= 2 && nextop->rightarg == 0)
		{
#if PARSERDEBUG >= 3
			printf("fold bracket break\n");
#endif
			break;
		}

		operand* rhs = NULL;
		if (topop->rightarg)
		{
			rhs = stack_pop(vstack);
			if (rhs == NULL)
			{
				printf("Error: rhs of '%s' (%d, %d) == NULL!\n", topop->name, topop->leftarg, topop->rightarg);
				exit(1);
			}
		}

		operand* lhs = NULL;
		if (topop->leftarg)
		{
			lhs = stack_pop(vstack);
			if (lhs == NULL)
			{
				printf("Error: lhs of '%s' (%d, %d) == NULL!\n", topop->name, topop->leftarg, topop->rightarg);
				exit(1);
			}
		}

		stack_push(vstack, binop_new(topop, lhs, rhs));

		if (topop != NULL && nextop != NULL && nextop->leftarg >= 2 && topop->rightarg == nextop->leftarg)
		{
#if PARSERDEBUG >= 3
			printf("fold function break\n");
#endif
			break;
		}
	}

#if PARSERDEBUG >= 3
	printf("fold: done\n");
#endif

	topop = stack_peek(ostack);

	if (topop != NULL && nextop != NULL && topop->rightarg >= 2 && nextop->leftarg >= 2 && topop->rightarg != nextop->leftarg)
	{
		printf("Error: mismatched brackets\n");
		exit(1);
	}

	if (nextop == NULL && topop != NULL && topop->rightarg >= 2)
	{
		printf("Error: unmatched left bracket\n");
		exit(1);
	}

	if (topop == NULL && nextop && NULL && nextop->leftarg >= 2)
	{
		if (topop == NULL)
		{
			printf("Error: unmatched right bracket! nextop = ");
			if (nextop != NULL)
			{
				printf("\"%s\" (%d, %d)", nextop->name, nextop->leftarg, nextop->rightarg);
			}
			else
			{
				printf("NULL");
			}

			if (topop != NULL)
			{
				printf(", topop = \"%s\" (%d, %d)\n", topop->name, topop->leftarg, topop->rightarg);
			}
			else
			{
				printf(", topop = NULL\n");
			}
			exit(1);
		}
	}

	if (nextop != NULL && !(nextop->leftarg >= 2 && nextop->rightarg == 0))
	{
#if PARSERDEBUG >= 3
		printf("fold: push\n");
#endif
		stack_push(ostack, nextop);
	}

#if PARSERDEBUG >= 3
	printstacks();
#endif
}

static unsigned int lineno = 0;

static void eatdocblockcomment(char** pp)
{
	char* p = *pp;

	while (*p)
	{
		if (*p++ == '}')
		{
			if (*p == '}')
			{
				p++;
				*pp = p;
				return;
			}
		}
	}
}

static void eatblockcomment(char** pp)
{
#if PARSERDEBUG >= 3
	printf("block comment on line %d: ", lineno);
#endif

	int level = 0;

	char* p = *pp;

	while (*p && (level > 0 || *p == '{'))
	{
		switch(*p++)
		{
			case 0   : goto end;
			case '{' :
			{
				if (*p == '{')
				{
					eatdocblockcomment(&p);
				}
				else
				{
					level++;
				}
				break;
			}
			case '}' : --level; break;
			case '\n': lineno++;
		}
	}

end:
#if PARSERDEBUG >= 3
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

#if PARSERDEBUG >= 3
	printf("Line %d\n", lineno);
#endif

	char* lastgloballabel = "!begin";


	blockdef* currblock = symbol_get("con")->data.block;

	symtabentry* symbols = currblock->symbols;
	optabentry* operators = currblock->operators;
	optabentry* preoperators = currblock->preoperators;


	stack* blocks = stack_new();

	stack* lines = stack_new();

	stack_push(blocks, lines);

	line* currline = malloc(sizeof(line));
	line* prevline = NULL;

	unsigned int indent = 0;

	stack* indentstack = stack_new();

	goto line;

newline:
#if PARSERDEBUG >= 3
	printf("\\n\n");
#endif

	if (stack_peek(vstack) != NULL)
	{
		currline->operand = stack_pop(vstack);

		if (currblock->hasindent)
		{
			if (prevline != NULL && indent > prevline->indent)
			{
#if PARSERDEBUG
				printf("indent push: %d %d\n", prevline->indentdepth, prevline->indent);
#endif
				stack_push(indentstack, prevline);
			}

			line* indentparent;
			while (indentparent = stack_peek(indentstack),
				   indentparent != NULL && indentparent->indent >= indent)
			{
				line* oldindentparent = stack_pop(indentstack);
#if PARSERDEBUG
				printf("indent pop: %d %d\n", oldindentparent->indentdepth, oldindentparent->indent);
#endif
			}

			currline->indent = indent;
			currline->parent = indentparent;
			currline->indentdepth = indentparent != NULL ? indentparent->indentdepth+1 : 0;
#if PARSERDEBUG
			printf("%d %d ", currline->indentdepth, currline->indent);
			// no newline, continued at operand_print(currline->operand)
#endif
		}
		else
		{
			currline->indent = 0;
			currline->parent = NULL;
			currline->indentdepth = 0;
		}

#if PARSERDEBUG
		operand_print(currline->operand);
		printf("\n");
#endif

		stack_push(lines, currline);

		prevline = currline;
		currline = malloc(sizeof(line));
	}

	lineno++;
	linestart = p;

#if PARSERDEBUG >= 3
	printf("Line %d\n", lineno);
#endif

	indent = 0;

	goto line;

line:
#if PARSERDEBUG >= 4
	//printf("line\n", *p);
	printf("line: '%c'\n", *p);
#endif
	switch (*p)
	{
		case 0   : goto end;
		case '\'': p++; goto comment;
		case '{' : eatblockcomment(&p); goto line;
		case '\n':
		case '\r': indent = 0; lineno++; linestart=p+1; p++; goto line;
		case ' ' : indent++;         p++; goto line;
		case '\t': indent+=tabwidth; p++; goto line;
	}

	goto expr_entry;

comment:
#if PARSERDEBUG >= 5
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
#if PARSERDEBUG >= 3
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
		case '%' : p++; goto binnum;
		case '$' : p++; goto here_or_hex;
	}

	if (*p >= '0' && *p <= '9') goto decnum;
	if (islabelchar[*p] >= 3 || (currblock->haslabels && islabelchar[*p] == 1)) goto ident;

	if ((*p == '\n' || *p == '\r') && stack_peek(vstack) == NULL)
	{
		goto newline;
	}

	currop = currop[tolower(*p)].next;


	if (currop != NULL)
	{
		p++;

		while (*p && currop[tolower(*p)].next != NULL)
		{
#if PARSERDEBUG >= 4
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


#if PARSERDEBUG >= 3
		printf("prefix op operator: \"%s\"\n", op->name);
#endif
		fold(op);

		goto expr;
	}

	goto error;

ident:
	ts = p;
ident_l:
	while (*p && (islabelchar[*p] >= (currblock->haslabels ? 1 : 2)) || currop != NULL)
	{
#if PARSERDEBUG >= 4
		printf("ident: '%c'\n", *p);
#endif

		if (currop != NULL)
		{
			currop = currop[tolower(*p)].next;
		}

		p++;
	}

	if (currop != NULL)
	{
#if PARSERDEBUG >= 4
		printf("ident: op\n");
#endif

		operator* op = currop[0].op;

		if (op != NULL)
		{
#if PARSERDEBUG >= 3
			printf("prefix ident operator: \"%s\"\n", op->name);
#endif
			fold(op);

			if (op->rightarg)
			{
				goto expr;
			}
			else
			{
				goto operator;
			}
		}
	}

	char* base = "";

	if (*ts == ':')
	{
		base = lastgloballabel;
	}

	char* s = tok2stra(base, ts, p);

#if PARSERDEBUG >= 3
	printf("ident: \"%s\"\n", s);
#endif
	operand* val = ident_new_intern(&s);

	symbol* sym = val->val.ident;

	int push = 1;

	if (stack_peek(vstack) == NULL)
	{
		if (currblock->haslabels)
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

			currblock = sym->data.block;

			symbols = currblock->symbols;
			operators = currblock->operators;
			preoperators = currblock->preoperators;

			indentstack->top = 0;

			push = 0;
		}
	}
	else
	{
		if (sym->type == SYM_BLOCK)
		{
			printf("Error: block name not first token on line!\n");

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
	p++;

	ts = p;

string_mid:
#if PARSERDEBUG >= 3
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
		p++;
	}
	else
	{
		nbase = 2;
	}
num:
	ts = p;

	plong n = 0;
num_l:
#if PARSERDEBUG >= 4
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
		p++;
		goto num_float;
	}
	else if (nbase < 0xE && *p == 'e')
	{
		goto num_float;
	}
	else
	{
		goto num_done;
	}

	n = n * nbase + digit;
	p++;

	goto num_l;

num_float:
	;
	float nf = n;
	float exponent = 1/(float)nbase;

num_float_l:
#if PARSERDEBUG >= 4
	printf("num_float: '%c'\n", *p);
#endif
	switch (*p)
	{
		case 0   : goto error;
	}

	digit = 0;
	if (*p >= '0' && *p <= '9')
	{
		digit = *p - '0';
	}
	else if (nbase < 0xE && (*p == 'e' || *p == 'E'))
	{
		p++;
		goto num_float_e;
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
	else
	{
		goto num_float_done;
	}

	nf += digit * exponent;
	exponent /= nbase;
	p++;

	goto num_float_l;

num_float_e:
	;
	int sign = 1;

	int e = 0;

num_float_e_l:
#if PARSERDEBUG >= 4
	printf("num_float_e: '%c'\n", *p);
#endif
	switch (*p)
	{
		case 0   : goto error;
	}

	digit = 0;
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
	else
	{
		nf *= powf(nbase, e);
		goto num_float_done;
	}

	e = e * nbase + digit;
	p++;

	goto num_float_e_l;

num_float_done:

#if PARSERDEBUG >= 3
	printf("num_float: %g\n", nf);
#endif
	n = *(plong*)&nf;

num_done:

#if PARSERDEBUG >= 3
	printf("num: %d\n", n);
#endif
	stack_push(vstack, int_new(n));

	goto operator;

here_or_hex:
#if PARSERDEBUG >= 3
	printf("here or hex: '%c'\n", *p);
#endif
	ts = p;

	if (isxdigit(*p) || *p == '_') goto hexnum;

	stack_push(vstack, ident_new("$"));
	goto operator;

operator:
	currop = operators;

#if PARSERDEBUG >= 4
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
#if PARSERDEBUG >= 4
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


#if PARSERDEBUG >= 3
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

	return blocks;


end:
#if PARSERDEBUG
	printf("EOF\n");
#endif

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
	islabelchar       [':'] = 1;
	//islabelchar       ['.'] = 3;
	islabelchar_r('0', '9',   2);
	islabelchar_r(128, 255,   3); // UTF-8 support

	vstack = stack_new();
	ostack = stack_new();

	parserlib_init();
}
