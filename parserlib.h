#pragma once

#include "parallax_types.h"

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
typedef union optabentry optabentry;


typedef struct blockdef
{
	symtabentry* symbols;
	optabentry* operators;
	optabentry* preoperators;
} blockdef;

// symbol

typedef enum symboltype
{
	SYM_UNKNOWN,
	SYM_LABEL,
	SYM_OPCODE,
	SYM_MODIFIER,
	SYM_BLOCK,
} symboltype;

typedef struct symbol
{
	union
	{
		line* line;
		opcode* op;
		//?? mod;
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

extern symtabentry* symbols;

// instruction

typedef struct operand operand;

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

	unsigned int leftarg  : 1;
	unsigned int rightarg : 1;
	unsigned int bracket;
} operator;

// operator table

typedef union optabentry
{
	union optabentry* next; // pointer to array of 256 symtabentries
	operator* op;
} optabentry;

extern optabentry* operators;
extern optabentry* preoperators;


// line

typedef struct line
{
	operand* operand;
} line;


// opcode

typedef void (*opcode_func)(symbol* op, line* l);

typedef struct opcode
{
	opcode_func func;
	void* data;
	char* name;
} opcode;


// expression
typedef struct operand
{
	enum {INT, IDENT, REF, STRING, BINOP} tp;
	union
	{
		plong val;
		symbol* ident;
		line* line;
		char* str;
		struct
		{
			struct operand* operands[2];
			operator* op;
		} binop;
	} val;
} operand;


symbol* symbol_get(char* p);
symbol* symbol_define(char* s, symboltype type);


operator* operator_new(char* p, double precedence, int leftarg, int rightarg, int bracket);
blockdef* block_new(char* s);
symbol* label_new(char* s, line* l);
symbol* mod_new(char* s, const char* bits);
opcode* opcode_new(char* s, const char* bits);



operand* int_new(plong x);
operand* ref_new(line* l);
operand* ident_new(char** p);
operand* string_new(char* s);
operand* binop_new(operator* op, operand* lhs, operand* rhs);

void symbol_print(symbol* sym);

void operand_print(operand* this);
