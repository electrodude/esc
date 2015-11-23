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
	static chartree<T>* create();
	static chartree<T>* clone(chartree<T>* optab);
	static chartree<T>* get(chartree<T>* base, char* p);
	static T* get_if_exist(chartree<T>* base, char* p);

	chartree<T>* next;
	T* curr;

private:
	chartree() : next(NULL) {}
};

class Symbol;
typedef chartree<Symbol> symtabentry;

class Operator;
typedef chartree<Operator> optabentry;

class Grammar
{
public:
	Grammar();
	Grammar(symtabentry* symbols, optabentry* operators = NULL, optabentry* preoperators = NULL, int haslabels = 0, int hasindent = 0);
	static void push();
	static void push(Grammar* newgrammar);
	static Grammar* pop(void);
	void print(void);
	static void reset(Grammar* grammar);

	symtabentry* symbols;
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
	static Symbol* get(symtabentry* symtab, char* p);
	static Symbol* define(char* s, symboltype type);

	void print();
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
	Operator(char* name, double precedence, int leftarg, int rightarg, int push = 1, Grammar* localgrammar = NULL);
	Operator(char* name, double precedence, int leftarg, int rightarg, Grammar* localgrammar)
	{
		Operator(name, precedence, leftarg, rightarg, 1, localgrammar);
	}

	static Operator* get(optabentry* optab, char* name);

	int alias(optabentry* optab, char* p, int overwriteif);

	char* name;
	double precedence;

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
