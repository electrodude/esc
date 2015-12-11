#include <stack>
#include <string>

#include <iostream>
#include <sstream>


class Parser;

class Token
{
public:
	Token() {};
	Token(std::string::iterator start, std::string::iterator end, Parser* parser);

	virtual ~Token() {}

	virtual bool isIdent() const { return false; }

	virtual bool isNum() const { return false; }

	//virtual String(const Token& token);

	std::string tok;
};

class TokenFactory
{
public:
	virtual Token* factory_new(std::string::iterator start, std::string::iterator end, Parser* parser);
};

class Operator : public Token
{
public:
	Operator(std::string::iterator start, std::string::iterator end, Parser* parser);

	virtual ~Operator() {};
};

class OperatorFactory
{
public:
	virtual Operator* factory_new(std::string::iterator start, std::string::iterator end, Parser* parser);
};


// The basic abstract parser state
class ParserState
{
public:
	ParserState();

	virtual ParserState* next(Parser* parser) = 0;

	virtual bool hasToken() const { return false; }

	std::string name;	// for debug

	//virtual std::ostream& print(std::ostream& out) const;

	int graphviz_gen;

	int id;
	
	virtual std::ostream& graphviz_append(std::ostream& out);
};

// Superclass for states that have exactly one non-error next state
class ParserStateTransition : public ParserState
{
public:
	ParserStateTransition();
	ParserStateTransition(ParserState* _nextstate) : nextstate(_nextstate) {}

	virtual ~ParserStateTransition() {}

	virtual ParserState* next(Parser* parser);

	virtual std::ostream& graphviz_append(std::ostream& out);

	ParserState* nextstate;
};

// debug
class ParserStateDebug : public ParserStateTransition
{
public:
	ParserStateDebug();
	ParserStateDebug(ParserState* _nextstate, std::string _msg) : ParserStateTransition(_nextstate), msg(_msg) {}

	virtual ~ParserStateDebug() {}

	virtual ParserState* next(Parser* parser);

	//virtual std::ostream& graphviz_append(std::ostream& out);

	std::string msg;
};

class ParserStateAdvance : public ParserStateTransition
{
public:
	ParserStateAdvance(ParserState* _nextstate) : ParserStateTransition(_nextstate), distance(1) {}
	ParserStateAdvance(ParserState* _nextstate, int _distance) : ParserStateTransition(_nextstate), distance(_distance) {}

	virtual ~ParserStateAdvance() {}

	virtual ParserState* next(Parser* parser);

	//virtual std::ostream& graphviz_append(std::ostream& out);

	int distance;
};

// accept char
class ParserStateChar : public ParserState
{
public:
	ParserStateChar();
	ParserStateChar(const ParserStateChar& original);
	ParserStateChar(ParserState* _defaultstate, ParserStateChar* _templatestate);

	virtual ~ParserStateChar() {}


	void add(unsigned char c, ParserState* nextstate);
	void add(std::string word, ParserState* nextstate);
	void add(std::string::iterator start, std::string::iterator end, ParserState* nextstate);
	void add(const ParserStateChar& additional);
	void add(unsigned char first, unsigned char last, ParserState* nextstate);

	void replace(ParserState* target, ParserState* replacement);

	virtual ParserState* next(Parser* parser);

	virtual std::ostream& graphviz_append(std::ostream& out);

	ParserState* defaultstate;
	ParserStateChar* templatestate;

	ParserState* transitions[256];
};


// state stack call
class ParserStateCall : public ParserStateTransition
{
public:
	ParserStateCall();
	ParserStateCall(ParserState* _nextstate, ParserState* _retstate, std::stack<ParserState*>* _stack) :
		ParserStateTransition(_nextstate),
		retstate(_retstate),
		stack(_stack) {}

	virtual ~ParserStateCall() {}

	virtual ParserState* next(Parser* parser);

	//virtual std::ostream& graphviz_append(std::ostream& out);

	ParserState* retstate;

	std::stack<ParserState*>* stack;
};

// state stack return
class ParserStateRet : public ParserState
{
public:
	ParserStateRet();
	ParserStateRet(ParserState* _failstate, std::stack<ParserState*>* _stack) :
		failstate(_failstate),
		stack(_stack) {}

	virtual ~ParserStateRet() {}

	virtual ParserState* next(Parser* parser);

	//virtual std::ostream& graphviz_append(std::ostream& out);

	ParserState* failstate;

	std::stack<ParserState*>* stack;
};


// token start
class ParserStateMark : public ParserStateTransition
{
public:
	ParserStateMark();
	ParserStateMark(ParserState* _nextstate, std::string::iterator* _mark) :
		ParserStateTransition(_nextstate),
		mark(_mark) {}

	virtual ~ParserStateMark() {}

	virtual ParserState* next(Parser* parser);

	//virtual std::ostream& graphviz_append(std::ostream& out);

	std::string::iterator* mark;
};

// token end
class ParserStateEmit : public ParserStateTransition
{
public:
	ParserStateEmit();
	ParserStateEmit(ParserState* _nextstate, std::string::iterator* _startmark, std::string::iterator* _endmark, TokenFactory* _tokenfactory, std::stack<Token*>* _stack) :
		ParserStateTransition(_nextstate),
		startmark(_startmark),
		endmark(_endmark),
		tokenfactory(_tokenfactory),
		stack(_stack) {}

	virtual ~ParserStateEmit() {}

	virtual bool hasToken() const { return true; }

	virtual ParserState* next(Parser* parser);

	//virtual std::ostream& graphviz_append(std::ostream& out);

	std::string::iterator* startmark;

	std::string::iterator* endmark;

	TokenFactory* tokenfactory;

	std::stack<Token*>* stack;
};

// operator end
class ParserStateEmitOperator : public ParserStateEmit
{
public:
	ParserStateEmitOperator();
	ParserStateEmitOperator(ParserState* _nextstate, std::string::iterator* _startmark, std::string::iterator* _endmark, TokenFactory* _tokenfactory, std::stack<Token*>* _valuestack, std::stack<Operator*>* _operatorstack) :
		ParserStateEmit(_nextstate, _startmark, _endmark, _tokenfactory, _valuestack),
		operatorstack(_operatorstack) {}

	virtual ~ParserStateEmitOperator() {}

	virtual ParserState* next(Parser* parser);

	//virtual std::ostream& graphviz_append(std::ostream& out);

	std::stack<Operator*>* operatorstack;
};

// parser class
class Parser
{
public:
	Parser(ParserState* initial, std::string& code);

	void parse();

	std::string::iterator curr;

	std::string::iterator end;

	ParserState* currState;
};


std::ostream& graphviz_gen(std::ostream& out, ParserState* root);

