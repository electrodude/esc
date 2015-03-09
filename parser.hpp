#include <string>

class Parser;

class Token
{
public:
	Token(std::string str);

	virtual ~Token() {};

	virtual bool isIdent() const = 0;

	virtual bool isNum() const;

	std::string tok;

};


class ParserState
{
public:
	virtual ParserState* next(Parser* parser) const;

	virtual bool acceptsChar() const { return false; }

	virtual bool hasToken() const { return false; }

};

class ParserStateChar : public ParserState
{
public:
	virtual ParserState* next(Parser* parser, char c) const;

	virtual bool acceptsChar() const { return true; }

	ParserState* transitions[256];
};

class ParserStateToken : public ParserState
{
public:
	virtual bool hasToken() const { return true; }

	virtual Token* getToken(Parser* parser) const;

	Token* token;
};

class Parser
{
public:
	Parser(ParserState* initial, std::string code);

	Token* next();

	std::string::iterator curr;

	std::string::iterator end;

	ParserState* currState;
	
};
