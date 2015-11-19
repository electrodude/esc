#pragma once

#include "parallax_types.h"

#include "bitfield.h"

#include "stack.h"

typedef uint16_t phaddr;
typedef uint16_t pcaddr;

// cog register

/*
typedef struct cogreg
{
	pcaddr caddr;

	char* name;
} cogreg;
*/


typedef struct opcode opcode;
typedef struct line line;


typedef union symtabentry symtabentry;
typedef struct symscope symscope;
typedef union optabentry optabentry;

typedef struct grammardef
{
	symscope* symbols;
	optabentry* operators;
	optabentry* preoperators;

	int haslabels : 1;
	int hasindent : 1;
} grammardef;

typedef struct blockdef
{
	grammardef* headergrammar;
	grammardef* grammar;

	char* name;
} blockdef;

typedef struct block
{
	blockdef* def;
	stack* lines;

	char* name;
} block;

// symbol

typedef struct operand operand;

typedef enum symboltype
{
	SYM_UNKNOWN,
	SYM_LABEL,
	SYM_OPCODE,
	SYM_BLOCK,
} symboltype;

typedef struct symbol
{
	union
	{
		operand* val;
		opcode* op;
		blockdef* block;
	} data;
	char* name;
	symboltype type;
	int defined;
} symbol;

// symbol table

typedef union symtabentry
{
	union symtabentry* next; // pointer to array of 256 symtabentries
	symbol* sym;
} symtabentry;

typedef struct symscope
{
	symtabentry* symtab;
	struct symscope* parent;
} symscope;


// instruction

/*
typedef struct instruction
{
	operand* operands;

	cogreg* reg;

	//struct* instruction next;
} instruction;
*/

// operator
typedef struct operator
{
	double precedence;
	char* name;

	int leftarg;
	int rightarg;

	int push : 1;

	grammardef* grammar;
} operator;

// operator table

typedef union optabentry
{
	union optabentry* next; // pointer to array of 256 symtabentries
	operator* op;
} optabentry;

extern grammardef* grammar;


// line

typedef struct line
{
	operand* operand;
	unsigned int indent;
	unsigned int indentdepth;
	struct line* parent;
} line;


// opcode

typedef void (*opcode_func)(symbol* op, line* l);

typedef struct opcode
{
	opcode_func func;
	void* data;

	bitfield bits;
	char* name;
} opcode;


// expression
typedef struct operand
{
	enum {INT, IDENT, PTR, STRING, BINOP} type;
	union
	{
		plong val;
		symbol* ident;
		char* str;
		struct
		{
			struct operand* operands[2];
			operator* op;
		} binop;
	} val;
} operand;


symbol* symbol_get(char* p);
symbol* symbol_get_if_exist(symtabentry* base, char* p);
symbol* symbol_define(char* s, symboltype type);

void symbol_print(symbol* sym);

symtabentry* symtabentry_new(void);
symtabentry* symtabentry_get(symtabentry* base, char* p);

void symscope_push(symtabentry* symtab);
symtabentry* symscope_pop(void);

grammardef* grammar_new(symscope* symbols, optabentry* operators, optabentry* preoperators, int haslabels, int hasindent);
void grammar_push(grammardef* newgrammar);
grammardef* grammar_pop(void);
grammardef* grammarstack_print(void);
void grammar_reset(grammardef* grammar);

block* block_new(stack* blocks, blockdef* def, stack** lines);

blockdef* blockdef_new(char* s);

operator* operator_new(char* p, double precedence, int leftarg, int rightarg);
symbol* label_new(char* s);
opcode* opcode_new(char* s, const char* bits);
symbol* modifier_new(char* s, const char* bits);



operand* int_new(plong x);
operand* ref_new(line* l);
operand* ident_new(char* s);
operand* ident_new_intern(char** p);
operand* string_new(char* s);
operand* binop_new(operator* op, operand* lhs, operand* rhs);

void operand_print(operand* this);

void parserlib_init(void);
