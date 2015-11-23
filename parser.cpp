#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <limits.h>
#include <ctype.h>

#include <math.h>

#include "parser.hpp"

#define PARSERDEBUG 4


int tabwidth = 8;

static std::vector<Operand*>* vstack;
static std::vector<Operator*>* ostack;

static char* tok2stra(char* base, char* start, char* end)
{
	size_t baselen = strlen(base);

	char* s = (char*)malloc(baselen + end - start + 1);

	strncpy(s, base, baselen);

	strncpy(&s[baselen], start, end - start);

	s[baselen + end-start] = 0;

	return s;
}

static char* tok2str(char* start, char* end)
{
	char* s = (char*)malloc(end - start + 1);

	strncpy(s, start, end - start);

	s[end-start] = 0;

	return s;
}

void printstacks(void)
{
	printf("vstack: ");
	for (std::vector<Operand*>::iterator it = vstack->begin(); it != vstack->end(); ++it)
	{
		Operand* val = *it;

		val->print();

		printf("\t");
	}
	printf("\n");
	printf("ostack: ");
	for (std::vector<Operator*>::iterator it = ostack->begin(); it != ostack->end(); ++it)
	{
		Operator* op = *it;

		printf("\"%s\" ", op->name);
	}
	printf("\n");
}

static void fold(Operator* nextop)
{
#if PARSERDEBUG >= 3
	if (nextop != NULL)
	{
		printf("fold: nextop = \"%s\" (%d, %d)\n", nextop->name, nextop->leftarg, nextop->rightarg);
	}
	else
	{
		printf("fold: nextop = NULL\n");
	}
#endif

	if (nextop != NULL && nextop->localgrammar != NULL)
	{
#if PARSERDEBUG >= 2
		printf("op grammar push\n");
#endif
		Grammar::push(nextop->localgrammar);
	}

	if (nextop != NULL && nextop->leftarg == 0 && nextop->rightarg >= 2)
	{
		// don't fold if left bracket

#if PARSERDEBUG >= 3
		printf("fold: left bracket, no fold\n");
#endif

		// BAD: duplicate of the end of this function

		goto folddone;
	}

	Operator* topop;

	while (!ostack->empty() // stop if stack is empty
	       && (topop = ostack->back(),
                   (nextop == NULL // continue all the way if nextop == NULL
		    || (nextop->leftarg >= 2 || (topop->rightarg < 2 && nextop->precedence > topop->precedence)) // stop if right bracket or failed precedence test
		   ))
	        //&& ((nextop != NULL && topop->rightarg == nextop->leftarg) || topop->rightarg == 0) // unless tos is left bracket, stop if mat
	      )
	{
		topop = ostack->back();ostack->pop_back();
#if PARSERDEBUG >= 3
		printf("fold: op \"%s\" (%d, %d)\n", topop->name, topop->leftarg, topop->rightarg);
#endif

		if (topop != NULL && nextop != NULL && topop->leftarg == 0 && topop->rightarg >= 2 && nextop->leftarg >= 2 && nextop->rightarg == 0)
		{
#if PARSERDEBUG >= 3
			printf("fold bracket break\n");
#endif
			goto folded;
		}

		Operand* rhs = NULL;
		if (topop->rightarg && (topop->rightarg >= 0 || vstack->size() >= 2))
		{
			if (vstack->empty())
			{
				printf("Missing rhs of \"%s\"!\n", topop->name);
#if PARSERDEBUG
				printstacks();
#endif
				exit(1);
			}

			rhs = vstack->back();vstack->pop_back();
			if (rhs == NULL)
			{
				printf("Error: rhs of '%s' (%d, %d) == NULL!\n", topop->name, topop->leftarg, topop->rightarg);
				exit(1);
			}
		}


		Operand* lhs = NULL;
		if (topop->leftarg)
		{
			if (vstack->empty())
			{
				printf("Missing lhs of \"%s\"!\n", topop->name);
#if PARSERDEBUG
				printstacks();
#endif
				exit(1);
			}

			Operand* lhs_peek = vstack->back();
			if (0 && lhs_peek != NULL && lhs_peek->type == Operand::IDENT && (lhs_peek->val.ident->type != SYM_LABEL && lhs_peek->val.ident->type != SYM_UNKNOWN))
			{
				// deal with cases like "jmp #label"
				ostack->push_back(grammar->operators[0].curr);
			}
			else
			{
				lhs = vstack->back();vstack->pop_back();
				if (lhs == NULL && topop->rightarg >= 0)
				{
					printf("Error: lhs of '%s' (%d, %d) == NULL!\n", topop->name, topop->leftarg, topop->rightarg);
					exit(1);
				}
			}
		}

		if (topop->localgrammar != NULL)
		{
#if PARSERDEBUG >= 2
			printf("op grammar pop\n");
#endif
			Grammar* oldgrammar = Grammar::pop();

			if (oldgrammar != topop->localgrammar)
			{
				printf("Mismatched grammar pop! %p, %p, %p\n", oldgrammar, topop->localgrammar, nextop != NULL ? nextop->localgrammar : NULL);
				exit(1);
			}
		}

		vstack->push_back(binop_new(topop, lhs, rhs));


		if (topop != NULL && nextop != NULL && nextop->leftarg >= 2 && topop->rightarg == nextop->leftarg)
		{
#if PARSERDEBUG >= 3
			printf("fold function break\n");
#endif
			goto folded;
		}
	}

#if PARSERDEBUG >= 3
	printf("fold: done\n");
#endif

	if (!ostack->empty())
	{
		Operator* topop = ostack->back();

		if (nextop != NULL && topop->rightarg >= 2 && nextop->leftarg >= 2 && topop->rightarg != nextop->leftarg)
		{
			printf("Error: mismatched brackets: %d, %d\n", topop->rightarg, nextop->leftarg);
			exit(1);
		}
	}

folded:

	if (!ostack->empty())
	{
		Operator* topop = ostack->back();

		if (nextop == NULL && topop->rightarg >= 2)
		{
			printf("Error: unmatched left bracket\n");
			exit(1);
		}
	}

	if (ostack->empty() && nextop && NULL && nextop->leftarg >= 2)
	{
		printf("Error: unmatched right bracket! nextop = ");
		if (nextop != NULL)
		{
			printf("\"%s\" (%d, %d)", nextop->name, nextop->leftarg, nextop->rightarg);
		}
		else
		{
			printf("NULL");
		}
		exit(1);
	}

folddone:
	if (nextop != NULL && nextop->push)
	{
#if PARSERDEBUG >= 3
		printf("fold: push\n");
#endif
		ostack->push_back(nextop);
	}

#if PARSERDEBUG >= 3
	printstacks();
#endif
}

static void push_null_operator()
{
#if PARSERDEBUG >= 4
	printf("push_null_operator\n");
#endif
	fold(grammar->operators[0].curr);
}

static unsigned int lineno = 0;

static void eatdocblockcomment(char** pp)
{
	char* p = *pp;

	while (*p)
	{
		if (*p++ == '}')
		{
			if (*p == '}')
			{
				p++;
				*pp = p;
				return;
			}
		}

		if (*p == '\n')
		{
			lineno++;
		}
	}
}

static void eatblockcomment(char** pp)
{
#if PARSERDEBUG >= 3
	printf("block comment on line %d: ", lineno);
#endif

	int level = 0;

	char* p = *pp;

	while (*p && (level > 0 || *p == '{'))
	{
		switch(*p++)
		{
			case 0   : goto end;
			case '{' :
			{
				if (*p == '{')
				{
					eatdocblockcomment(&p);
				}
				else
				{
					level++;
				}
				break;
			}
			case '}' : --level; break;
			case '\n': lineno++;
		}
	}

end:
#if PARSERDEBUG >= 3
	printf("ate %ld chars\n", p - *pp);
#endif

	*pp = p;
}

static char islabelchar[256];

std::vector<Block*>* parser(char* p)
{
	char* ts = p;

	optabentry* currop;

	int pc = 5;

	lineno = 1;
	char* linestart = p;

#if PARSERDEBUG >= 3
	printf("Line %d\n", lineno);
#endif

	char* lastgloballabel = "!begin";


	BlockDef* currblockdef = Symbol::get("con")->data.blockdef;


#if PARSERDEBUG >= 3
	printf("bodygrammar\n");
#endif
	grammar = currblockdef->bodygrammar; // should this be headgrammar instead?


	std::vector<Block*>* blocks = new std::vector<Block*>();

	std::vector<Line*>* lines;

	new Block(blocks, currblockdef, &lines);

	Line* currline = new Line();
	Line* prevline = NULL;

	unsigned int indent = 0;

	std::vector<Line*> indentstack;

	goto line;

newline:
#if PARSERDEBUG >= 3
	printf("\\n\n");
#endif

#if PARSERDEBUG >= 3
	printf("bodygrammar\n");
#endif
	grammar = currblockdef->bodygrammar;

	if (!vstack->empty())
	{
		currline->operand = vstack->back();vstack->pop_back();

		if (grammar->hasindent)
		{
			if (prevline != NULL && indent > prevline->indent)
			{
#if PARSERDEBUG
				printf("indent push: %d %d\n", prevline->indentdepth, prevline->indent);
#endif
				indentstack.push_back(prevline);
			}

			while (!indentstack.empty() && indentstack.back()->indent >= indent)
			{
				Line* oldindentparent = indentstack.back();indentstack.pop_back();
#if PARSERDEBUG
				printf("indent pop: %d %d\n", oldindentparent->indentdepth, oldindentparent->indent);
#endif
			}

			currline->indent = indent;
			currline->parent = indentstack.back();
			currline->indentdepth = indentstack.empty() == false ? indentstack.back()->indentdepth+1 : 0;
#if PARSERDEBUG
			printf("%d %d ", currline->indentdepth, currline->indent);
			// no newline, continued at currline->operand->print()
#endif
		}
		else
		{
			currline->indent = 0;
			currline->parent = NULL;
			currline->indentdepth = 0;
		}

#if PARSERDEBUG
		currline->operand->print();
		printf("\n");
#endif

		lines->push_back(currline);

		prevline = currline;
		currline = new Line();
	}

	lineno++;
	linestart = p;

#if PARSERDEBUG >= 3
	printf("Line %d\n", lineno);
#endif

	indent = 0;

	goto line;

line:
#if PARSERDEBUG >= 4
	//printf("line\n", *p);
	printf("line: '%c'\n", *p);
#endif
	switch (*p)
	{
		case 0   : goto end;
		case '\'': p++; goto comment;
		case '{' : eatblockcomment(&p); goto line;
		case '\n':
		case '\r': indent = 0; lineno++; linestart=p+1; p++;
#if PARSERDEBUG >= 3
		           printf("bodygrammar\n");
#endif
		           grammar = currblockdef->bodygrammar;
		           goto line;

		case ' ' : indent++;         p++; goto line;
		case '\t': indent+=tabwidth; p++; goto line;
	}

	goto expr_entry;

comment:
#if PARSERDEBUG >= 5
	printf("comment: '%c'\n", *p);
#endif
	switch (*p)
	{
		case 0   : goto end;
		case '\n':
		case '\r': p++; goto newline;
	}

	p++; goto comment;

expr_entry:
#if PARSERDEBUG
	//printf("opcode\n");
#endif
	{
		if (!ostack->empty())
		{
			Operator* stray = ostack->back();
			printf("Error at %d:%ld: stray operator on operator stack: %s\n", lineno, p-linestart, stray->name);
			goto error;
		}
	}
	{
		if (!vstack->empty())
		{
			Operand* stray = vstack->back();
			printf("Error at %d:%ld: stray value on value stack: ", lineno, p-linestart);
			stray->print();
			printf("\n");
			goto error;
		}
	}

expr:
#if PARSERDEBUG >= 3
	printf("expr: '%c'\n", *p);
#endif
	currop = grammar->preoperators;

	switch (*p)
	{
		case 0   : goto end;
		case ' ' :
		case '\t': p++; goto expr;
		case '{' : eatblockcomment(&p); goto expr;
		case '"' : goto string;
		case '%' : p++; goto binnum;
		case '$' : p++; goto here_or_hex;
		case '\'': fold(NULL); goto comment;
		case '\n':
		case '\r': fold(NULL); p++; goto newline;
	}

	if (*p >= '0' && *p <= '9') goto decnum;
	if (islabelchar[*p] >= 3 || (grammar->haslabels && islabelchar[*p] == 1)) goto ident;

	if ((*p == '\n' || *p == '\r') && vstack->empty())
	{
		goto newline;
	}

	currop = currop[tolower(*p)].next;


	if (currop != NULL)
	{
		p++;

		while (*p && currop[tolower(*p)].next != NULL)
		{
#if PARSERDEBUG >= 4
			printf("prefix operator char: '%c'\n", *p);
#endif
			currop = currop[tolower(*p)].next;

			p++;
		}

		Operator* op = currop[0].curr;

		if (op == NULL)
		{
			goto error;
		}


#if PARSERDEBUG >= 3
		printf("prefix op operator: \"%s\"\n", op->name);
#endif
		fold(op);

		goto expr;
	}

	goto error;

ident:
	ts = p;
ident_l:
	{
		while (*p && (islabelchar[*p] >= (grammar->haslabels ? 1 : 2)))
		{
#if PARSERDEBUG >= 4
			printf("ident: '%c'\n", *p);
#endif

			if (currop != NULL)
			{
				currop = currop[tolower(*p)].next;
			}

			p++;
		}

		if (currop != NULL)
		{
#if PARSERDEBUG >= 4
			printf("ident: op\n");
#endif
			char* p2 = p;
			while (*p2 && currop[tolower(*p)].next != NULL)
			{
#if PARSERDEBUG >= 4
				printf("prefix ident operator char: '%c'\n", *p2);
#endif
				currop = currop[tolower(*p)].next;

				p2++;
			}

			Operator* op = currop[0].curr;

			if (op != NULL)
			{
#if PARSERDEBUG >= 3
				printf("prefix ident operator: \"%s\"\n", op->name);
#endif
				fold(op);

				p = p2;

				if (op->rightarg)
				{
					goto expr;
				}
				else
				{
					goto op;
				}
			}
		}

		char* base = "";

		if (*ts == ':')
		{
			base = lastgloballabel;
		}

		char* s = tok2stra(base, ts, p);

#if PARSERDEBUG >= 3
		printf("ident: \"%s\"\n", s);
#endif
		Operand* val = ident_new_intern(&s);

		Symbol* sym = val->val.ident;

		if (vstack->empty())
		{
			if (grammar->haslabels)
			{
				// DAT blocks have local labels, which we need to scope now
				if (sym->type == SYM_UNKNOWN)
				{
					// *ts as opposed to *s since s has lastgloballabel prepended to it
					if (*ts != ':' && *ts != '.')
					{
						lastgloballabel = s;
					}

					sym->type = SYM_LABEL;
				}
				else if (sym->type == SYM_LABEL)
				{
					printf("Duplicate symbol \"%s\" at %d:%ld\n", s, lineno, p-linestart);
					//goto error;
				}
			}

			if (sym->type == SYM_BLOCK)
			{
#if PARSERDEBUG
				printf("\nBlock \"%s\"\n", s);
#endif

				currblockdef = sym->data.blockdef;

				new Block(blocks, currblockdef, &lines);

#if PARSERDEBUG >= 3
				printf("headgrammar\n");
#endif
				grammar = currblockdef->headgrammar;

				indentstack.clear();

				goto line;
			}
		}
		else
		{
			if (sym->type == SYM_BLOCK)
			{
				printf("Error: block name not first token on line!\n");

				goto error;
			}
		}

		if (sym->type == SYM_OPCODE)
		{
			push_null_operator();
		}

		vstack->push_back(val);


		if (sym->type == SYM_OPCODE)
		{
#if PARSERDEBUG >= 3
			printf("ident: goto expr\n");
#endif
			goto expr;
		}
		else
		{
#if PARSERDEBUG >= 3
			printf("ident: goto op\n");
#endif
			goto op;
		}

	}


string:
	p++;

	ts = p;

string_mid:
#if PARSERDEBUG >= 3
	printf("string: '%c'\n", *p);
#endif

	switch (*p)
	{
		case 0   : goto error;
		case '"' : break;
		default  : p++; goto string_mid;
	}

	vstack->push_back(string_new(tok2str(ts, p)));
	p++;

	goto op;



	int nbase;
decnum:
	nbase = 10;
	goto num;

hexnum:
	nbase = 16;
	goto num;

binnum:
	if (*p == '%')
	{
		nbase = 4;
		p++;
	}
	else
	{
		nbase = 2;
	}

	goto num;

	plong n;
num:
	ts = p;

	n = 0;

	while (1)
	{
#if PARSERDEBUG >= 4
		printf("num: '%c'\n", *p);
#endif
		switch (*p)
		{
			case 0   : goto error;
		}

		int digit = 0;
		if (*p >= '0' && *p <= '9')
		{
			digit = *p - '0';
		}
		else if (*p >= 'A' && *p <= 'Z')
		{
			digit = *p + 10 - 'A';
		}
		else if (*p >= 'a' && *p <= 'z')
		{
			digit = *p + 10 - 'z';
		}
		else if (*p == '_')
		{
			// eat underscores
		}
		else if (*p == '.' && /* check for operator starting with . */
				      (grammar->operators['.'].next == NULL || grammar->operators['.'].next[tolower(p[1])].next == NULL))
		{
			p++;
			goto num_float;
		}
		else if (nbase < 0xE && *p == 'e')
		{
			goto num_float;
		}
		else
		{
			goto num_done;
		}

		n = n * nbase + digit;
		p++;
	}

num_float:
	{
		float nf = n;
		float exponent = 1/(float)nbase;

		while (1)
		{
#if PARSERDEBUG >= 4
			printf("num_float: '%c'\n", *p);
#endif
			switch (*p)
			{
				case 0   : goto error;
			}

			int digit = 0;
			if (*p >= '0' && *p <= '9')
			{
				digit = *p - '0';
			}
			else if (nbase < 0xE && (*p == 'e' || *p == 'E'))
			{
				p++;
				goto num_float_e;
			}
			else if (*p >= 'A' && *p <= 'Z')
			{
				digit = *p + 10 - 'A';
			}
			else if (*p >= 'a' && *p <= 'z')
			{
				digit = *p + 10 - 'z';
			}
			else if (*p == '_')
			{
				// eat underscores
			}
			else
			{
				goto num_float_done;
			}

			nf += digit * exponent;
			exponent /= nbase;
			p++;
		}

num_float_e:
		{
			int sign = 1;

			int e = 0;

			while (1)
			{
#if PARSERDEBUG >= 4
				printf("num_float_e: '%c'\n", *p);
#endif
				switch (*p)
				{
					case 0   : goto error;
				}

				int digit = 0;
				if (*p >= '0' && *p <= '9')
				{
					digit = *p - '0';
				}
				else if (*p >= 'A' && *p <= 'Z')
				{
					digit = *p + 10 - 'A';
				}
				else if (*p >= 'a' && *p <= 'z')
				{
					digit = *p + 10 - 'z';
				}
				else if (*p == '_')
				{
					// eat underscores
				}
				else
				{
					nf *= powf(nbase, e);
					break;
				}

				e = e * nbase + digit;
				p++;
			}
		}

num_float_done:

#if PARSERDEBUG >= 3
		printf("num_float: %g\n", nf);
#endif
		n = *(plong*)&nf;

	}

num_done:

#if PARSERDEBUG >= 3
	printf("num: %d\n", n);
#endif
	vstack->push_back(int_new(n));

	goto op;

here_or_hex:
#if PARSERDEBUG >= 3
	printf("here or hex: '%c'\n", *p);
#endif
	ts = p;

	if (isxdigit(*p) || *p == '_') goto hexnum;

	vstack->push_back(ident_new("$"));
	goto op;

op:
	{
		currop = grammar->operators;

#if PARSERDEBUG >= 4
		printf("operator char: '%c'\n", *p);
#endif
		switch (*p)
		{
			case 0   : goto end;
			case ' ' :
			case '\t': p++; goto op;
			case '\n':
			case '\r': fold(NULL); p++; goto newline;
			case '\'': fold(NULL); p++; goto comment;
			case '{' : eatblockcomment(&p); goto op;
		}

		ts = p;

		Operator* lastop = NULL;
		char* lastop_p = p+1;

	operator_mid:

		while (*p && currop[tolower(*p)].next != NULL)
		{
#if PARSERDEBUG >= 4
			printf("operator char: '%c'\n", *p);
#endif
			if (currop[0].curr != NULL)
			{
#if PARSERDEBUG >= 3
				printf("operator potential end: '%c'\n", *p);
#endif
				lastop = currop[0].curr;
				lastop_p = p+1;
			}

			currop = currop[tolower(*p)].next;

			p++;
		}

		Operator* op = currop[0].curr;

		if (op == NULL)
		{
			p = lastop_p;
			op = lastop;
#if PARSERDEBUG >= 3
			printf("operator: backtracking to '%c', op = \"%s\"\n", *p, op != NULL ? op->name : "NULL");
#endif
		}

		if (op == NULL)
		{
			push_null_operator();
	/*
	// TODO: Running this commented out results in an infinte loop.  Commenting this
	//  out and just leaving the "goto ident" seems to result in some, if not all,
	//  undefined operators getting through and being registered as the null ""
	//  operator.
#if PARSERDEBUG >= 3
			printf("ident->op backtrack\n");
#endif
			p = ts; // backtracking! Blech!
	*/
#if PARSERDEBUG >= 3
			printf("ident->op\n");
#endif
			goto ident;
		}


#if PARSERDEBUG >= 3
		printf("operator: \"%s\"\n", op->name);
#endif

		if (!op->leftarg)
		{
			push_null_operator();
		}

		fold(op);

		if (op->rightarg)
		{
			goto expr;
		}
		else
		{
			goto op;
		}
	}


error:
	printf("Parse error at %d:%ld: '%c' (%02X)\n", lineno, p-linestart, *p, *p);
#if PARSERDEBUG
	printstacks();
#endif

	return NULL;

#if 0
	while (*p != 0 && *p != '\n' && *p != '\r') p++;

	while (!vstack->empty())
	{
		Operand* o = vstack->back();vstack->pop_back();
		printf("dropping Operand ");
		o->print();
		printf("\n");
	}

	while (!ostack->empty())
	{
		operator* o = ostack->back();ostack->pop_back();
		if (o != NULL)
		{
			printf("dropping operator '%s'\n", o->name);
		}
		else
		{
			printf("dropping operator NULL\n");
		}
	}

	printf("continuing on next line\n\n");

	goto line;
#endif

	return blocks;


end:
#if PARSERDEBUG
	printf("EOF\n\n");
#endif

	return blocks;
}


static inline void islabelchar_r(size_t start, size_t end, char c)
{
	while (start <= end)
	{
		islabelchar[start++] = c;
	}
}

void parser_init(void)
{
	islabelchar_r(0,   127,   0);
	islabelchar_r('A', 'Z',   3);
	islabelchar_r('a', 'z',   3);
	islabelchar       ['_'] = 3;
	islabelchar       [':'] = 1;
	//islabelchar       ['.'] = 3;
	islabelchar_r('0', '9',   2);
	islabelchar_r(128, 255,   3); // UTF-8 support

	vstack = new std::vector<Operand*>();
	ostack = new std::vector<Operator*>();

	parserlib_init();
}
