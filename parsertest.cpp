#include <iostream>
#include <fstream>
#include <streambuf>

#include "parser.hpp"

class TokenLabel : public Token
{
public:
	TokenLabel(std::string::iterator start, std::string::iterator end, Parser* parser) : Token(start, end, parser) {}

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

int main(int argc, char** argv)
{
	std::cout << "Parser test...\n";

	TokenFactory* labelfactory = new TokenLabelFactory();

	std::stack<Token*> stack;

	ParserStateDebug* errstate = new ParserStateDebug(NULL, "Parse error\n");
	errstate->name = "error";

	ParserStateChar* defaultstate = new ParserStateChar(errstate, NULL);
	defaultstate->name = "";

	defaultstate->templatestate = defaultstate;

	ParserState* defaultstate2 = new ParserStateDebug(defaultstate, "entering default state\n");
	defaultstate2->name = "default_dbg";

	defaultstate->add(" ", defaultstate2);
	defaultstate->add("\t", defaultstate2);

	ParserStateChar* newline = new ParserStateChar(errstate, defaultstate);
	newline->name = "newline";

	ParserStateChar* comment = new ParserStateChar();
	comment->name = "comment";
	comment->add(0, 255, comment);
	comment->add('\n', newline);
	comment->add('\r', newline);

	newline->add('#', comment);

	newline->add("debug", newline);

	defaultstate->add('\n', newline);
	defaultstate->add('\r', newline);

	std::string::iterator mark;
	ParserStateChar* labelstate = new ParserStateChar(*defaultstate);
	labelstate->name = "label";
	ParserStateMark* labelstartstate = new ParserStateMark(labelstate, &mark);
	labelstartstate->name = "label_start";

	labelstate->add('a', 'z', labelstate);
	labelstate->add('A', 'Z', labelstate);

	ParserStateTransition* labelstateend = new ParserStateEmit(defaultstate, &mark, NULL, labelfactory, &stack);
	labelstateend->name = "label_end";

	ParserStateChar* labelstatemid = new ParserStateChar(labelstateend, NULL);
	labelstatemid->name = "label_mid";
	labelstatemid->add('_', labelstatemid);
	labelstatemid->add('a', 'z', labelstatemid);
	labelstatemid->add('A', 'Z', labelstatemid);
	labelstatemid->add('A', 'Z', labelstatemid);



	if (argc <= 1)
	{
		std::cout << "Usage: parsertest file\n";
		return -1;
	}

	std::cout << "Parsing " << argv[1] << "\n";
	std::ifstream f(argv[1]);
	std::string s((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

	//std::cout << s;

	Parser* p = new Parser(newline, s);

	p->parse();

	return 0;
}
