#include "parser.hpp"

Token::Token(std::string::iterator start, std::string::iterator end, Parser* parser)
{
	tok = std::string(start, end);
}

Token* TokenFactory::factory_new(std::string::iterator start, std::string::iterator end, Parser* parser)
{
	return new Token(start, end, parser);
}


ParserState* ParserStateTransition::next(Parser* parser)
{
	std::cout << "transition" << this << "\n";

	return nextstate;
}


ParserState* ParserStateDebug::next(Parser* parser)
{
	std::cout << "debug" << this << ": " << msg;

	return nextstate;
}

ParserState* ParserStateBackUp::next(Parser* parser)
{
	std::cout << "backup" << this << "\n";

	parser->curr -= distance;

	return nextstate;
}


// Default constuctor, initializes everything to NULL (is this necessary?)
ParserStateChar::ParserStateChar() : templatestate(this)
{
	std::cout << "char new" << this << "\n";

	for (int i=0; i<256; i++)
	{
		transitions[i] = NULL;
	}
}

// Clone constructor.  Updates all self references, copies all others
ParserStateChar::ParserStateChar(const ParserStateChar& original) : templatestate(original.templatestate)
{
	std::cout << "char clone" << &original << "\n";

	for (int i=0; i<256; i++)
	{
		if (original.transitions[i] == &original)
		{
			transitions[i] = this;
		}
		else
		{
			transitions[i] = original.transitions[i];
		}
	}
}

ParserStateChar::ParserStateChar(ParserState* _defaultstate, ParserStateChar* _templatestate) : templatestate(_templatestate)
{
	std::cout << "char basic " << _defaultstate << ", " << _templatestate << "\n";

	for (int i=0; i<256; i++)
	{
		transitions[i] = _defaultstate;
	}
}

// copy transitions from another state
void ParserStateChar::add(const ParserStateChar& other)
{
	std::cout << "char" << this << " |= " << &other << "\n";

	for (int i=0; i<256; i++)
	{
		if (other.transitions[i] != NULL)
		{
			transitions[i] = other.transitions[i];
		}
	}
}


// add char transition
void ParserStateChar::add(unsigned char c, ParserState* nextstate)
{
	std::cout << "char" << this << " add char " << c2s(c) << "\n";

	transitions[c] = nextstate;
}

// range
void ParserStateChar::add(unsigned char first, unsigned char last, ParserState* nextstate)
{
	std::cout << "char" << this << " add range [" << first << "-" << last << "](" << ((unsigned int)(first) & 255) << "-" << ((unsigned int)(last) & 255) << ") -> " << nextstate << "[" << nextstate->name << "]\n";
	for (unsigned char curr=first; curr != last; curr++)
	{
		transitions[curr] = nextstate;
		//std::cout << curr << "\n";
	}
}

// replace occurences of one transition state with another
void ParserStateChar::replace(ParserState* target, ParserState* replacement)
{
	std::cout << "char" << this << " repl " << target << " -> " << replacement << "]\n";

	for (int i=0; i<256; i++)
	{
		if (transitions[i] == target)
		{
			transitions[i] = replacement;
		}
	}
}


// add string transition
void ParserStateChar::add(std::string word, ParserState* nextstate)
{
	std::cout << "char" << this << " add word \"" << word << "\" -> " << nextstate << "\n";

	add(word.begin(), word.end(), nextstate);
}

// add string transition by start and end iterators
void ParserStateChar::add(std::string::iterator start, std::string::iterator end, ParserState* nextstate)
{
	//std::cout << "char" << this << " add i \"" << std::string(start, end) << "\"[0] == " << c2s(*start) << " -> " << nextstate << "\n";
	std::cout << "char" << this << " add i \"" << std::string(start, end) << "\"[0] == " << c2s(*start) << " -> " << nextstate << ": ";

	if (start == end)
	{
		std::cout << "last\n";
		transitions[*start] = nextstate;
	}
	else
	{
		bool consed = false;
		if (transitions[*start] == NULL)
		{
			std::cout << "create {";
			transitions[*start] = new ParserStateChar(*templatestate);
			consed = true;
			std::cout << "}, ";
		}
		else if (dynamic_cast<ParserStateChar*>(transitions[*start]) == NULL)
		{
			std::cout << "replace {";
			transitions[*start] = new ParserStateChar(new ParserStateBackUp(transitions[*start]), templatestate);
			consed = true;
			std::cout << "}, ";
		}

		if (consed)
		{
			transitions[*start]->name = transitions[*start]->name.append(name);
		}

		std::cout << "more\n";

		((ParserStateChar*)transitions[*start])->add(start+1, end, nextstate);
	}
}



ParserState* ParserStateChar::next(Parser* parser)
{
	std::cout << "char" << this << ": " << c2s(*parser->curr) << "\n";

	ParserState* nextstate = transitions[*parser->curr];

	parser->curr++;

	return nextstate;
}


std::ostream& ParserState::print(std::ostream& out) const
{
	out << "(" << this << ")";

	if (this != NULL)
	{
		out << "[" << this->name << "]";
	}

	return out;
}

std::ostream& operator<<(std::ostream& out, const ParserState& st) { return st.print(out); }
std::ostream& operator<<(std::ostream& out, const ParserStateTransition& st) { return st.print(out); }
std::ostream& operator<<(std::ostream& out, const ParserStateDebug& st) { return st.print(out); }
std::ostream& operator<<(std::ostream& out, const ParserStateBackUp& st) { return st.print(out); }
std::ostream& operator<<(std::ostream& out, const ParserStateChar& st) { return st.print(out); }
std::ostream& operator<<(std::ostream& out, const ParserStateCall& st) { return st.print(out); }
std::ostream& operator<<(std::ostream& out, const ParserStateRet& st) { return st.print(out); }
std::ostream& operator<<(std::ostream& out, const ParserStateMark& st) { return st.print(out); }
std::ostream& operator<<(std::ostream& out, const ParserStateEmit& st) { return st.print(out); }


ParserState* ParserStateCall::next(Parser* parser)
{
	std::cout << "char" << this << "\n";

	stack->push(retstate);

	return nextstate;
}

ParserState* ParserStateRet::next(Parser* parser)
{
	std::cout << "ret" << this << "\n";

	if (!stack->empty())
	{
		stack->pop();
		return stack->top();
	}

	return failstate;
}


ParserState* ParserStateMark::next(Parser* parser)
{
	std::cout << "mark" << this << "\n";

	*mark = parser->curr;

	return nextstate;
}

ParserState* ParserStateEmit::next(Parser* parser)
{
	std::cout << "emit" << this << "\n";

	std::string::iterator enditer = endmark != NULL ? *endmark : parser->curr;

	stack->push(tokenfactory->factory_new(*startmark, enditer, parser));

	return nextstate;
}


Parser::Parser(ParserState* initial, std::string& code) : currState(initial), curr(code.begin()), end(code.end())
{

}

void Parser::parse()
{
	std::cout << "initial state: " << currState << "; initial char: '" << *curr << "' (" << ((unsigned int)((*curr) & 255)) << "); " << std::distance(curr,end) << " chars left\n";

	while (curr != end && currState != NULL)
	{
		std::cout << "current state: " << currState << "; current char: '" << *curr << "' (" << ((unsigned int)((*curr) & 255)) << "); " << std::distance(curr,end) << " chars left\n";
		currState = currState->next(this);
	}

	std::cout << "final state: " << currState << "; final char: '" << *curr << "' (" << ((unsigned int)(*curr) & 255) << "); " << std::distance(curr,end) << " chars left\n";


	std::cout << "Done!\n";

	return NULL;
}
