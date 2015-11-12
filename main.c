#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#define ASMDEBUG 2
#define LIBDEBUG 0


// symbol

typedef struct symbol
{
	union
	{
		struct
		{
		} label;
		struct
		{
		} opcode;
	} data;
	const char* s;
	enum
	{
		SYM_LABEL,
		SYM_OPCODE,
	} type;
} symbol;

// symbol table

typedef union symtabentry
{
	union symtabentry* next; // pointer to array of 256 symtabentries
	symbol* sym;
} symtabentry;

symtabentry symbols[256];

void addsymbol(char* p, symbol* sym)
{
	symtabentry* symtab = symbols;

	while (*p != 0)
	{
		char c = tolower(*p);

		symtabentry* symtab2 = symtab[c].next;
		if (symtab2 == NULL)
		{
			symtab2 = symtab[c].next = calloc(256,sizeof(symtabentry));
		}
		symtab = symtab2;
		p++;
	}
	symtab[0].sym = sym;
}

symbol* getsymbol(char* p)
{
	char* s2 = p;

	symtabentry* symtab = symbols;

	//printf("getsymbol(\"%s\"): ", p);
	while (*p != 0)
	{
#if LIBDEBUG
		putchar(*p);
#endif
		symtab = symtab[tolower(*p)].next;
		if (symtab == NULL)
		{
			//printf("\nUnknown symbol: %s\n", s2);
			//exit(1);
			return NULL;
		}
		p++;
	}
#if LIBDEBUG
	putchar('\n');
#endif
	return symtab[0].sym;
}


symbol* label_new(const char* s)
{
	symbol* sym = malloc(sizeof(symbol));

	sym->type = SYM_LABEL;
	sym->s = s;

	addsymbol(s, sym);

	return sym;
}

symbol* opcode_new(const char* s)
{
	symbol* sym = malloc(sizeof(symbol));

	sym->type = SYM_OPCODE;
	sym->s = s;

	addsymbol(s, sym);

	return sym;
}

// expression
typedef struct operand operand;

typedef struct operand
{
	enum {INT, IDENT, BINOP} tp;
	union
	{
		int val;
		char* ident;
		struct
		{
			operand* operands[2];
			char op;
		} binop;
	} val;
} operand;

operand* int_new(int x)
{
	operand* this = malloc(sizeof(operand));

	this->tp = INT;
	this->val.val = x;

	return this;
}

operand* ident_new(char* p)
{
	operand* this = malloc(sizeof(operand));

	this->tp = IDENT;
	this->val.ident = p;

	return this;
}

operand* binop_new(char op, operand* lhs, operand* rhs)
{
	operand* this = malloc(sizeof(operand));

	this->tp = BINOP;
	this->val.binop.op = op;
	this->val.binop.operands[0] = lhs;
	this->val.binop.operands[1] = rhs;

	return this;
}

int operand_eval(operand* this)
{
	if (this == NULL)
	{
		return 0;
	}

#if LIBDEBUG
	printf("operand_eval: ");
#endif

	switch (this->tp)
	{
		case INT:
		{
#if LIBDEBUG
			printf("int: %d\n", this->val.val);
#endif
			return this->val.val;
		}
		case IDENT:
		{
#if LIBDEBUG
			printf("label: %s\n", this->val.ident);
#endif
			return getsymbol(this->val.ident);
		}
		case BINOP:
		{
#if LIBDEBUG
			printf("binop: %c\n", this->val.binop.op);
#endif
			int lhs = operand_eval(this->val.binop.operands[0]);
			int rhs = operand_eval(this->val.binop.operands[1]);
			switch (this->val.binop.op)
			{
				case '+':
				{
					return lhs + rhs;
				}
				case '-':
				{
					return lhs - rhs;
				}
				case '*':
				{
					return lhs * rhs;
				}
				case '/':
				{
					return lhs / rhs;
				}
				case 'n':
				{
					return -lhs;
				}
			}
			printf("Error: unknown op: %c\n", this->val.binop.op);
			exit(1);
			return 0;
		}
	}
	printf("Error: unknown value type: %d\n", this->tp);
	exit(1);
}

void operand_kill(operand* this)
{
	if (this == NULL)
	{
		return;
	}

	switch (this->tp)
	{
		case INT:
		{
			break;
		}
		case IDENT:
		{
			free(this->val.ident);
			break;
		}
		case BINOP:
		{
			operand_kill(this->val.binop.operands[0]);
			operand_kill(this->val.binop.operands[1]);
			break;
		}
		default:
		{
			printf("Error: unknown value type: %d\n", this->tp);
			//exit(1); // who cares about memory leaks? :)
			break;
		}
	}

	free(this);
}


// stack

typedef struct cons
{
	struct cons* next;
	void* this;
} cons;

typedef struct stack
{
	void** base;
	int top;
	int len;
} stack;

stack* stack_new()
{
	stack* this = malloc(sizeof(stack));

	this->len = 16;
	this->base = malloc(this->len*sizeof(void*));
	this->top = 0;

	return this;
}

void stack_push(stack* this, void* v)
{
	this->top++;

	if (this->top >= this->len)
	{
		this->len *= 2;
		this->base = realloc(this->base, this->len*sizeof(void*));
#if LIBDEBUG
		printf("resize stack to %d\n", this->len);
#endif
	}

	this->base[this->top] = v;
}

void* stack_pop(stack* this)
{
	if (this->top <= 0)
	{
		return NULL;
	}

	return this->base[this->top--];
}

void* stack_peek(stack* this)
{
	if (this->top <= 0)
	{
		return NULL;
	}

	return this->base[this->top];
}
	

// parser

stack* vstack;

stack* ostack;

char* tok2str(char* start, char* end)
{
	char* s = malloc(end - start + 1);

	strncpy(s, start, end - start);

	s[end-start] = 0;

	return s;
}

int precedence(char op)
{
	switch (op)
	{
		case 'n' : return 6;
		case '#' : return 6;
		case '/' : return 5;
		case '*' : return 4;
		case '-' : return 3;
		case '+' : return 2;
		case ',' : return 1;
		case ' ' : return 0;
		case '(' : return INT_MIN;
		case ')' : return INT_MIN;
		default  : return INT_MIN;
	}
}

void fold(char nextop)
{
#if ASMDEBUG
	printf("fold: nextop = %c (0x%x)\n", nextop, nextop);
#endif

	char topop;

	while (stack_peek(ostack) != NULL && precedence(nextop) < precedence(topop = *(char*)stack_peek(ostack)))
	{
		char op = *(char*)stack_pop(ostack);
#if ASMDEBUG
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

unsigned int lineno = 0;

void eatblockcomment(const char** pp)
{
#if ASMDEBUG >= 2
	printf("block comment on line %d: ", lineno);
#endif

	int level = 0;

	const char* p = *pp;

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
	printf("ate %ld chars\n", p - *pp);

	*pp = p;
}

char islabelchar[256];

int* parser(char* p)
{
	char* ps = p;

	char* ts = p;

	//char* s;

	int pc = 5;

	lineno = 1;
	char* linestart = p;

#if ASMDEBUG >= 2
	printf("Line %d\n", lineno);
#endif

	int memlen = 64;
	operand** mem = malloc(memlen*sizeof(operand*));

	int isinstr = 0;

	goto line;

newline:
#if ASMDEBUG
	printf("\\n\n");
#endif

	lineno++;
	linestart = p;

#if ASMDEBUG >= 2
	printf("Line %d\n", lineno);
#endif

	if (isinstr)
	{
		operand* val = stack_pop(vstack);
		/*
		mem[pc] = stack_pop(vstack);

		pc++;
		if (pc >= memlen)
		{
			memlen *= 2;
			mem = realloc(mem, memlen*sizeof(int));
		}
		*/
	}
	
	isinstr = 0;

	goto line;

line:
#if ASMDEBUG >= 2
	//printf("line\n", *p);
	printf("line: %c\n", *p);
#endif
	switch (*p)
	{
		case 0   : goto end;
		case '\'': p++; goto comment;
		case '{' : eatblockcomment(&p);
		case '\n':
		case '\r': lineno++; linestart=p+1; 
		case ' ' :
		case '\t': p++; goto line;
	}

	if (islabelchar[*p] >= 2) goto label;

	goto error;

comment:
#if ASMDEBUG >= 2
	printf("comment: %c\n", *p);
#endif
	switch (*p)
	{
		case 0   : goto end;
		case '\n':
		case '\r': p++; goto newline;
	}

	p++; goto comment;

label:
	ts = p;
label_l:
#if ASMDEBUG >= 2
	printf("label: %c\n", *p);
#endif
	switch (*p)
	{
		case 0   : goto error;
	}
	if (islabelchar[*p] >= 2) { p++; goto label_l; }

	char* s = tok2str(ts, p);
	symbol* sym = getsymbol(s);
	if (sym != NULL)
	{
		switch (sym->type)
		{
		//case SYM_LABEL  : goto error;
			case SYM_OPCODE : printf("opcode: %s\n", s); goto expr_entry;
		}
	}
	else
	{
		label_new(s);
#if ASMDEBUG
		printf("label: %s\n", s);
#endif
	}

	goto line;

expr_entry:
#if ASMDEBUG
	printf("opcode\n");
#endif
	isinstr = 1;
	{
		char* stray = (char*)stack_peek(ostack);
		if (stray != NULL)
		{
			printf("Error at %d:%ld: stray operator: %c\n", lineno, p-linestart, *stray);
			exit(1);
		}
	}
	{
		operand* stray = stack_peek(vstack);
		if (stray != NULL)
		{
			printf("Error at %d:%ld: stray value: %d\n", lineno, p-linestart, operand_eval(stray));
			exit(1);
		}
	}

expr:
#if ASMDEBUG >= 2
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
	}

	if (*p >= '0' && *p <= '9') goto decnum;
	if (islabelchar[*p] >= 3) goto ident;

	if ((*p == '\n' || *p == '\r') && stack_peek(vstack) == NULL)
	{
		goto newline;
	}

	goto error;

lparen:
#if ASMDEBUG
	printf("lparen: %c\n", *p);
#endif
	stack_push(ostack, "(");
	p++;

	goto expr;

	int base;

decnum:
	base = 10;
	goto num;

hexnum:
	base = 16;
	goto num;

binnum:
	if (*p == '%')
	{
		base = 4;
	}
	else
	{
		base = 2;
	}
num:
	ts = p;
num_l:
#if ASMDEBUG >= 2
	printf("num: %c\n", *p);
#endif
	switch (*p)
	{
		case 0   : goto error;
	}

	if ((*p >= '0' && *p <= '9') || *p == '_') { p++; goto num_l; }

	s = tok2str(ts, p);
#if ASMDEBUG
	printf("num: %s\n", s);
#endif
	stack_push(vstack, int_new(atoi(s)));
	free(s);

	goto operator;

ident:
	ts = p;
ident_l:
#if ASMDEBUG >= 2
	printf("ident: %c\n", *p);
#endif
	switch (*p)
	{
		case 0   : goto error;
	}
	if (islabelchar[*p] >= 2) { p++; goto ident_l; }

	s = tok2str(ts, p);
#if ASMDEBUG
	printf("ident: %s\n", s);
#endif
	stack_push(vstack, ident_new(s));

	goto operator;

unm:
#if ASMDEBUG
	printf("unm: %c\n", *p);
#endif
	stack_push(ostack, "n");
	p++;
	goto expr;

indirect:
#if ASMDEBUG
	printf("indirect: %c\n", *p);
#endif
	stack_push(ostack, "#");
	p++;
	goto expr;

here_or_hex:
#if ASMDEBUG
	printf("here or hex: %c\n", *p);
#endif
	p++;
	if (isxdigit(*p)) goto hexnum;

	stack_push(vstack, int_new(pc));
	goto operator;

operator:
#if ASMDEBUG >= 2
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
		case '{' : eatblockcomment(&p);
		case ',' :
		case '+' :
		case '-' :
		case '*' :
		case '/' : 
		{
			fold(*p);
#if ASMDEBUG
			printf("operator: %c\n", *p);
#endif
			stack_push(ostack, p);
			p++; goto expr;
		}
		case ')' : goto rparen;
	}

	if ((islabelchar[*p] >= 1) && (p[-1] == ' ' || p[-1] == '\t'))
	{
		fold(' ');
		printf("operator: whitespace\n");

		stack_push(ostack, " ");
		goto expr;
	}

	goto error;

rparen:
#if ASMDEBUG
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
#if 0
	int* mem2 = malloc(pc*sizeof(int));
	for (int i=0; i<pc; i++)
	{
		mem2[i] = operand_eval(mem[i]);
		operand_kill(mem[i]);

#if ASMDEBUG
		printf("%d: %d\n", i, mem2[i]);
#endif
	}
	for (int i=0; i<pc; i++)
	{
		printf("%d: %d\n", i, mem2[i]);
	}

#if ASMDEBUG
	puts("done");
#endif
	return mem2;
#endif
}


// main

inline void islabelchar_r(size_t start, size_t end, char c)
{
	while (start <= end)
	{
		islabelchar[start++] = c;
	}
}

int main(int argc, char** argv)
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

	opcode_new("org");
	opcode_new("res");
	opcode_new("fit");

	opcode_new("byte");
	opcode_new("word");
	opcode_new("long");


	opcode_new("mov");
	opcode_new("add");
	opcode_new("abs");
	opcode_new("sub");
	opcode_new("cmp");
	opcode_new("shl");
	opcode_new("shr");
	opcode_new("sar");
	opcode_new("xor");
	opcode_new("and");
	opcode_new("test");
	opcode_new("or");
	opcode_new("muxc");
	opcode_new("negc");
	opcode_new("cmpsub");
	opcode_new("rev");
	opcode_new("rdbyte");
	opcode_new("wrbyte");
	opcode_new("rdword");
	opcode_new("wrword");
	opcode_new("rdlong");
	opcode_new("wrlong");
	opcode_new("lockset");
	opcode_new("lockclr");
	opcode_new("jmp");
	opcode_new("ret");
	opcode_new("tjz");
	opcode_new("tjnz");
	opcode_new("djnz");
	opcode_new("call");

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
	parser(s);
}
