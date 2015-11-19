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

	if (nextop != NULL && nextop->grammar != NULL)
	{
#if PARSERDEBUG >= 2
		printf("op grammar push\n");
#endif
		grammar_push(nextop->grammar);
	}

	if (nextop != NULL && nextop->leftarg == 0 && nextop->rightarg >= 2)
	{
		// don't fold if left bracket

#if PARSERDEBUG >= 3
		printf("fold: left bracket, no fold\n");
#endif

		// BAD: duplicate of the end of this function

		goto folddone;
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
			goto folded;
		}

		operand* rhs = NULL;
		if (topop->rightarg && (topop->rightarg >= 0 || vstack->top >= 2))
		{
			rhs = stack_pop(vstack);
			if (rhs == NULL)
			{
				printf("Error: rhs of '%s' (%d, %d) == NULL!\n", topop->name, topop->leftarg, topop->rightarg);
				exit(1);
			}
		}

		int hash_hack = 0;

		operand* lhs = NULL;
		if (topop->leftarg)
		{
			operand* lhs_peek = stack_peek(vstack);
			if (topop->leftarg == -1 && lhs_peek != NULL && lhs_peek->type == IDENT && (lhs_peek->val.ident->type != SYM_LABEL && lhs_peek->val.ident->type != SYM_UNKNOWN))
			{
				// deal with cases like "jmp #label"
				hash_hack = 1;
			}
			else
			{
				lhs = stack_pop(vstack);
				if (lhs == NULL && topop->rightarg >= 0)
				{
					printf("Error: lhs of '%s' (%d, %d) == NULL!\n", topop->name, topop->leftarg, topop->rightarg);
					exit(1);
				}
			}
		}

		if (topop->grammar != NULL)
		{
#if PARSERDEBUG >= 2
			printf("op grammar pop\n");
#endif
			grammardef* oldgrammar = grammar_pop();

			if (oldgrammar != topop->grammar)
			{
				printf("Mismatched grammar pop! %p, %p, %p\n", oldgrammar, topop->grammar, nextop != NULL ? nextop->grammar : NULL);
				exit(1);
			}
		}

		stack_push(vstack, binop_new(topop, lhs, rhs));

		if (hash_hack)
		{
			stack_push(ostack, grammar->operators[0].op);
		}

		if (topop != NULL && nextop != NULL && nextop->leftarg >= 2 && topop->rightarg == nextop->leftarg)
		{
#if PARSERDEBUG >= 3
			printf("fold function break\n");
#endif
			goto folded;
		}
	}

#if PARSERDEBUG >= 3
	printf("fold: done\n");
#endif

	topop = stack_peek(ostack);

	if (topop != NULL && nextop != NULL && topop->rightarg >= 2 && nextop->leftarg >= 2 && topop->rightarg != nextop->leftarg)
	{
		printf("Error: mismatched brackets: %d, %d\n", topop->rightarg, nextop->leftarg);
		exit(1);
	}

folded:

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

folddone:
	if (nextop != NULL && nextop->push)
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


	blockdef* currblockdef = symbol_get("con")->data.block;


#if PARSERDEBUG >= 3
	printf("grammar\n");
#endif
	grammar = currblockdef->grammar; // should this be headergrammar instead?


	stack* blocks = stack_new();

	stack* lines;

	block_new(blocks, currblockdef, &lines);

	line* currline = malloc(sizeof(line));
	line* prevline = NULL;

	unsigned int indent = 0;

	stack* indentstack = stack_new();

	goto line;

newline:
#if PARSERDEBUG >= 3
	printf("\\n\n");
#endif

#if PARSERDEBUG >= 3
	printf("grammar\n");
#endif
	grammar = currblockdef->grammar;

	if (stack_peek(vstack) != NULL)
	{
		currline->operand = stack_pop(vstack);

		if (grammar->hasindent)
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
		case '\r': indent = 0; lineno++; linestart=p+1; p++;
#if PARSERDEBUG >= 3
		           printf("grammar\n");
#endif
		           grammar = currblockdef->grammar;
		           goto line;

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
	currop = grammar->preoperators;

	switch (*p)
	{
		case 0   : goto end;
		case ' ' :
		case '\t': p++; goto expr;
		case '{' : eatblockcomment(&p); goto expr;
		case '"' : goto string;
		case '%' : p++; goto binnum;
		case '$' : p++; goto here_or_hex;
		case '\'': fold(NULL); goto comment;
		case '\n':
		case '\r': fold(NULL); p++; goto newline;
	}

	if (*p >= '0' && *p <= '9') goto decnum;
	if (islabelchar[*p] >= 3 || (grammar->haslabels && islabelchar[*p] == 1)) goto ident;

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
	while (*p && (islabelchar[*p] >= (grammar->haslabels ? 1 : 2)))
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
		char* p2 = p;
		while (*p2 && currop[tolower(*p)].next != NULL)
		{
#if PARSERDEBUG >= 4
			printf("prefix ident operator char: '%c'\n", *p2);
#endif
			currop = currop[tolower(*p)].next;

			p2++;
		}

		operator* op = currop[0].op;

		if (op != NULL)
		{
#if PARSERDEBUG >= 3
			printf("prefix ident operator: \"%s\"\n", op->name);
#endif
			fold(op);

			p = p2;

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

	if (stack_peek(vstack) == NULL)
	{
		if (grammar->haslabels)
		{
			// DAT blocks have local labels, which we need to scope now
			if (sym->type == SYM_UNKNOWN)
			{
				// *ts as opposed to *s since s has lastgloballabel prepended to it
				if (*ts != ':' && *ts != '.')
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

			currblockdef = sym->data.block;

			block_new(blocks, currblockdef, &lines);

#if PARSERDEBUG >= 3
			printf("headergrammar\n");
#endif
			grammar = currblockdef->headergrammar;

			indentstack->top = 0;

			goto line;
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

	stack_push(vstack, val);

	goto operator;


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
	else if (*p == '.' && /* check for operator starting with . */
	                      (grammar->operators['.'].next == NULL || grammar->operators['.'].next[tolower(p[1])].next == NULL))
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
	currop = grammar->operators;

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
		fold(grammar->operators[0].op);
/*
// TODO: Running this commented out results in an infinte loop.  Commenting this
//  out and just leaving the "goto ident" seems to result in some, if not all,
//  undefined operators getting through and being registered as the null ""
//  operator.
#if PARSERDEBUG >= 3
		printf("ident->op backtrack\n");
#endif
		p = ts; // backtracking! Blech!
*/
		goto ident;
	}


#if PARSERDEBUG >= 3
	printf("operator: \"%s\"\n", op->name);
#endif

	if (!op->leftarg)
	{
		fold(grammar->operators[0].op);
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

	return NULL;

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
	printf("EOF\n\n");
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
