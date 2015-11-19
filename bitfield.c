#include <stdio.h>

#include "bitfield.h"

// bitfield

bitfield bitfield_new_s(const char* s)
{
	plong data = 0;
	plong mask = 0;

	while (*s)
	{
		switch (*s++)
		{
			case '0':
			{
				data = (data << 1) | 0;
				mask = (mask << 1) | 1;
				break;
			}
			case '1':
			{
				data = (data << 1) | 1;
				mask = (mask << 1) | 1;
				break;
			}
			case 'x':
			{
				data = (data << 1) | 0;
				mask = (mask << 1) | 0;
				break;
			}
			case 'y':
			{
				data = (data << 1) | 1;
				mask = (mask << 1) | 0;
				break;
			}
		}
	}

	bitfield bf = {.data = data, .mask = mask/*, .conflict = 0*/};

	return bf;
}

void bitfield_print(bitfield* bf)
{
	plong data = bf->data;
	plong mask = bf->mask;

	char out[33];

	for (int i=31; i >= 0; i--)
	{
		out[i] = ((mask & 1) ? '0' : 'x') + (data & 1);

		data >>= 1;
		mask >>= 1;
	}

	out[32] = 0;

	printf("%s", out);
}

/*
bitfield bitfield_merge(bitfield* a, bitfield* b)
{
	bitfield bf = *a;

	bf.conflict |= bf.mask & b->mask;

	bf.data |= (b->mask & b->data);

	bf.mask |= b->mask;

	return bf;
}
*/

int bitfield_merge(bitfield* dest, bitfield* src)
{
	int conflict = dest->mask & src->mask;

	if (conflict)
	{
		return conflict;
	}

	dest->data |= src->mask & src->data;

	dest->mask |= src->mask;

	return 0;
}

bitfield bitfield_or(bitfield* lhs, bitfield* rhs)
{
	plong mask = (lhs->mask & rhs->mask) | (lhs->mask & lhs->data) | (rhs->mask & rhs->data);

	plong data = lhs->data | rhs->data;

	return (bitfield){.data = data, .mask = mask};
}

bitfield bitfield_and(bitfield* lhs, bitfield* rhs)
{
	plong mask = (lhs->mask & rhs->mask) | (lhs->mask & ~lhs->data) | (rhs->mask & ~rhs->data);

	plong data = lhs->data & rhs->data;

	return (bitfield){.data = data, .mask = mask};
}

bitfield bitfield_xor(bitfield* lhs, bitfield* rhs)
{
	plong mask = lhs->mask & rhs->mask;

	plong data = lhs->data ^ rhs->data;

	return (bitfield){.data = data, .mask = mask};
}

bitfield bitfield_not(bitfield* rhs)
{
	plong mask = rhs->mask;

	plong data = ~rhs->data;

	return (bitfield){.data = data, .mask = mask};
}

bitfield bitfield_shl(bitfield* lhs, bitfield* rhs)
{
	if (rhs->mask & 31 != 31)
	{
		// TODO: should try each rotation and look for anything they all share
		return (bitfield){.data=0, .mask=0};
	}

	return bitfield_shl_int(lhs, rhs->data);
}

bitfield bitfield_shr(bitfield* lhs, bitfield* rhs)
{
	if (rhs->mask & 31 != 31)
	{
		// TODO: should try each rotation and look for anything they all share
		return (bitfield){.data=0, .mask=0};
	}

	return bitfield_shr_int(lhs, rhs->data);
}

bitfield bitfield_shl_int(bitfield* lhs, int rhs)
{
	plong mask = lhs->mask << (rhs & 31);

	plong data = lhs->data << (rhs & 31);

	return (bitfield){.data = data, .mask = mask};
}

bitfield bitfield_shr_int(bitfield* lhs, int rhs)
{
	plong mask = lhs->mask >> (rhs & 31);

	plong data = lhs->data >> (rhs & 31);

	return (bitfield){.data = data, .mask = mask};
}

// higher level ops

bitfield bitfield_add(bitfield* lhs, bitfield* rhs)
{
	bitfield sum = bitfield_xor(lhs, rhs);
	bitfield carry = bitfield_and(lhs, rhs);

	while (carry.data)
	{
		bitfield carry2 = bitfield_shl_int(&carry, 1);

		bitfield sum2 = bitfield_xor(&sum, &carry2);
		carry = bitfield_and(&sum, &carry2);

		sum = sum2;
	}

	return sum;
}

bitfield bitfield_neg(bitfield* rhs)
{
	bitfield not = bitfield_not(rhs);

	static bitfield one = {.data = 1, .mask = -1};

	return bitfield_add(&not, &one);
}

bitfield bitfield_sub(bitfield* lhs, bitfield* rhs)
{
	bitfield neg = bitfield_neg(rhs);

	return bitfield_add(lhs, rhs);
}
