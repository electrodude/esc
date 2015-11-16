#pragma once

#include "parallax_types.h"

typedef uint16_t phaddr;
typedef uint16_t pcaddr;

// cog register

/*
typedef struct cogreg
{
	pcaddr caddr;

	const char* name;
} cogreg;
*/


typedef struct opcode opcode;
typedef struct line line;



// symbol

typedef enum symboltype
{
	SYM_UNKNOWN,
	SYM_LABEL,
	SYM_OPCODE,
	SYM_MODIFIER,
} symboltype;

typedef struct symbol
{
	union
	{
		line* line;
		opcode* op;
		//?? mod;
	} data;
	const char* name;
	symboltype type;
	int defined;
} symbol;

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
	const char* name;
} opcode;


// expression
typedef struct operand
{
	enum {INT, IDENT, REF, BINOP} tp;
	union
	{
		plong val;
		symbol* ident;
		line* line;
		struct
		{
			struct operand* operands[2];
			char op;
		} binop;
	} val;
} operand;


symbol* symbol_get(const char* p);
symbol* symbol_define(const char* s, enum symboltype type);


symbol* label_new(const char* s, line* l);
symbol* mod_new(const char* s, const char* bits);
symbol* opcode_new(const char* s, const char* bits);



operand* int_new(plong x);
operand* ref_new(line* l);
operand* ident_new(char** p);
operand* binop_new(char op, operand* lhs, operand* rhs);

void symbol_print(symbol* sym);

void operand_print(operand* this);
