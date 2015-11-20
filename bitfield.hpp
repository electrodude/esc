#pragma once

#include "parallax_types.hpp"

class bitfield
{
public:
	bitfield(const char* s);

	bitfield() : data(0), mask(0) {}

	bitfield(plong _data) : data(data), mask(-1) {}

	bitfield(plong _data, plong _mask) : data(data), mask(_mask) {}

	void print() const;

	static bitfield merge(const bitfield& lhs, const bitfield& rhs);
	int merge(const bitfield* src);

	plong data;
	plong mask;
	//plong conflict;
};

bitfield operator | (const bitfield& lhs, const bitfield& rhs);
bitfield operator & (const bitfield& lhs, const bitfield& rhs);
bitfield operator ^ (const bitfield& lhs, const bitfield& rhs);
bitfield operator ~ (const bitfield& rhs);

bitfield operator << (const bitfield& lhs, const bitfield& rhs);
bitfield operator >> (const bitfield& lhs, const bitfield& rhs);

bitfield operator << (const bitfield& lhs, int rhs);
bitfield operator >> (const bitfield& lhs, int rhs);

// higher level ops

bitfield operator + (const bitfield& lhs, const bitfield& rhs);
bitfield operator - (const bitfield& rhs);
bitfield operator - (const bitfield& lhs, const bitfield& rhs);


