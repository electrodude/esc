#include <iostream>
#include <fstream>
#include <streambuf>

#include "parser.hpp"

class TokenLabel : public Token
{
public:
	TokenLabel(std::string::iterator start, std::string::iterator end, Parser* parser) : Token(start, end, parser)
	{
		std::cout << "Label: " << std::string(start, end) << "\n";
	}

	virtual bool isIdent() const { return true; }

};

class TokenLabelFactory : public TokenFactory
{
public:
	virtual Token* factory_new(std::string::iterator start, std::string::iterator end, Parser* parser)
	{
		return new TokenLabel(start, end, parser);
	}
};


class TokenQuote : public Token
{
public:
	TokenQuote(std::string::iterator start, std::string::iterator end, Parser* parser) : Token(start, end, parser)
	{
		std::cout << "Quote: " << std::string(start, end) << "\n";
	}

	virtual bool isIdent() const { return true; }

};

class TokenQuoteFactory : public TokenFactory
{
public:
	virtual Token* factory_new(std::string::iterator start, std::string::iterator end, Parser* parser)
	{
		return new TokenQuote(start, end-1, parser);
	}
};



int main(int argc, char** argv)
{
	std::cout << "Parser test...\n";

	TokenFactory* labelfactory = new TokenLabelFactory();
	TokenFactory* quotefactory = new TokenQuoteFactory();

	std::stack<Token*> tokenstack;
	std::stack<Operator*> operatorstack;
	std::stack<ParserState*> callstack;

	ParserStateDebug* errstate = new ParserStateDebug(NULL, "Parse error\n");
	errstate->name = "error";

	ParserStateChar* defaultstate = new ParserStateChar(errstate, NULL);
	defaultstate->name = "default";

	defaultstate->templatestate = defaultstate;

	ParserState* defaultstate2 = new ParserStateDebug(defaultstate, "entering default state\n");
	defaultstate2->name = "default_dbg";

	defaultstate->add(' ', defaultstate2);
	defaultstate->add('\t', defaultstate2);


	std::string::iterator mark;
	ParserStateChar* labelstartstate = new ParserStateChar(*defaultstate);
	labelstartstate->name = "label_start";

	ParserStateMark* labelstate = new ParserStateMark(labelstartstate, &mark);
	labelstate->name = "label";


	ParserStateTransition* labelstateend = new ParserStateAdvance(new ParserStateEmit(new ParserStateRet(errstate, &callstack), &mark, NULL, labelfactory, &tokenstack));
	labelstateend->name = "label_end";

	ParserStateChar* labelmidstate = new ParserStateChar(labelstateend, NULL);
	labelmidstate->name = "label_mid";
	labelmidstate->add('_', labelmidstate);
	labelmidstate->add('a', 'z', labelmidstate);
	labelmidstate->add('A', 'Z', labelmidstate);
	labelmidstate->add('0', '9', labelmidstate);
	labelmidstate->add(0x80, 0xFF, labelmidstate); // utf-8

	labelstartstate->add('_', labelmidstate);
	labelstartstate->add('a', 'z', labelmidstate);
	labelstartstate->add('A', 'Z', labelmidstate);


	ParserStateChar* idmodifierstate = new ParserStateChar(errstate, NULL);
	idmodifierstate->name = "idmodifierstate";
	idmodifierstate->add('@', new ParserStateCall(labelstate, idmodifierstate, &callstack));
	idmodifierstate->add(':', new ParserStateCall(labelstate, idmodifierstate, &callstack));
	idmodifierstate->add('?', new ParserStateCall(labelstate, idmodifierstate, &callstack));
	idmodifierstate->replace(errstate, new ParserStateAdvance(defaultstate));

	ParserStateCall* idstate = new ParserStateCall(new ParserStateAdvance(labelstate), idmodifierstate, &callstack);
	idstate->name = "idstate";


	defaultstate->add('_', idstate);
	defaultstate->add('a', 'z', idstate);
	defaultstate->add('A', 'Z', idstate);



	ParserStateChar* quotebodystate = new ParserStateChar(NULL, NULL);
	quotebodystate->name = "quotebodystate";
	quotebodystate->templatestate = quotebodystate;

	ParserStateChar* quoteescapestate = new ParserStateChar(quotebodystate, quotebodystate);
	quoteescapestate->name = "quoteescapestate";

	quotebodystate->add('\\', quoteescapestate);
	quotebodystate->add('"', new ParserStateEmit(new ParserStateRet(errstate, &callstack), &mark, NULL, quotefactory, &tokenstack));

	quotebodystate->replace(NULL, quotebodystate);
	quotebodystate->defaultstate = quotebodystate;

	ParserStateMark* quotestate = new ParserStateMark(quotebodystate, &mark);


	defaultstate->add('"', new ParserStateCall(quotestate, defaultstate, &callstack));



	ParserStateChar* newline = new ParserStateChar(*defaultstate);
	newline->name = "newline";
	newline->add(' ', newline);
	newline->add('\t', newline);

	newline->add('\n', newline);
	newline->add('\r', newline);

	defaultstate->add('\n', newline);
	defaultstate->add('\r', newline);


	ParserStateChar* comment = new ParserStateChar();
	comment->name = "comment";
	comment->add(0, 255, comment);
	comment->add('\n', newline);
	comment->add('\r', newline);

	newline->add('#', comment);

	//newline->add("debug", newline);



	if (argc <= 1)
	{
		graphviz_gen(std::cout, newline);
		return 0;
	}

	std::cout << "Parsing " << argv[1] << "\n";
	std::ifstream f(argv[1]);
	std::string s((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

	//std::cout << s;

	Parser* p = new Parser(newline, s);

	p->parse();

	return 0;
}
