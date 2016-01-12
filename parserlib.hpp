#pragma once

#include <string>
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

template<class T> chartree<T>* chartree<T>::create(void)
{
	//return calloc(256, sizeof(symtabentry));
	return new chartree<T>[256]();
}

template<class T> chartree<T>* chartree<T>::clone(chartree<T>* optab)
{
	if (optab == NULL)
	{
		return NULL;
	}

	chartree<T>* optab2 = chartree<T>::create();

	optab2[0].curr = T::chartree_clone(optab[0].curr);

	for (int i=1; i < 256; i++)
	{
		optab2[i].next = chartree<T>::clone(optab[i].next);
	}

	return optab2;
}

template<class T> T* chartree<T>::get_if_exist(chartree<T>* base, char* p)
{
	chartree<T>* symtab = base;

	while (*p != 0)
	{
#if LIBDEBUG >= 2
		putchar(*p);
#endif
		unsigned char c = tolower(*p);

		symtab = symtab[c].next;

		if (symtab == NULL)
		{
			return NULL;
		}

		p++;
	}
#if LIBDEBUG >= 2
	putchar('\n');
#endif

	return symtab[0].curr;
}

template<class T> chartree<T>* chartree<T>::get(chartree<T>* base, char* p)
{
	chartree<T>* symtab = base;

	while (*p != 0)
	{
#if LIBDEBUG >= 2
		putchar(*p);
#endif
		unsigned char c = tolower(*p);

		chartree<T>* symtab2 = symtab[c].next;
		if (symtab2 == NULL)
		{
			symtab2 = symtab[c].next = chartree<T>::create();
		}
		symtab = symtab2;
		p++;
	}
#if LIBDEBUG >= 2
	putchar('\n');
#endif

	return symtab;
}

class Symbol;
typedef chartree<Symbol> symtabentry;

class OperatorSet;
typedef chartree<OperatorSet> optabentry;

class Grammar
{
public:
	Grammar();
	Grammar(Grammar* oldgrammar);
	Grammar(symtabentry* symbols, optabentry* operators = NULL, int haslabels = 0, int hasindent = 1);
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
	BlockDef(char* s, Grammar* headgrammar = NULL);

	Grammar* headgrammar;
	Grammar* bodygrammar;

	bool hasdesc;

	char* name;
};

class Line;

#if 0 == 1
class Block
{
public:
	Block(std::vector<Block*>* blocks, BlockDef* def, std::vector<Line*>** lines);

	BlockDef* def;
	std::vector<Line*> lines;

	char* name;

	bool haserrors;
};
#endif


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
enum tokentype
{
	OPERATOR,
	LITERAL,
	SYMBOL,
	OPCODE,
};

class Operand;

class TokenDesc
{
public:
	TokenDesc() : acceptsOperator(false), acceptsSymbol(true), acceptsOpcode(false), acceptsLiteral(true) {}

	virtual bool matches(const tokentype type) const;

	bool acceptsOperator;
	bool acceptsSymbol;

	bool acceptsOpcode;
	bool acceptsLiteral;
};

class Operator
{
public:
	Operator(char* name, double precedence, int leftarg, int rightarg, int push = 1, Grammar* localgrammar = NULL);
	/* BAD: I need to read up on the right way to do this...
	Operator(char* name, double precedence, int leftarg, int rightarg, Grammar* localgrammar)
	{
		Operator(name, precedence, leftarg, rightarg, 1, localgrammar);
	}
	*/

	static Operator* newBlock(char* name, int leftarg, int rightarg, Grammar* localgrammar = NULL, Grammar* indentgrammar = NULL);

	static Operator* get(optabentry* optab, char* name);

	bool preaccepts(const std::vector<Operand*>* vstack, tokentype prevtokentype) const;
	bool accepts(const std::vector<Operand*>* vstack, const Operator* nextop, tokentype prevtokentype) const;

	void print() const;

	char* name;

	double leftprecedence;
	double rightprecedence;

	int leftarg;
	int rightarg;

	TokenDesc lefttypes;
	TokenDesc righttypes;

	Grammar* localgrammar;

	Grammar* indentgrammar;
	BlockDef* blockdef;

	bool push;

	bool tree;

	OperatorSet* set;
};

class OperatorSet
{
public:
	OperatorSet();
	OperatorSet(const OperatorSet& original);
	~OperatorSet();

	OperatorSet* prepare_push(tokentype _prevtokentype) const;

	void addop(Operator* op);

	// preselectop
	// returns a representative operator that has the correct left behavior
	// expects vstack to only have lhs operand
	Operator* preselectop(const std::vector<Operand*>* vstack, tokentype prevtokentype) const;

	// selectop
	// returns exact operator to use based on all operands
	// expects all operands to be on stack
	Operator* selectop(const std::vector<Operand*>* vstack, const Operator* nextop);

	static OperatorSet* chartree_clone(OperatorSet* opset);

	void print() const;

	char* name;

	std::vector<Operator*> candidates;

private:
	Operator* selectedop;

	bool mastercopy;

	tokentype prevtokentype;
};


// operator table

extern Grammar* grammar;


// line

class Line
{
public:
	Line();

	void addchild(Line* child);

	Operand* operand;
	int indent;
	int indentdepth;

	//BlockDef* blockdef;

	Line* parent;

	std::vector<Line*> children;

	bool haserrors;

	void print(unsigned int depth = 0);
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
class OperandBinop;

class Operand
{
public:

	//virtual ??? eval();

	virtual bool matches(const TokenDesc& spec) const { return true; }

	virtual Grammar* getindentgrammar() const { return NULL; };
	virtual BlockDef* getblockdef() const { return NULL; };

	virtual void print() const = 0;
protected:
	virtual void print_list() const;

	friend OperandBinop;
};

class OperandInt : public Operand
{
public:
	OperandInt(plong _val) : val(_val) {}

	plong val;

	virtual bool matches(const TokenDesc& spec) const { return spec.acceptsLiteral; }

	virtual void print() const;
};

class OperandString : public Operand
{
public:
	OperandString(char* _str) : str(_str) {}

	char* str;

	virtual bool matches(const TokenDesc& spec) const { return spec.acceptsLiteral; }

	virtual void print() const;
};

class OperandBinop : public Operand
{
public:
	OperandBinop(Operator* _op, Operand* lhs, Operand* rhs);

	std::vector<Operand*> operands;
	Operator* op;

	virtual Grammar* getindentgrammar() const;
	virtual BlockDef* getblockdef() const;

	virtual void print() const;
protected:
	virtual void print_list() const;
};

// symbol

class Opcode;

class Symbol : public Operand
{
public:
	enum symboltype
	{
		UNKNOWN,
		LABEL,
		OPCODE,
	};

	Symbol(char* _name) : type(Symbol::UNKNOWN), name(_name), defined(0) {}
	union
	{
		Opcode* op;
	} data;
	char* name;
	symboltype type;
	int defined;

	static Symbol* get_if_exist(char* p);
	static Symbol* get(char* p);
	static Symbol* get(symtabentry* symtab, char* p);
	static Symbol* define(Grammar* grammar, char* s, symboltype type);
	static Symbol* define(char* s, symboltype type);

	virtual bool matches(const TokenDesc& spec) const;


	static Symbol* chartree_clone(Symbol* sym);


	void print() const;
};

symtabentry* symtabentry_new(void);
symtabentry* symtabentry_get(symtabentry* base, char* p);


Symbol* label_new(char* s);
Opcode* opcode_new(char* s, const char* bits);
Symbol* modifier_new(char* s, const char* bits);



Operand* ref_new(Line* l);
Symbol* ident_new(char* s);
Symbol* ident_new_intern(char** p);

void parserlib_init(void);

class CompilerError
{
public:
	void print() const;

	unsigned int line;
	unsigned int column;

	std::string msg;
};
