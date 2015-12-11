#include "lexer.hpp"

int graphviz_cgen = 0;      // the current graphviz generation number
int lastid = 0;

std::ostream& graphviz_gen(std::ostream& out, ParserState* root)
{
	graphviz_cgen++;

	out << "digraph statemachine {\n";

	root->graphviz_append(out);
	
	out << "}\n";

	return out;
}

inline std::string c2s(char c)
{
	std::stringstream ss;
	ss << "'";
	switch (c)
	{
		case '\n': ss << "\\\\n"; break;
		case '\r': ss << "\\\\r"; break;
		case '\t': ss << "\\\\t"; break;
		case '"' : ss << "\\" ; break;
		default  :
		{
			if (c >= 32 && c < 127)
			{
				ss << c;
			}
			else
			{
				ss << '?';
			}
			break;
		}
	}

	ss << "'(" << ((unsigned int)(c) & 255) << ")";

	return ss.str();
}

std::ostream& st2s_ss(std::ostream& out, const ParserState* st)
{
	out << "(" << st << ")";

	if (st != NULL && st->name != "")
	{
		out << "[" << st->name << "]";
	}

	return out;
}

std::string st2s(const ParserState* st)
{
	std::stringstream out;

	st2s_ss(out, st);

	return out.str();
}

Token::Token(std::string::iterator start, std::string::iterator end, Parser* parser)
{
	tok = std::string(start, end);
}

Token* TokenFactory::factory_new(std::string::iterator start, std::string::iterator end, Parser* parser)
{
	return new Token(start, end, parser);
}


Operator::Operator(std::string::iterator start, std::string::iterator end, Parser* parser)
{
	tok = std::string(start, end);
}

Operator* OperatorFactory::factory_new(std::string::iterator start, std::string::iterator end, Parser* parser)
{
	return new Operator(start, end, parser);
}

ParserState::ParserState()
{
	graphviz_gen = graphviz_cgen;

	id = lastid++;
}

std::ostream& ParserState::graphviz_append(std::ostream& out)
{
	if (graphviz_gen == graphviz_cgen)
	{
		return out;
	}
	graphviz_gen = graphviz_cgen;

	out << '\t' << this->id << " [label = \"";
	st2s_ss(out, this);
	out << "\"];\n";

	return out;
}

ParserState* ParserStateTransition::next(Parser* parser)
{
	std::cout << "transition" << st2s(this) << "\n";

	return nextstate;
}

std::ostream& ParserStateTransition::graphviz_append(std::ostream& out)
{
	if (graphviz_gen == graphviz_cgen)
	{
		return out;
	}
	graphviz_gen = graphviz_cgen;

	out << '\t' << this->id << " [label = \"";
	st2s_ss(out, this);
	out << "\"];\n";

	if (nextstate != NULL)
	{
		nextstate->graphviz_append(out);
	}
	

	return out;
}


ParserState* ParserStateDebug::next(Parser* parser)
{
	std::cout << "debug" << st2s(this) << ": " << msg;

	return nextstate;
}

ParserState* ParserStateAdvance::next(Parser* parser)
{
	std::cout << "advance" << st2s(this) << "\n";

	parser->curr += distance;

	return nextstate;
}


// Default constuctor, initializes everything to NULL (is this necessary?)
ParserStateChar::ParserStateChar() : defaultstate(NULL), templatestate(this)
{
	std::cout << "char" << st2s(this) << " new" << st2s(this) << "\n";

	for (int i=0; i<256; i++)
	{
		transitions[i] = NULL;
	}
}

// Clone constructor.  Updates all self references, copies all others
ParserStateChar::ParserStateChar(const ParserStateChar& original) : defaultstate(original.defaultstate), templatestate(original.templatestate)
{
	std::cout << "char" << st2s(this) << " clone" << st2s(&original) << "\n";

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

ParserStateChar::ParserStateChar(ParserState* _defaultstate, ParserStateChar* _templatestate) : defaultstate(_defaultstate), templatestate(_templatestate)
{
	std::cout << "char" << st2s(this) << " basic " << _defaultstate << ", " << _templatestate << "\n";

	for (int i=0; i<256; i++)
	{
		transitions[i] = _defaultstate;
	}
}

// copy transitions from another state
void ParserStateChar::add(const ParserStateChar& other)
{
	std::cout << "char" << st2s(this) << " |= " << st2s(&other) << "\n";

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
	std::cout << "char" << st2s(this) << " add char " << c2s(c) << "\n";

	transitions[c] = nextstate;
}

// range
void ParserStateChar::add(unsigned char first, unsigned char last, ParserState* nextstate)
{
	std::cout << "char" << st2s(this) << " add range [" << first << "-" << last << "](" << ((unsigned int)(first) & 255) << "-" << ((unsigned int)(last) & 255) << ") -> " << nextstate << "[" << nextstate->name << "]\n";
	for (unsigned char curr=first; curr != last; curr++)
	{
		transitions[curr] = nextstate;
		//std::cout << curr << "\n";
	}

	transitions[last] = nextstate;
}

// replace occurences of one transition state with another
void ParserStateChar::replace(ParserState* target, ParserState* replacement)
{
	std::cout << "char" << st2s(this) << " repl " << st2s(target) << " -> " << st2s(replacement) << "]\n";

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
	std::cout << "char" << st2s(this) << " add word \"" << word << "\" -> " << st2s(nextstate) << "\n";

	add(word.begin(), word.end(), nextstate);
}

// add string transition by start and end iterators
void ParserStateChar::add(std::string::iterator start, std::string::iterator end, ParserState* nextstate)
{
	//std::cout << "char" << st2s(this) << " add i \"" << std::string(start, end) << "\"[0] == " << c2s(*start) << " -> " << st2s(nextstate) << "\n";
	std::cout << "char" << st2s(this) << " add i \"" << std::string(start, end) << "\"[0] == " << c2s(*start) << " -> " << st2s(nextstate) << ": ";

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
			transitions[*start] = new ParserStateChar(new ParserStateAdvance(transitions[*start]), templatestate);
			consed = true;
			std::cout << "}, ";
		}

		if (consed)
		{
			transitions[*start]->name = name + "," + *start;
		}

		std::cout << "more\n";

		((ParserStateChar*)transitions[*start])->add(start+1, end, nextstate);
	}
}



ParserState* ParserStateChar::next(Parser* parser)
{
	std::cout << "char" << st2s(this) << ": " << c2s(*parser->curr) << "\n";

	ParserState* nextstate = transitions[*parser->curr];

	parser->curr++;

	return nextstate;
}

std::ostream& ParserStateChar::graphviz_append(std::ostream& out)
{
	if (graphviz_gen == graphviz_cgen)
	{
		return out;
	}
	graphviz_gen = graphviz_cgen;

	out << '\t' << this->id << " [label = \"";

	st2s_ss(out, this);

	out << "\"];\n";
	
	int j=0;
	ParserState* prev = transitions[0];
	for (int i=0; i<255; i++)
	{
		ParserState* next = transitions[i];
		
		if (next != prev)
		{
			if (prev != NULL && prev->name != "error")
			{
				if (j != i-1)
				{
					out << "\t" << this->id << " -> " << prev->id << " [label = \"" << c2s(j) << " - " << c2s(i-1) << "\"];\n";
				}
				else
				{
					out << "\t" << this->id << " -> " << prev->id << " [label = \"" << c2s(j) << "\"];\n";
				}
			}
		
			j = i;
			
			prev = next;
		}
		
		if (next != NULL)
		{
			next->graphviz_append(out);
		}
	}	

	return out;
}


ParserState* ParserStateCall::next(Parser* parser)
{
	std::cout << "char" << st2s(this) << "\n";

	stack->push(retstate);

	return nextstate;
}

ParserState* ParserStateRet::next(Parser* parser)
{
	std::cout << "ret" << st2s(this) << "\n";

	if (!stack->empty())
	{
		ParserState* tos = stack->top();
		stack->pop();
		return tos;
	}

	std::cout << "ret failed!\n";

	return failstate;
}


ParserState* ParserStateMark::next(Parser* parser)
{
	std::cout << "mark" << st2s(this) << "\n";

	*mark = parser->curr;

	return nextstate;
}

ParserState* ParserStateEmit::next(Parser* parser)
{
	std::cout << "emit" << st2s(this) << "\n";

	std::string::iterator enditer = endmark != NULL ? *endmark : parser->curr;

	stack->push(tokenfactory->factory_new(*startmark, enditer, parser));

	return nextstate;
}

ParserState* ParserStateEmitOperator::next(Parser* parser)
{
	std::cout << "emit" << st2s(this) << "\n";

	std::string::iterator enditer = endmark != NULL ? *endmark : parser->curr;

	stack->push(tokenfactory->factory_new(*startmark, enditer, parser));

	return nextstate;
}



Parser::Parser(ParserState* initial, std::string& code) : currState(initial), curr(code.begin()), end(code.end())
{

}

void Parser::parse()
{
	std::cout << "initial state: " << st2s(currState) << "; initial char: " << c2s(*curr) << "; " << std::distance(curr,end) << " chars left\n";

	while (curr != end && currState != NULL)
	{
		//std::cout << "current state: " << st2s(currState) << "; current char: " << c2s(*curr) << "; " << std::distance(curr,end) << " chars left\n";
		currState = currState->next(this);
	}

	std::cout << "final state: " << st2s(currState) << "; final char: " << c2s(*curr) << "; " << std::distance(curr,end) << " chars left\n";


	std::cout << "Done!\n";

	return NULL;
}
