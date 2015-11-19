#pragma once

#include "parallax_types.h"

typedef struct bitfield
{
	plong data;
	plong mask;
	//plong conflict;
} bitfield;

bitfield bitfield_new_s(const char* s);

void bitfield_print(bitfield* bf);

//bitfield bitfield_merge(bitfield* a, bitfield* b)
int bitfield_merge(bitfield* dest, bitfield* src);

bitfield bitfield_or(bitfield* lhs, bitfield* rhs);
bitfield bitfield_and(bitfield* lhs, bitfield* rhs);
bitfield bitfield_xor(bitfield* lhs, bitfield* rhs);
bitfield bitfield_not(bitfield* rhs);

bitfield bitfield_shl(bitfield* lhs, bitfield* rhs);
bitfield bitfield_shr(bitfield* lhs, bitfield* rhs);

bitfield bitfield_shl_int(bitfield* lhs, int rhs);
bitfield bitfield_shr_int(bitfield* lhs, int rhs);

// higher level ops

bitfield bitfield_add(bitfield* lhs, bitfield* rhs);
bitfield bitfield_neg(bitfield* rhs);
bitfield bitfield_sub(bitfield* lhs, bitfield* rhs);
