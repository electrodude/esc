#pragma once

#include <string>
#include <vector>
#include <set>

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

class OperatorSet;
typedef chartree<OperatorSet> optabentry;

class Grammar
{
public:
	Grammar();
	Grammar(symtabentry* symbols, optabentry* operators = NULL, int haslabels = 0, int hasindent = 0);
	static void push();
	static void push(Grammar* newgrammar);
	static Grammar* pop(void);
	//void print() const;
	static void reset(Grammar* grammar);

	static void putblocknames();

	symtabentry* symbols;
	optabentry* operators;

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

// symbol

class Operand;
class Opcode;

enum tokentype
{
	OPERATOR,
	SYMBOL,
};

class Symbol
{
public:
	enum symboltype
	{
		UNKNOWN,
		LABEL,
		OPCODE,
		BLOCK,
	};

	Symbol(char* _name) : type(Symbol::UNKNOWN), name(_name), defined(0) {}
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
	static Symbol* define(Grammar* grammar, char* s, symboltype type);
	static Symbol* define(char* s, symboltype type);

	static Symbol* chartree_clone(Symbol* sym);

	void print() const;
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

// operator
class Operator
{
public:
	Operator(char* name, double precedence, int leftarg, int rightarg, int push = 1, Grammar* localgrammar = NULL);
	Operator(char* name, double precedence, int leftarg, int rightarg, Grammar* localgrammar)
	{
		Operator(name, precedence, leftarg, rightarg, 1, localgrammar);
	}

	static Operator* get(optabentry* optab, char* name);

	bool preaccepts(const std::vector<Operand*>* vstack, tokentype prevtokentype) const;
	bool accepts(const std::vector<Operand*>* vstack, const Operator* nextop, tokentype prevtokentype) const;

	char* name;

	double precedence;

	int leftarg;
	int rightarg;

	std::set<operand_type> lefttypes;
	std::set<operand_type> righttypes;

	Grammar* localgrammar;

	int push;

	OperatorSet* set;
};

class OperatorSet
{
public:
	OperatorSet();
	OperatorSet(const OperatorSet& original);
	~OperatorSet();

	OperatorSet* clone_unmaster() const;

	void addop(Operator* op);

	// preselectop
	// returns a representative operator that has the correct left behavior
	// expects vstack to only have lhs operand
	Operator* preselectop(const std::vector<Operand*>* vstack, tokentype prevtokentype) const;

	// selectop
	// returns exact operator to use based on all operands
	// expects all operands to be on stack
	Operator* selectop(const std::vector<Operand*>* vstack, const Operator* nextop, tokentype prevtokentype);

	static OperatorSet* chartree_clone(OperatorSet* opset);

	char* name;

	std::vector<Operator*> candidates;

private:
	Operator* selectedop;

	bool mastercopy;
};


// operator table

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
enum operand_type
{
	VALUE,
	OPCODE,
	OPERATOR,
};

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

	void print() const;
private:
	void print_list() const;
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

class CompilerError
{
public:
	void print() const;

	unsigned int line;
	unsigned int column;

	std::string msg;
};
