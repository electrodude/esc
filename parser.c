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

static int precedence(char op)
{
	switch (op)
	{
		case 'n' : return 7;
		case '/' : return 6;
		case '*' : return 5;
		case '-' : return 4;
		case '+' : return 3;
		case '#' : return 2;
		case ',' : return 1;
		case ' ' : return 0;
		case '(' : return INT_MIN;
		case ')' : return INT_MIN;
		default  : return INT_MIN;
	}
}

static void fold(char nextop)
{
#if PARSERDEBUG
	printf("fold: nextop = %c (0x%x)\n", nextop, nextop);
#endif

	char topop;

	while (stack_peek(ostack) != NULL && precedence(nextop) < precedence(topop = *(char*)stack_peek(ostack)))
	{
		char op = *(char*)stack_pop(ostack);
#if PARSERDEBUG
		printf("fold: op %c (0x%x)\n", op, op);
#endif
		operand* rhs = (op != 'n' && op != '#') ? stack_pop(vstack) : NULL;
		operand* lhs = stack_pop(vstack);
		stack_push(vstack, binop_new(op, lhs, rhs));
	}

	if (nextop == 0 && stack_peek(ostack) != NULL && *(char*)stack_peek(ostack) == '(')
	{
		printf("Error: unmatched left parentheses\n");
		exit(1);
	}
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
	char* ps = p;

	char* ts = p;

	//char* s;

	int pc = 5;

	lineno = 1;
	char* linestart = p;

#if PARSERDEBUG >= 2
	printf("Line %d\n", lineno);
#endif

	char* lastgloballabel = "!begin";

	stack* lines = stack_new();

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

#if 1
		operand_print(currline->operand);
		printf("\n");
#endif

		stack_push(lines, currline);

		currline = malloc(sizeof(line));
	}

	goto line;

line:
#if PARSERDEBUG >= 2
	//printf("line\n", *p);
	printf("line: %c\n", *p);
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
#if PARSERDEBUG >= 2
	printf("comment: %c\n", *p);
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
		char* stray = (char*)stack_peek(ostack);
		if (stray != NULL)
		{
			printf("Error at %d:%ld: stray operator on operator stack: %c\n", lineno, p-linestart, *stray);
			exit(1);
		}
	}
	{
		operand* stray = stack_peek(vstack);
		if (stray != NULL)
		{
			printf("Error at %d:%ld: stray value on value stack: ", lineno, p-linestart);
			operand_print(stray);
			printf("\n");
			exit(1);
		}
	}

expr:
#if PARSERDEBUG >= 2
	printf("expr: %c\n", *p);
#endif
	switch (*p)
	{
		case 0   : goto end;
		case ' ' :
		case '\t': p++; goto expr;
		case '$' : goto here_or_hex;
		case '-' : goto unm;
		case '#' : goto indirect;
		case '(' : goto lparen;
		case '%' : p++; goto binnum;
		case '{' : eatblockcomment(&p); goto expr;
	}

	if (*p >= '0' && *p <= '9') goto decnum;
	if (islabelchar[*p] >= 3) goto ident;

	if ((*p == '\n' || *p == '\r') && stack_peek(vstack) == NULL)
	{
		goto newline;
	}

	goto error;

lparen:
#if PARSERDEBUG
	printf("lparen: %c\n", *p);
#endif
	stack_push(ostack, "(");
	p++;

	goto expr;

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
num_l:
#if PARSERDEBUG >= 2
	printf("num: %c\n", *p);
#endif
	switch (*p)
	{
		case 0   : goto error;
	}

	if ((*p >= '0' && *p <= '9') || *p == '_') { p++; goto num_l; }

	char* s = tok2str(ts, p);
#if PARSERDEBUG
	printf("num: %s\n", s);
#endif
	stack_push(vstack, int_new(atoi(s)));
	free(s);

	goto operator;

ident:
	ts = p;
ident_l:
#if PARSERDEBUG >= 2
	printf("ident: %c\n", *p);
#endif
	switch (*p)
	{
		case 0   : goto error;
	}
	if (islabelchar[*p] >= 2) { p++; goto ident_l; }

	char* base = "";

	if (*ts == ':')
	{
		base = lastgloballabel;
	}

	s = tok2stra(base, ts, p);

#if PARSERDEBUG
	printf("ident: %s\n", s);
#endif
	operand* op = ident_new(s);

	if (stack_peek(vstack) == NULL)
	{
		symbol* sym = op->val.ident;
		if (sym->type == SYM_UNKNOWN)
		{
			if (*ts != ':')
			{
				lastgloballabel = s;
			}

			sym->type = SYM_LABEL;
		}
		else if (sym->type == SYM_LABEL)
		{
			printf("Duplicate symbol \"%s\"!\n", s);
			exit(1);
		}
	}

	stack_push(vstack, op);

	goto operator;

unm:
#if PARSERDEBUG
	printf("unm: %c\n", *p);
#endif
	stack_push(ostack, "n");
	p++;
	goto expr;

indirect:
#if PARSERDEBUG
	printf("indirect: %c\n", *p);
#endif
	stack_push(ostack, "#");
	p++;
	goto expr;

here_or_hex:
#if PARSERDEBUG
	printf("here or hex: %c\n", *p);
#endif
	p++;
	if (isxdigit(*p)) goto hexnum;

	stack_push(vstack, ref_new(currline));
	goto operator;

operator:
#if PARSERDEBUG >= 2
	printf("operator: %c\n", *p);
#endif
	switch (*p)
	{
		case 0   : goto error;
		case ' ' :
		case '\t': p++; goto operator;
		case '\n':
		case '\r': fold(0); p++; goto newline;
		case '\'': fold(0); p++; goto comment;
		case '{' : eatblockcomment(&p); goto operator;
		case ',' :
		case '+' :
		case '-' :
		case '*' :
		case '/' :
		{
			fold(*p);
#if PARSERDEBUG
			printf("operator: %c\n", *p);
#endif
			stack_push(ostack, p);
			p++; goto expr;
		}
		case ')' : goto rparen;
	}

	fold(' ');
#if PARSERDEBUG >= 2
	printf("operator: whitespace\n");
#endif

	stack_push(ostack, " ");
	goto expr;

rparen:
#if PARSERDEBUG
	printf("rparen: %c\n", *p);
#endif
	fold(')');
	char* lp = (char*)stack_pop(ostack);
	if (lp == NULL || *lp != '(')
	{
		printf("Error: unmatched right parentheses");
		exit(1);
	}
	p++;

	goto operator;

error:
	printf("Parse error at %d:%ld: %c (%02X)\n", lineno, p-linestart, *p, *p);
	exit(1);

end:	; // silly compile error without the ;

	stack_push(lines, currline);

	for (int i=0; i < lines->top; i++)
	{
		line* l = lines->base[i];
		operand_print(l->operand);
		printf("\n");

	}

	return lines;
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
	islabelchar_r(0, 255,     0);
	islabelchar       ['$'] = 1;
	islabelchar       ['%'] = 1;
	islabelchar       ['-'] = 1;
	islabelchar       ['#'] = 1;
	islabelchar       ['('] = 1;
	islabelchar_r('A', 'Z',   3);
	islabelchar_r('a', 'z',   3);
	islabelchar       ['_'] = 3;
	islabelchar       [':'] = 3;
	islabelchar       ['.'] = 3;
	islabelchar_r('0', '9',   2);

	vstack = stack_new();
	ostack = stack_new();
}
