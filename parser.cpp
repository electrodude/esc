#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <limits.h>
#include <ctype.h>

#include <math.h>

#include "parser.hpp"

#define PARSERDEBUG 3
/*
 * Debug levels:
 * 0: none
 * 1: lines
 * 2: folding
 * 3: tokens
 * 4: characters
 * 5: characters and comments
 */

#define INDENTDEBUG 0


int tabwidth = 8;

static std::vector<Operand*>* vstack;
static std::vector<OperatorSet*>* ostack;

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
	for (std::vector<OperatorSet*>::iterator it = ostack->begin(); it != ostack->end(); ++it)
	{
		OperatorSet* op = *it;

		printf("\"%s\" ", op->name);
	}
	printf("\n");
}

static void fold(OperatorSet* nextopset, tokentype prevtokentype)
{
	Operator* nextop = NULL;
	if (nextopset != NULL)
	{
		nextop = nextopset->preselectop(vstack, prevtokentype);
	}

#if PARSERDEBUG >= 3
	if (nextop != NULL)
	{
		printf("fold: nextop = \"%s\" (%d)\n", nextop->name, nextop->leftarg);
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

	while (!ostack->empty())
	{
		OperatorSet* topopset = ostack->back();

		topop = topopset->selectop(vstack, nextop, prevtokentype);

		if (nextop != NULL && !(topop->rightarg < 2 && nextop->precedence > topop->precedence))
		{
			break;
		}

		ostack->pop_back();

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
		if (topop->rightarg)
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

			lhs = vstack->back();vstack->pop_back();

			if (lhs == NULL)
			{
				printf("Error: lhs of '%s' (%d, %d) == NULL!\n", topop->name, topop->leftarg, topop->rightarg);
				exit(1);
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

#if PARSERDEBUG
		printstacks();
#endif
	}

#if PARSERDEBUG >= 3
	printf("fold: done\n");
#endif

	/*
	if (!ostack->empty())
	{
		Operator* topop = ostack->back()->selectop();

		if (nextop != NULL && topop->rightarg >= 2 && nextop->leftarg >= 2 && topop->rightarg != nextop->leftarg)
		{
			printf("Error: mismatched brackets: %d, %d\n", topop->rightarg, nextop->leftarg);
			exit(1);
		}
	}
	*/

folded:

	/*
	if (!ostack->empty())
	{
		Operator* topop = ostack->back()->selectop();

		if (nextop == NULL && topop->rightarg >= 2)
		{
			printf("Error: unmatched left bracket\n");
			exit(1);
		}
	}
	*/

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
		ostack->push_back(nextopset->clone_unmaster());
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
	fold(grammar->operators[0].curr, tokentype::SYMBOL);
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

	tokentype expectnexttoken = SYMBOL;

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
	fold(NULL, expectnexttoken);
#if PARSERDEBUG >= 3
	printf("\\n\n");
#endif

	if (!vstack->empty())
	{
		currline->operand = vstack->back();vstack->pop_back();

		if (grammar->hasindent)
		{
			if (prevline != NULL && indent > prevline->indent)
			{
#if INDENTDEBUG
				printf("indent push: %d %d\n", prevline->indentdepth, prevline->indent);
#endif
				indentstack.push_back(prevline);
			}

			while (!indentstack.empty() && indentstack.back()->indent >= indent)
			{
				Line* oldindentparent = indentstack.back();indentstack.pop_back();
#if INDENTDEBUG
				printf("indent pop: %d %d\n", oldindentparent->indentdepth, oldindentparent->indent);
#endif
			}

			currline->indent = indent;
			currline->parent = indentstack.back();
			currline->indentdepth = indentstack.empty() == false ? indentstack.back()->indentdepth+1 : 0;
#if INDENTDEBUG
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

#if PARSERDEBUG >= 3
	printf("bodygrammar\n");
#endif
	grammar = currblockdef->bodygrammar;

	expectnexttoken = SYMBOL;

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
		while (!ostack->empty())
		{
			OperatorSet* stray = ostack->back();
			printf("Error at %d:%ld: stray operator on operator stack: %s\n", lineno, p-linestart, stray->name);
			//goto error;
			ostack->pop_back();
		}
	}
	{
		while (!vstack->empty())
		{
			Operand* stray = vstack->back();
			printf("Error at %d:%ld: stray value on value stack: ", lineno, p-linestart);
			stray->print();
			printf("\n");
			//goto error;
			vstack->pop_back();
		}
	}

expr:
#if PARSERDEBUG >= 4
	printf("expr: '%c'\n", *p);
#endif

	switch (*p)
	{
		case 0   : goto end;
		case ' ' :
		case '\t': p++; goto expr;
		case '{' : eatblockcomment(&p); goto expr;
		case '"' : goto string;
		case '%' : p++; goto binnum;
		case '$' : p++; goto here_or_hex;
		case '\'': goto comment;
		case '\n':
		case '\r': p++; goto newline;
	}

	if (*p >= '0' && *p <= '9') goto decnum;

	ts = p;

	{
		optabentry* currop = grammar->operators;

		bool islabel = (islabelchar[*p] >= 2 || (grammar->haslabels && islabelchar[*p] == 1));
		bool waslabel = islabel;

		OperatorSet* lastop = currop[0].curr;
		char* lastop_p = p;

#if PARSERDEBUG >= 4
		printf("token: '%c', islabel = %d, currop = %p, lastop = \"%s\"\n", *p, islabel, currop, lastop != NULL ? lastop->name : "NULL");
#endif

		do
		{
			waslabel = islabel;

			if (!(islabelchar[*p] >= 2 || (grammar->haslabels && islabelchar[*p] == 1)))
			{
				islabel = false;
			}

			if (currop != NULL)
			{
				currop = currop[tolower(*p)].next;

				if (currop != NULL && currop[0].curr != NULL)
				{
#if PARSERDEBUG >= 4
					printf("op candidate\n");
#endif
					lastop = currop[0].curr;
					lastop_p = p+1;
				}
			}

			p++;

#if PARSERDEBUG >= 4
			printf("token: '%c', islabel = %d, currop = %p, lastop = \"%s\"\n", *p, islabel, currop, lastop != NULL ? lastop->name : "NULL");
#endif
		}
		while (*p && (islabel || currop != NULL));

		if (*p == 0)
		{
			printf("Unexpected end of file!\n");

			goto error;
		}

		char* s = NULL;
		Symbol* sym = NULL;
		if (waslabel)
		{
			s = tok2str(ts, p-1);

			sym = symtabentry::get_if_exist(grammar->symbols, s);
		}

#if PARSERDEBUG >= 4
		printf("token: \"%s\", islabel = %d, waslabel = %d currop = %p, lastop = \"%s\", sym = \"%s\"\n", s, islabel, waslabel, currop, lastop != NULL ? lastop->name : "NULL", sym != NULL ? sym->name : "NULL");
#endif

		if (sym != NULL)
		{
			printf("sym: ");
			sym->print();
			printf("\n");
		}

		if (lastop != NULL && (currop != NULL || !waslabel))
		{
			printf("op: \"%s\"\n", lastop->name);
			p = lastop_p;

			fold(lastop, expectnexttoken);

			expectnexttoken = SYMBOL;

			free(s);
		}
		else if (waslabel)
		{
			printf("ident: \"%s\"\n", s);

			p--;


			if (expectnexttoken == OPERATOR)
			{
				push_null_operator();
			}

			expectnexttoken = OPERATOR;

			Operand* val = ident_new_intern(&s);

			Symbol* sym = val->val.ident;

			if (vstack->empty())
			{
				if (grammar->haslabels)
				{
					// DAT blocks have local labels, which we need to scope now
					if (sym->type == Symbol::UNKNOWN)
					{
						// *ts as opposed to *s since s has lastgloballabel prepended to it
						if (*ts != ':' && *ts != '.')
						{
							lastgloballabel = s;
						}

						sym->type = Symbol::LABEL;
					}
					else if (sym->type == Symbol::LABEL)
					{
						printf("Duplicate symbol \"%s\" at %d:%ld\n", s, lineno, p-linestart);
						//goto error;
					}
				}

				if (sym->type == Symbol::BLOCK)
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


					expectnexttoken = SYMBOL;

					goto line;
				}
			}
			else
			{
				if (sym->type == Symbol::BLOCK)
				{
					printf("Error: block name not first token on line!\n");

					goto error;
				}
			}

			if (sym->type == Symbol::OPCODE)
			{
				//push_null_operator();
			}

			vstack->push_back(val);
		}
		else
		{
			p--;

			char* s = tok2str(ts, p);
			printf("Unknown token: \"%s\"\n", s);

			free(s);

			exit(1);
		}
	}

	goto expr;


#if 0==1
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
		fold(op, expectnexttoken);

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
				fold(op, expectnexttoken);

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


		/*
		if (sym->type == Symbol::OPCODE)
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
		*/

	}
#endif

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

	goto expr;



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

	if (expectnexttoken == OPERATOR)
	{
		push_null_operator();
	}

	expectnexttoken = OPERATOR;

	vstack->push_back(int_new(n));

	goto expr;

here_or_hex:
#if PARSERDEBUG >= 3
	printf("here or hex: '%c'\n", *p);
#endif
	ts = p;

	if (isxdigit(*p) || *p == '_') goto hexnum;

	vstack->push_back(ident_new("$"));
	goto expr;

#if 0==1
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
			case '\r': fold(NULL, expectnexttoken); p++; goto newline;
			case '\'': fold(NULL, expectnexttoken); p++; goto comment;
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

		fold(op, expectnexttoken);

		if (op->rightarg)
		{
			goto expr;
		}
		else
		{
			goto op;
		}
	}
#endif


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
	islabelchar_r('A', 'Z',   2);
	islabelchar_r('a', 'z',   2);
	islabelchar       ['_'] = 2;
	islabelchar       [':'] = 1;
	//islabelchar       ['.'] = 3;
	islabelchar_r('0', '9',   2);
	islabelchar_r(128, 255,   2); // UTF-8 support

	vstack = new std::vector<Operand*>();
	ostack = new std::vector<OperatorSet*>();

	parserlib_init();
}
