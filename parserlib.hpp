#pragma once

#include <vector>

#include "parallax_types.hpp"

#include "bitfield.hpp"

typedef uint16_t phaddr;
typedef uint16_t pcaddr;

// cog register

/*
class cogreg
{
	pcaddr caddr;

	char* name;
};
*/



template <class T>
union chartree
{
public:
	chartree<T>* next;
	T* curr;
};

class Symbol;
typedef chartree<Symbol> symtabentry;

class Operator;
typedef chartree<Operator> optabentry;

class symscope;

class Grammar
{
public:
	Grammar() {};
	Grammar(symscope* symbols, optabentry* operators, optabentry* preoperators, int haslabels, int hasindent);
	static void push(Grammar* newgrammar);
	static Grammar* pop(void);
	void print(void);
	static void reset(Grammar* grammar);

	symscope* symbols;
	optabentry* operators;
	optabentry* preoperators;

	int haslabels : 1;
	int hasindent : 1;
};

class BlockDef
{
public:
	BlockDef(char* s);

	Grammar* headgrammar;
	Grammar* bodygrammar;

	char* name;
};

class Line;

class Block
{
public:
	Block(std::vector<Block*>* blocks, BlockDef* def, std::vector<Line*>** lines);

	BlockDef* def;
	std::vector<Line*> lines;

	char* name;
};

// Symbol

class Operand;
class Opcode;

enum symboltype
{
	SYM_UNKNOWN,
	SYM_LABEL,
	SYM_OPCODE,
	SYM_BLOCK,
};

class Symbol
{
public:
	Symbol(char* _name) : type(SYM_UNKNOWN), name(_name), defined(0) {}
	union
	{
		Operand* val;
		Opcode* op;
		BlockDef* blockdef;
	} data;
	char* name;
	symboltype type;
	int defined;

	static Symbol* get(char* p);
	static Symbol* get_if_exist(symtabentry* base, char* p);
	static Symbol* define(char* s, symboltype type);

	void print();
};

// Symbol table

class symscope
{
public:
	symtabentry* symtab;
	symscope* parent;

	static void push(symtabentry* symtab);
	static symtabentry* pop(void);
};


// instruction

/*
class instruction
{
	Operand* operands;

	cogreg* reg;

	//struct* instruction next;
};
*/

// Operator
class Operator
{
public:
	Operator(char* p, double precedence, int leftarg, int rightarg);

	double precedence;
	char* name;

	int leftarg;
	int rightarg;

	int push : 1;

	Grammar* localgrammar;
};

// Operator table

extern Grammar* grammar;


// line

class Line
{
public:
	Line() {}

	Operand* operand;
	unsigned int indent;
	unsigned int indentdepth;
	Line* parent;
};


// Opcode

typedef void (*opcode_func)(Symbol* op, Line* l);

class Opcode
{
public:
	Opcode() {}
	opcode_func func;
	void* data;

	bitfield bits;
	char* name;
};


// expression
class Operand
{
public:
	enum {INT, IDENT, PTR, STRING, BINOP} type;
	union
	{
		plong val;
		Symbol* ident;
		char* str;
		struct
		{
			Operand* operands[2];
			Operator* op;
		} binop;
	} val;

	void print();
private:
	void print_list();
};


symtabentry* symtabentry_new(void);
symtabentry* symtabentry_get(symtabentry* base, char* p);


Symbol* label_new(char* s);
Opcode* opcode_new(char* s, const char* bits);
Symbol* modifier_new(char* s, const char* bits);



Operand* int_new(plong x);
Operand* ref_new(Line* l);
Operand* ident_new(char* s);
Operand* ident_new_intern(char** p);
Operand* string_new(char* s);
Operand* binop_new(Operator* op, Operand* lhs, Operand* rhs);

void parserlib_init(void);
