#include <cstdlib>
#include <cstdio>
#include <ctype.h>

#include "parserlib.hpp"

#define LIBDEBUG 0


// Symbol table


Grammar* grammar;

symtabentry* usersymbols;


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

	optab2[0].curr = optab[0].curr;

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

Symbol* Symbol::define(char* s, symboltype type)
{
	Symbol* sym = Symbol::get(grammar->symbols, s);

	if (sym->defined)
	{
		printf("Symbol %s already defined as %s!\n", s, sym->type == SYM_LABEL ? "label" : sym->type == SYM_OPCODE ? "opcode" : "unknown");
		return NULL;
	}

	sym->defined = 1;
	sym->type = type;

	return sym;
}

void Symbol::print()
{
	switch (this->type)
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

	printf("%s", this->name);
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
                : symbols(symtabentry::create()), operators(optabentry::create()), preoperators(optabentry::create()),
                  haslabels(0), hasindent(0)
{
}

Grammar::Grammar(symtabentry* symbols, optabentry* operators, optabentry* preoperators, int haslabels, int hasindent)
                : haslabels(haslabels), hasindent(hasindent)
{
	this->symbols = symbols != NULL ? symbols : symtabentry::create();
	//this->symbols = symbols;
	this->operators = operators != NULL ? operators : optabentry::create();
	this->preoperators = preoperators != NULL ? preoperators : optabentry::create();
}

void Grammar::push()
{
	// save old Grammar
	Grammar* oldgrammar = grammar;

	grammarstack.push_back(grammar);


	// make new Grammar if newgrammar == NULL
	grammar = new Grammar(symtabentry::clone(oldgrammar->symbols),
	                       optabentry::clone(oldgrammar->operators),
	                       optabentry::clone(oldgrammar->preoperators),
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
}


BlockDef::BlockDef(char* s)
{
	Symbol* sym = Symbol::define(s, SYM_BLOCK);

	if (sym == NULL) throw 0;

	this->headgrammar = grammar;
	this->bodygrammar = grammar;

	this->name = s;

	sym->data.blockdef = this;
}


// Operator

int Operator::alias(optabentry* optab, char* p, int overwriteif)
{
	const char* s2 = p;

#if 1
	optab = optabentry::get(optab, p);
#else
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
			optab2 = optab[c].next = optabentry::create();
		}
		optab = optab2;
		p++;
	}
#if LIBDEBUG >= 2
	putchar('\n');
#endif
#endif
	// if there's already something here, and both are actually supposed to
	//  have left arguments (this is the left argument table), then complain
	if (optab[0].curr != NULL)
	{
		Operator* op2 = optab[0].curr;

#if LIBDEBUG
		printf("Operator \"%s\" already defined: \"%s\" (%d, %d): ", s2, op2->name, op2->leftarg, op2->rightarg);
#endif

		if (!overwriteif || (this->leftarg && op2->leftarg))
		{
#if LIBDEBUG
			printf("erroring\n");
#endif
			return 1;
		}
		else if (this->leftarg)
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

	optab[0].curr = this;

	return 0;
}

Operator::Operator(char* _name, double _precedence, int _leftarg, int _rightarg, int _push, Grammar* _localgrammar)
                  : name(_name), precedence(_precedence), leftarg(_leftarg), rightarg(_rightarg), push(_push), localgrammar(_localgrammar)
{
#if LIBDEBUG
	printf("operator_new \"%s\" (%d, %d, %g)\n", s, leftarg, rightarg, precedence);
#endif


	if (leftarg == 0)
	{
		if (alias(grammar->preoperators, name, 0))
		{
			printf("prefix operator \"%s\" already defined!\n", name);
			throw 0;
		}
	}

	//if (leftarg >= 0)
	{
		if (alias(grammar->operators, name, 1))
		{
			printf("operator \"%s\" already defined!\n", name);
			throw 0;
		}
	}

#if LIBDEBUG
	printf("\n");
#endif
}


Symbol* label_new(char* s)
{
	Symbol* sym = Symbol::define(s, SYM_LABEL);

	if (sym == NULL) return NULL;

	return sym;
}

Opcode* opcode_new(char* s, const char* bits)
{
	Symbol* sym = Symbol::define(s, SYM_OPCODE);

	if (sym == NULL) return NULL;

	Opcode* op = new Opcode();

	op->bits = bitfield(bits);
	op->name = s;

	sym->data.op = op;

	return op;
}

Symbol* modifier_new(char* s, const char* bits)
{
	Symbol* sym = Symbol::define(s, SYM_OPCODE);

	if (sym == NULL) return NULL;

	Opcode* op = new Opcode();

	op->func = NULL;
	op->data = NULL;
	op->bits = bitfield(bits);
	op->name = s;

	sym->data.op = op;

	return sym;
}


Operand* int_new(plong x)
{
	Operand* operand = new Operand();

	operand->type = Operand::INT;
	operand->val.val = x;

	return operand;
}

Operand* ident_new(char* s)
{
	Operand* operand = new Operand();

	operand->type = Operand::IDENT;
	operand->val.ident = Symbol::get(s);

	return operand;
}

Operand* ident_new_intern(char** p)
{
	Operand* operand = ident_new(*p);

	Symbol* sym = operand->val.ident;

	char* s = sym->name;

	// TODO: if one of s or p is the definition, throw out the other one
	//  (how?)

	if (s != *p)
	{
		char* p_old = *p;

		*p = s;

		free(p_old);
	}

	return operand;
}

Operand* string_new(char* s)
{
	Operand* operand = new Operand();

	operand->type = Operand::STRING;
	operand->val.str = s;

	return operand;
}

Operand* binop_new(Operator* op, Operand* lhs, Operand* rhs)
{
	Operand* operand = new Operand();

	operand->type = Operand::BINOP;
	operand->val.binop.op = op;
	operand->val.binop.operands[0] = lhs;
	operand->val.binop.operands[1] = rhs;

	return operand;
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

void Operand::print_list()
{
	if (this->type != Operand::BINOP || this->val.binop.op == NULL || this->val.binop.op->name[0] != 0)
	{
		this->print();
		return;
	}

	Operand** operands = this->val.binop.operands;

	operands[0]->print_list();

	printf(" ");

	operands[1]->print_list();
}

void Operand::print()
{
	if (this == NULL)
	{
		printf("NULL");
		return;
	}

	switch (this->type)
	{
		case Operand::INT:
		{
			printf("%u", this->val.val);

			break;
		}
		case Operand::IDENT:
		{
			printf("(");
			this->val.ident->print();
			printf(")");

			break;
		}
		case Operand::BINOP:
		{
			Operator* op = this->val.binop.op;
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

				if (this->val.binop.operands[0] != NULL)
				{
					printf(" ");
					this->val.binop.operands[0]->print();
				}
				if (this->val.binop.operands[1] != NULL)
				{
					printf(" ");
					this->val.binop.operands[1]->print();
				}

				printf(")");
			}

			break;
		}
		case Operand::PTR:
		{
			//printf("[%s]", this->val.line->name);
			printf("[ptr]");

			break;
		}
		case Operand::STRING:
		{
			printf("\"%s\"", this->val.str);

			break;
		}
		default:
		{
			printf("(? type=%d ?)", this->type);

			break;
		}
	}
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
