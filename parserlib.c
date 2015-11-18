#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "stack.h"

#include "parserlib.h"

#define LIBDEBUG 0


// symbol table

symtabentry* symbols;

optabentry* operators;
optabentry* preoperators;


symbol* symbol_get(char* p)
{
	char* s2 = p;

	symtabentry* symtab = symbols;

	//printf("symbol_get(\"%s\"): ", p);
	while (*p != 0)
	{
#if LIBDEBUG >= 2
		putchar(*p);
#endif
		unsigned char c = tolower(*p);

		symtabentry* symtab2 = symtab[c].next;
		if (symtab2 == NULL)
		{
			symtab2 = symtab[c].next = calloc(256,sizeof(symtabentry));
		}
		symtab = symtab2;
		p++;
	}
#if LIBDEBUG >= 2
	putchar('\n');
#endif
	if (symtab[0].sym == NULL)
	{
		symbol* sym = malloc(sizeof(symbol));
		sym->type = SYM_UNKNOWN;
		sym->name = s2;
		sym->defined = 0;

		symtab[0].sym = sym;

		return sym;
	}

	return symtab[0].sym;
}

symbol* symbol_define(char* s, symboltype type)
{
	symbol* sym = symbol_get(s);

	if (sym->defined)
	{
		printf("symbol %s already defined as %s!\n", s, sym->type == SYM_LABEL ? "label" : sym->type == SYM_OPCODE ? "opcode" : "unknown");
		return NULL;
	}

	sym->defined = 1;
	sym->type = type;

	return sym;
}

void symbol_print(symbol* sym)
{
	switch (sym->type)
	{
		case SYM_LABEL:
		{
			printf("label ");
			break;
		}
		case SYM_OPCODE:
		{
			printf("opcode ");
			break;
		}
		case SYM_MODIFIER:
		{
			printf("modifier ");
			break;
		}
		case SYM_BLOCK:
		{
			printf("block ");
			break;
		}
		default:
		{
			//printf("??? ");
			break;
		}
	}

	printf("%s", sym->name);
}


// operator

static inline int operator_alias(optabentry* optab, char* p, operator* op, int overwriteif)
{
	const char* s2 = p;

	//printf("operator_alias(\"%s\"): ", p);
	while (*p != 0)
	{
#if LIBDEBUG >= 2
		putchar(*p);
#endif
		unsigned char c = tolower(*p);

		optabentry* optab2 = optab[c].next;
		if (optab2 == NULL)
		{
			optab2 = optab[c].next = calloc(256,sizeof(optabentry));
		}
		optab = optab2;
		p++;
	}
#if LIBDEBUG >= 2
	putchar('\n');
#endif
	// if there's already something here, and both are actually supposed to
	//  have left arguments (this is the left argument table), then complain
	if (optab[0].op != NULL)
	{
		operator* op2 = optab[0].op;

#if LIBDEBUG
		printf("operator \"%s\" already defined: \"%s\" (%d, %d): ", s2, op2->name, op2->leftarg, op2->rightarg);
#endif

		if (!overwriteif || (op->leftarg && op2->leftarg))
		{
#if LIBDEBUG
			printf("erroring\n");
#endif
			return 1;
		}
		else if (op->leftarg)
		{
#if LIBDEBUG
			printf("overwriting\n");
#endif
		}
		else
		{
#if LIBDEBUG
			printf("yielding\n");
#endif
			return 0;
		}
	}

	optab[0].op = op;

	return 0;
}

operator* operator_new(char* s, double precedence, int leftarg, int rightarg)
{
#if LIBDEBUG
	printf("operator_new \"%s\" (%d, %d, %g)\n", s, leftarg, rightarg, precedence);
#endif

	operator* op = malloc(sizeof(operator));
	op->precedence = precedence;
	op->name = s;

	op->leftarg = leftarg;
	op->rightarg = rightarg;

	if (leftarg == 0)
	{
		if (operator_alias(preoperators, s, op, 0))
		{
			printf("operator \"%s\" already defined!\n", s);
			return NULL;
		}
	}

	//if (leftarg >= 0)
	{
		if (operator_alias(operators, s, op, 1))
		{
			printf("operator \"%s\" already defined!\n", s);
			return NULL;
		}
	}

#if LIBDEBUG
	printf("\n");
#endif

	return op;
}

static optabentry* optabentry_clone(optabentry* optab)
{
	if (optab == NULL)
	{
		return NULL;
	}

	optabentry* optab2 = calloc(256, sizeof(optabentry));

	optab2[0].op = optab[0].op;

	for (int i=1; i < 256; i++)
	{
		optab2[i].next = optabentry_clone(optab[i].next);
	}

	return optab2;
}

static stack* grammarstack;

void grammar_push(void)
{
	// save old grammar
	blockdef* block = malloc(sizeof(blockdef));

	block->symbols = symbols;
	block->operators = operators;
	block->preoperators = preoperators;

	stack_push(grammarstack, block);

	// and clone it

	//symbols = symtabentry_clone(symbols);
	operators = optabentry_clone(operators);
	preoperators = optabentry_clone(preoperators);
}

void grammar_pop(void)
{
	blockdef* block = stack_pop(grammarstack);

	if (block == NULL)
	{
		printf("Grammar stack underflow!\n");
		exit(1);
	}

	symbols = block->symbols;
	operators = block->operators;
	preoperators = block->preoperators;

	free(block);
}


blockdef* block_new(char* s)
{
	symbol* sym = symbol_define(s, SYM_BLOCK);

	if (sym == NULL) return NULL;

	blockdef* block = malloc(sizeof(blockdef));

	block->symbols = symbols;
	block->operators = operators;
	block->preoperators = preoperators;

	block->haslabels = 0;

	sym->data.block = block;

	return block;
}

void block_select(blockdef* block)
{
	symbols = block->symbols;
	operators = block->operators;
	preoperators = block->preoperators;
}

symbol* label_new(char* s)
{
	symbol* sym = symbol_define(s, SYM_LABEL);

	if (sym == NULL) return NULL;

	return sym;
}

symbol* mod_new(char* s, const char* bits)
{
	symbol* sym = symbol_define(s, SYM_MODIFIER);

	if (sym == NULL) return NULL;

	/*
	opcode* op = malloc(sizeof(opcode));

	op->name = s;
	//op->bits = bitfield_new_s(bits);

	sym->data.op = op;
	*/

	return sym;
}

opcode* opcode_new(char* s, const char* bits)
{
	symbol* sym = symbol_define(s, SYM_OPCODE);

	if (sym == NULL) return NULL;

	opcode* op = malloc(sizeof(opcode));

	op->name = s;
	//op->bits = bitfield_new_s(bits);

	sym->data.op = op;

	return op;
}


operand* int_new(plong x)
{
	operand* this = malloc(sizeof(operand));

	this->tp = INT;
	this->val.val = x;

	return this;
}

operand* ref_new(line* l)
{
	operand* this = malloc(sizeof(operand));

	this->tp = REF;
	this->val.line = l;

	return this;
}

operand* ident_new(char** p)
{
	operand* this = malloc(sizeof(operand));

	this->tp = IDENT;
	this->val.ident = symbol_get(*p);

	char* s = this->val.ident->name;

	if (s != *p)
	{
		char* p_old = *p;

		*p = s;

		free(p_old);
	}

	return this;
}

operand* string_new(char* s)
{
	operand* this = malloc(sizeof(operand));

	this->tp = STRING;
	this->val.str = s;

	return this;
}

operand* binop_new(operator* op, operand* lhs, operand* rhs)
{
	operand* this = malloc(sizeof(operand));

	this->tp = BINOP;
	this->val.binop.op = op;
	this->val.binop.operands[0] = lhs;
	this->val.binop.operands[1] = rhs;

	return this;
}

/*
int operand_eval(operand* this)
{
	if (this == NULL)
	{
		return 0;
	}

#if LIBDEBUG >= 2
	printf("operand_eval: ");
#endif

	switch (this->tp)
	{
		case INT:
		{
#if LIBDEBUG >= 2
			printf("int: %d\n", this->val.val);
#endif
			return this->val.val;
		}
		case IDENT:
		{
#if LIBDEBUG >= 2
			printf("label: %s\n", this->val.ident);
#endif
			return this->val.ident->data.reg->caddr;
		}
		case BINOP:
		{
#if LIBDEBUG >= 2
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
*/

static void operand_print_list(operand* this)
{
	if (this->tp != BINOP || this->val.binop.op == NULL || this->val.binop.op->name[0] != 0)
	{
		operand_print(this);
		return;
	}

	operand** operands = this->val.binop.operands;

	operand_print_list(operands[0]);

	printf(" ");

	operand_print_list(operands[1]);
}

void operand_print(operand* this)
{
	if (this == NULL)
	{
		printf("NULL");
		return;
	}

	switch (this->tp)
	{
		case INT:
		{
			printf("%u", this->val.val);

			break;
		}
		case IDENT:
		{
			printf("(");
			symbol_print(this->val.ident);
			printf(")");

			break;
		}
		case BINOP:
		{
			operator* op = this->val.binop.op;
			if (op != NULL && op->name[0] == 0)
			{
				printf("[");
				operand_print_list(this);
				printf("]");
			}
			else
			{
				printf("(");

				if (op != NULL)
				{
					printf("'%s'", op->name);
				}
				else
				{
					printf("NULL");
				}

				if (this->val.binop.operands[0] != NULL)
				{
					printf(" ");
					operand_print(this->val.binop.operands[0]);
				}
				if (this->val.binop.operands[1] != NULL)
				{
					printf(" ");
					operand_print(this->val.binop.operands[1]);
				}

				printf(")");
			}

			break;
		}
		case REF:
		{
			//printf("[%s]", this->val.line->name);
			printf("[line]");

			break;
		}
		case STRING:
		{
			printf("\"%s\"", this->val.str);

			break;
		}
		default:
		{
			printf("? type=%d ?", this->tp);

			break;
		}
	}
}

/*
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
*/

void parserlib_init(void)
{
	symbols = malloc(sizeof(symtabentry)*256);
	operators = malloc(sizeof(optabentry)*256);
	preoperators = malloc(sizeof(optabentry)*256);

	grammarstack = stack_new();
}
