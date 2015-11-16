#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "parserlib.h"


// symbol table

typedef union symtabentry
{
	union symtabentry* next; // pointer to array of 256 symtabentries
	symbol* sym;
} symtabentry;

static symtabentry symbols[256];


symbol* symbol_get(const char* p)
{
	const char* s2 = p;

	symtabentry* symtab = symbols;

	//printf("symbol_get(\"%s\"): ", p);
	while (*p != 0)
	{
#if LIBDEBUG
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
#if LIBDEBUG
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

symbol* symbol_define(const char* s, enum symboltype type)
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
			printf("label");
			break;
		}
		case SYM_OPCODE:
		{
			printf("opcode");
			break;
		}
		case SYM_MODIFIER:
		{
			printf("modifier");
			break;
		}
		case SYM_BLOCK:
		{
			printf("block");
			break;
		}
		default:
		{
			printf("???");
			break;
		}
	}

	printf(" %s", sym->name);
}



symbol* block_new(const char* s)
{
	symbol* sym = symbol_define(s, SYM_BLOCK);

	if (sym == NULL) return NULL;

	return sym;
}

symbol* label_new(const char* s, line* l)
{
	symbol* sym = symbol_define(s, SYM_LABEL);

	if (sym == NULL) return NULL;

	sym->data.line = l;

	return sym;
}

symbol* mod_new(const char* s, const char* bits)
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

symbol* opcode_new(const char* s, const char* bits)
{
	symbol* sym = symbol_define(s, SYM_OPCODE);

	if (sym == NULL) return NULL;

	opcode* op = malloc(sizeof(opcode));

	op->name = s;
	//op->bits = bitfield_new_s(bits);

	sym->data.op = op;

	return sym;
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

operand* binop_new(char op, operand* lhs, operand* rhs)
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
			return this->val.ident->data.reg->caddr;
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
*/

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
			printf("('%c'", this->val.binop.op);
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

			break;
		}
		case REF:
		{
			//printf("[%s]", this->val.line->name);
			printf("[line]");

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
