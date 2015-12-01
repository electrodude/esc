#include <cstdlib>
#include <cstdio>
#include <ctype.h>
#include <set>

#include "parserlib.hpp"

#define LIBDEBUG 0


// Symbol table


Grammar* grammar;

symtabentry* usersymbols;

std::vector<BlockDef*> blockdefs;
std::set<Grammar*> blockgrammars;


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

Symbol* Symbol::get(symtabentry* symtab, char* p)
{
	symtabentry* symtab2 = symtabentry::get(symtab, p);

	if (symtab2[0].curr != NULL)
	{
		return symtab2[0].curr;
	}

	Symbol* sym = new Symbol(p);

	symtab2[0].curr = sym;

	return sym;
}

Symbol* Symbol::get(char* p)
{
	Symbol* sym = NULL;

	sym = symtabentry::get_if_exist(usersymbols, p);

	if (sym == NULL)
	{
		sym = symtabentry::get_if_exist(grammar->symbols, p);
	}

	if (sym != NULL)
	{
		return sym;
	}

	sym = new Symbol(p);

	symtabentry* symtab = symtabentry::get(usersymbols, p);

	symtab[0].curr = sym;

	return sym;
}

Symbol* Symbol::define(Grammar* grammar, char* s, symboltype type)
{
	Symbol* sym = Symbol::get(grammar->symbols, s);

	if (sym->defined)
	{
		printf("Symbol %s already defined as %s!\n", s, sym->type == Symbol::LABEL ? "label" : sym->type == Symbol::OPCODE ? "opcode" : "unknown");
		return NULL;
	}

	sym->defined = 1;
	sym->type = type;

	return sym;
}

Symbol* Symbol::define(char* s, symboltype type)
{
	return Symbol::define(grammar, s, type);
}

Symbol* Symbol::chartree_clone(Symbol* sym)
{
	return sym;
}

void Symbol::print() const
{
	printf("(");
	switch (this->type)
	{
		case Symbol::LABEL:
		{
			printf("label ");
			break;
		}
		case Symbol::OPCODE:
		{
			printf("opcode ");
			break;
		}
		case Symbol::BLOCK:
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

	printf("%s", this->name);
	printf(")");
}

Block::Block(std::vector<Block*>* blocks, BlockDef* def, std::vector<Line*>** lines)
{
	this->def = def;

	this->name = def->name;

	blocks->push_back(this);

	*lines = &this->lines;
}

static std::vector<Grammar*> grammarstack;

Grammar::Grammar()
                : symbols(symtabentry::create()), operators(optabentry::create()),
                  haslabels(0), hasindent(0)
{
}

Grammar::Grammar(symtabentry* symbols, optabentry* operators, int haslabels, int hasindent)
                : haslabels(haslabels), hasindent(hasindent)
{
	this->symbols = symbols != NULL ? symbols : symtabentry::create();
	//this->symbols = symbols;
	this->operators = operators != NULL ? operators : optabentry::create();
}

void Grammar::push()
{
	// save old Grammar
	Grammar* oldgrammar = grammar;

	grammarstack.push_back(grammar);


	// make new Grammar if newgrammar == NULL
	//grammar = new Grammar(symtabentry::clone(oldgrammar->symbols),
	grammar = new Grammar(symtabentry::clone(oldgrammar->symbols),
	                       optabentry::clone(oldgrammar->operators),
	                       oldgrammar->haslabels, oldgrammar->hasindent
	                      );

#if LIBDEBUG
	printf("Grammar::push(new %p)\n", grammar);
#endif
}

void Grammar::push(Grammar* newgrammar)
{
	// save old Grammar
	Grammar* oldgrammar = grammar;

	grammarstack.push_back(grammar);


	grammar = newgrammar;

#if LIBDEBUG
	printf("Grammar::push(%p)\n", grammar);
#endif
}

Grammar* Grammar::pop(void)
{
	Grammar* oldgrammar = grammar;

	grammar = grammarstack.back();
	grammarstack.pop_back();

#if LIBDEBUG
	printf("Grammar::pop(%p)\n", grammar);
#endif

	if (grammar == NULL)
	{
		printf("Grammar stack underflow!\n");
		exit(1);
	}

	return oldgrammar;
}

void grammarstack_print(void)
{
	for (std::vector<Grammar*>::iterator it = grammarstack.begin(); it != grammarstack.end(); ++it)
	{
		Grammar* grammar = *it;
		printf("grammar: %p\n", grammar);
	}
}

void Grammar::reset(Grammar* newgrammar)
{
	grammar = newgrammar;

	grammarstack.clear();

	blockdefs.clear();
}

void Grammar::putblocknames()
{
#if LIBDEBUG >= 2
	printf("putblocknames\n");
#endif
	for (std::set<Grammar*>::iterator it = blockgrammars.begin(); it != blockgrammars.end(); ++it)
	{
		Grammar* bodygrammar = *it;
#if LIBDEBUG >= 2
		printf("putblocknames: grammar %p\n", bodygrammar);
#endif

		for (std::vector<BlockDef*>::iterator it = blockdefs.begin(); it != blockdefs.end(); ++it)
		{
			BlockDef* blockdef = *it;
#if LIBDEBUG >= 2
			printf("putblocknames: add %s\n", blockdef->name);
#endif

			Symbol* sym = Symbol::define(bodygrammar, blockdef->name, Symbol::BLOCK);

			sym->data.blockdef = blockdef;
		}
	}
}


BlockDef::BlockDef(char* s)
{
	this->headgrammar = grammar;
	this->bodygrammar = grammar;

	this->name = s;

	blockdefs.push_back(this);
	blockgrammars.insert(this->bodygrammar);
}


bool TokenDesc::matches(const tokentype type) const
{
	if (type == OPERATOR)
	{
		printf("operator\n");
		if (this->acceptsOperator)
		{
			return true;
		}
	}
	else if (type == LITERAL)
	{
		printf("literal\n");
		if (this->acceptsLiteral)
		{
			return true;
		}
	}
	else if (type == SYMBOL)
	{
		printf("symbol\n");
		if (this->acceptsSymbol)
		{
			return true;
		}
	}
	else if (type == OPCODE)
	{
		printf("opcode\n");
		if (this->acceptsOpcode)
		{
			return true;
		}
	}

	return false;
}

// Operator

Operator::Operator(char* _name, double _precedence, int _leftarg, int _rightarg, int _push, Grammar* _localgrammar)
                  : name(_name), leftprecedence(_precedence), rightprecedence(_precedence), localgrammar(_localgrammar), push(_push), tree(true),
                    leftarg(_leftarg), rightarg(_rightarg)
{
	optabentry* optab = optabentry::get(grammar->operators, name);

	OperatorSet* opset = optab[0].curr;

	if (opset == NULL)
	{
		opset = new OperatorSet();

		opset->name = name;

		optab[0].curr = opset;
	}

	if (name[0] == 0)
	{
		lefttypes.acceptsOpcode = true;
		lefttypes.acceptsOperator = true;
		righttypes.acceptsOpcode = true;
	}

	if (!leftarg)
	{
		lefttypes.acceptsLiteral = false;
		lefttypes.acceptsSymbol = false;

		lefttypes.acceptsOperator = true;
		lefttypes.acceptsOpcode = true;
	}

	if (!leftarg && rightarg >= 2) // if opening bracket
	{
		this->tree = false;
	}

	if (rightarg >= 2) // if opening bracket-ish
	{
		this->rightprecedence = 1e9; // lowest precedence - don't get folded
	}


	if (leftarg >= 2 && !rightarg) // if closing bracket
	{
		this->push = false;
	}

	if (leftarg >= 2) // if closing bracket-ish
	{
		this->leftprecedence = 2e9; // lowest precedence - fold everything beneath
	}

	opset->addop(this);

	set = opset;

#if LIBDEBUG
	printf("operator_new \"%s\" (%d, %d, %g, %g)\n", opset->name, leftarg, rightarg, leftprecedence, rightprecedence);
#endif
}

bool Operator::preaccepts(const std::vector<Operand*>* vstack, tokentype prevtokentype) const
{
	if (leftarg && rightarg) // infix binary
	{
		if (!lefttypes.matches(prevtokentype))
		{
			printf("reject: wrong left token\n");
			return false;
		}

		if (vstack->empty())
		{
			printf("reject: vstack empty\n");
			return false;
		}

		const Operand* lhs = vstack->back();

		if (!lhs->matches(lefttypes))
		{
			printf("reject: doesn't match\n");
			return false;
		}

		return true;
	}
	else if (leftarg && !rightarg) // postfix unary
	{
		if (!lefttypes.matches(prevtokentype))
		{
			printf("reject: wrong left token\n");
			return false;
		}

		if (vstack->empty())
		{
			return false;
		}

		const Operand* lhs = vstack->back();

		if (!lhs->matches(lefttypes))
		{
			return false;
		}

		return true;
	}
	else if (!leftarg && rightarg) // prefix unary
	{
		if (!lefttypes.matches(prevtokentype))
		{
			printf("reject: wrong left token\n");
			return false;
		}

		return true;
	}
	else // nullary
	{
		return true;
	}
}

bool Operator::accepts(const std::vector<Operand*>* vstack, const Operator* nextop, tokentype prevtokentype) const
{
	if (leftarg && rightarg) // infix binary
	{
		if (!lefttypes.matches(prevtokentype))
		{
			printf("reject: wrong left token\n");
			return false;
		}

		if (vstack->size() >= 2 && (nextop == NULL || nextop->leftarg))
		{
			const Operand* rhs = vstack->end()[-1];
			const Operand* lhs = vstack->end()[-2];

			printf("Operator::accepts \"%s\" binary: both operands: ", this->name);
			lhs->print();
			printf(", ");
			rhs->print();
			printf("\n");

			if (!lhs->matches(lefttypes))
			{
				return false;
			}

			if (!rhs->matches(righttypes))
			{
				return false;
			}
		}
		else if (vstack->size() == 1)
		{
			// Should never get here!

			const Operand* lhs = vstack->end()[-1];

			printf("Operator::accepts \"%s\" binary: only left operand", this->name);
			lhs->print();
			printf("\n");

			if (nextop == NULL || nextop->leftarg)
			{
				return false;
			}

			if (!lhs->matches(lefttypes))
			{
				return false;
			}
		}

		return true;
	}
	else if (leftarg && !rightarg) // postfix unary
	{
		if (!lefttypes.matches(prevtokentype))
		{
			printf("reject: wrong left token\n");
			return false;
		}

		if (vstack->empty())
		{
			return false;
		}

		const Operand* lhs = vstack->back();

		if (!lhs->matches(lefttypes))
		{
			return false;
		}

		return true;
	}
	else if (!leftarg && rightarg) // prefix unary
	{
		if (!lefttypes.matches(prevtokentype))
		{
			printf("reject: wrong left token\n");
			return false;
		}

		if (vstack->empty())
		{
			if (nextop == NULL || nextop->leftarg)
			{
				return false;
			}

			return true;
		}

		const Operand* rhs = vstack->back();

		if (!rhs->matches(righttypes))
		{
			return false;
		}

		return true;
	}
	else // nullary
	{
		return true;
	}
}


OperatorSet::OperatorSet() : mastercopy(true)
{
}

OperatorSet::OperatorSet(const OperatorSet& original)
	: name(original.name), candidates(original.candidates), selectedop(original.selectedop), mastercopy(original.mastercopy)
{
}

OperatorSet::~OperatorSet()
{
}

OperatorSet* OperatorSet::prepare_push(tokentype _prevtokentype) const
{
	OperatorSet* opset2 = new OperatorSet(*this);

	opset2->mastercopy = false;

	opset2->prevtokentype = _prevtokentype;

	return opset2;
}


void OperatorSet::addop(Operator* op)
{
	candidates.push_back(op);
}

Operator* OperatorSet::preselectop(const std::vector<Operand*>* vstack, tokentype prevtokentype) const
{
	Operator* preselectedop = NULL;

	for (std::vector<Operator*>::const_iterator it = candidates.begin(); it != candidates.end(); ++it)
	{
		Operator* candidate = *it;

		if (candidate->preaccepts(vstack, prevtokentype))
		{
			if (selectedop != NULL)
			{
				if (candidate->leftarg != preselectedop->leftarg)
				{
					printf("multiple candidates for OperatorSet::preselectop: \"%s\" (%d, %d), \"%s\" (%d, %d)\n",
					      preselectedop->name, preselectedop->leftarg, preselectedop->rightarg,
					      candidate->name, candidate->leftarg, candidate->rightarg);
					throw "Multiple candidates!";
				}
			}

			preselectedop = candidate;
		}
	}

	if (preselectedop == NULL)
	{
		printf("Error: preselectop: no operator candidates remain for \"%s\" out of %ld\n", name, candidates.size());
		for (std::vector<Operator*>::const_iterator it = candidates.begin(); it != candidates.end(); ++it)
		{
			const Operator* candidate = *it;

			printf("Rejected candidate: \"%s\" (%d, %d)\n",
				      candidate->name, candidate->leftarg, candidate->rightarg);
		}
		printf("vstack: ");
		for (std::vector<Operand*>::const_iterator it = vstack->begin(); it != vstack->end(); ++it)
		{
			Operand* val = *it;

			val->print();

			printf("\t");
		}
		printf("\n");
		throw "no candidates";
	}

	return preselectedop;
}

Operator* OperatorSet::selectop(const std::vector<Operand*>* vstack, const Operator* nextop)
{
	if (mastercopy)
	{
		printf("error: attempt to call OperatorSet::selectop on master copy of \"%s\"\n", name);
		exit(1);
	}

	if (selectedop != NULL)
	{
		return selectedop;
	}


	for (std::vector<Operator*>::iterator it = candidates.begin(); it != candidates.end(); ++it)
	{
		Operator* candidate = *it;

		printf("OperatorSet::selectop candidate: \"%s\" (%d, %d)\n",
			      candidate->name, candidate->leftarg, candidate->rightarg);

		if (candidate->accepts(vstack, nextop, prevtokentype))
		{
			if (selectedop != NULL)
			{
				printf("multiple candidates for OperatorSet::selectop: \"%s\" (%d, %d), \"%s\" (%d, %d)\n",
				      selectedop->name, selectedop->leftarg, selectedop->rightarg,
				      candidate->name, candidate->leftarg, candidate->rightarg);
				throw "Multiple candidates!";
			}

			selectedop = candidate;
		}
	}

	if (selectedop == NULL)
	{
		printf("Error: selectop: no operator candidates remain for \"%s\" out of %ld\n", name, candidates.size());
		for (std::vector<Operator*>::iterator it = candidates.begin(); it != candidates.end(); ++it)
		{
			Operator* candidate = *it;
			printf("Rejected candidate: \"%s\" (%d, %d)\n",
				      candidate->name, candidate->leftarg, candidate->rightarg);
		}
		printf("vstack: ");
		for (std::vector<Operand*>::const_iterator it = vstack->begin(); it != vstack->end(); ++it)
		{
			Operand* val = *it;

			val->print();

			printf("\t");
		}
		printf("\n");
		throw "no candidates";
	}

	return selectedop;
}

OperatorSet* OperatorSet::chartree_clone(OperatorSet* opset)
{
	if (opset == NULL)
	{
		return NULL;
	}

	return new OperatorSet(*opset);
}


Symbol* label_new(char* s)
{
	Symbol* sym = Symbol::define(s, Symbol::LABEL);

	if (sym == NULL) return NULL;

	return sym;
}

Opcode* opcode_new(char* s, const char* bits)
{
	Symbol* sym = Symbol::define(s, Symbol::OPCODE);

	if (sym == NULL) return NULL;

	Opcode* op = new Opcode();

	op->bits = bitfield(bits);
	op->name = s;

	sym->data.op = op;

	return op;
}

Symbol* modifier_new(char* s, const char* bits)
{
	Symbol* sym = Symbol::define(s, Symbol::OPCODE);

	if (sym == NULL) return NULL;

	Opcode* op = new Opcode();

	op->func = NULL;
	op->data = NULL;
	op->bits = bitfield(bits);
	op->name = s;

	sym->data.op = op;

	return sym;
}


Symbol* ident_new(char* s)
{
	return Symbol::get(s);
}

Symbol* ident_new_intern(char** p)
{
	Symbol* sym = ident_new(*p);

	char* s = sym->name;

	// TODO: if one of s or p is the definition, throw out the other one
	//  (how?)

	if (s != *p)
	{
		char* p_old = *p;

		*p = s;

		free(p_old);
	}

	return sym;
}

OperandBinop::OperandBinop(Operator* _op, Operand* lhs, Operand* rhs) : op(_op)
{
	operands.push_back(lhs);
	operands.push_back(rhs);
}

/*
int Operand::eval(Operand* this)
{
	if (this == NULL)
	{
		return 0;
	}

#if LIBDEBUG >= 2
	printf("Operand::eval: ");
#endif

	switch (this->type)
	{
		case Operand::INT:
		{
#if LIBDEBUG >= 2
			printf("int: %d\n", this->val.val);
#endif
			return this->val.val;
		}
		case Operand::IDENT:
		{
#if LIBDEBUG >= 2
			printf("label: %s\n", this->val.ident);
#endif
			return this->val.ident->data.reg->caddr;
		}
		case Operand::BINOP:
		{
#if LIBDEBUG >= 2
			printf("binop: %c\n", this->val.binop.op);
#endif
			int lhs = Operand::eval(this->val.binop.operands[0]);
			int rhs = Operand::eval(this->val.binop.operands[1]);
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
	printf("Error: unknown value type: %d\n", this->type);
	exit(1);
}
*/

void Operand::print_list() const
{
	print();
}

void OperandBinop::print_list() const
{
	if (op->name[0] != 0)
	{
		this->print();
		return;
	}

	operands[0]->print_list();

	printf(" ");

	operands[1]->print_list();
}

void OperandBinop::print() const
{
	if (op != NULL && op->name[0] == 0)
	{
		printf("[");
		this->print_list();
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

		if (operands[0] != NULL)
		{
			printf(" ");
			operands[0]->print();
		}
		if (operands[1] != NULL)
		{
			printf(" ");
			operands[1]->print();
		}

		printf(")");
	}
}

void OperandInt::print() const
{
	printf("%u", val);
}

void OperandString::print() const
{
	printf("\"%s\"", str);
}

/*
void Operand::kill(Operand* this)
{
	if (this == NULL)
	{
		return;
	}

	switch (this->type)
	{
		case Operand::INT:
		{
			break;
		}
		case Operand::IDENT:
		{
			free(this->val.ident);
			break;
		}
		case Operand::BINOP:
		{
			Operand::kill(this->val.binop.operands[0]);
			Operand::kill(this->val.binop.operands[1]);
			break;
		}
		default:
		{
			printf("Error: unknown value type: %d\n", this->type);
			//exit(1); // who cares about memory leaks? :)
			break;
		}
	}

	free(this);
}
*/

void parserlib_init(void)
{
	grammar = new Grammar();

	usersymbols = symtabentry::create();
}


void CompilerError::print() const
{
	printf("Compiler error at %ld:%d: %s\n", -1L, -1, msg.c_str());
}
