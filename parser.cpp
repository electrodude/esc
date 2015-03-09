#include "parser.hpp"

Token::Token(std::string str)
{
	tok = str;
}

ParserState* ParserStateChar::next(Parser* parser, char c) const
{
	return transitions[c];
}

Token* ParserStateToken::getToken(Parser* parser) const
{

}




Parser::Parser(ParserState* initial, std::string code)
{
	curr = code.begin();

	end = code.end();

	currState = initial;
	
}

Token* Parser::next()
{

	while (curr != end)
	{
		ParserState* newState;
		if (currState->acceptsChar())
		{
			newState = ((ParserStateChar*)currState)->next(this, *curr++);
		}
		else
		{
			newState = currState->next(this);
		}

		currState = newState;


		if (currState->hasToken())
		{
			return ((ParserStateToken*)currState)->getToken(this);
		}
	}


	return EOF;
}
