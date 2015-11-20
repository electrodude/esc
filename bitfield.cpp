#include <stdio.h>

#include "bitfield.hpp"

// bitfield

bitfield::bitfield(const char* s)
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

	this->data = data;
	this->mask = mask;
}

void bitfield::print() const
{
	plong data = this->data;
	plong mask = this->mask;

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

bitfield bitfield::merge(const bitfield& lhs, const bitfield& rhs)
{
	bitfield bf = lhs;

	//bf.conflict |= bf.mask & rhs.mask;
	int conflict = bf.mask & rhs.mask;

	if (conflict)
	{
		throw conflict;
	}

	bf.data |= rhs.mask & rhs.data;

	bf.mask |= rhs.mask;

	return bf;
}

int bitfield::merge(const bitfield* src)
{
	int conflict = this->mask & src->mask;

	if (conflict)
	{
		return conflict;
	}

	this->data |= src->mask & src->data;

	this->mask |= src->mask;

	return 0;
}

bitfield operator |(const bitfield& lhs, const bitfield& rhs)
{
	plong mask = (lhs.mask & rhs.mask) | (lhs.mask & lhs.data) | (rhs.mask & rhs.data);

	plong data = lhs.data | rhs.data;

	return bitfield(data, mask);
}

bitfield operator &(const bitfield& lhs, const bitfield& rhs)
{
	plong mask = (lhs.mask & rhs.mask) | (lhs.mask & ~lhs.data) | (rhs.mask & ~rhs.data);

	plong data = lhs.data & rhs.data;

	return bitfield(data, mask);
}

bitfield operator ^(const bitfield& lhs, const bitfield& rhs)
{
	plong mask = lhs.mask & rhs.mask;

	plong data = lhs.data ^ rhs.data;

	return bitfield(data, mask);
}

bitfield operator ~(const bitfield& rhs)
{
	plong mask = rhs.mask;

	plong data = ~rhs.data;

	return bitfield(data, mask);
}

bitfield operator <<(const bitfield& lhs, const bitfield& rhs)
{
	if (rhs.mask & 31 != 31)
	{
		// TODO: should try each rotation and look for anything they all share
		return (bitfield){.data=0, .mask=0};
	}

	return lhs << rhs.data;
}

bitfield operator >>(const bitfield& lhs, const bitfield& rhs)
{
	if (rhs.mask & 31 != 31)
	{
		// TODO: should try each rotation and look for anything they all share
		return bitfield(0, 0);
	}

	return lhs >> rhs.data;
}

bitfield operator <<(const bitfield& lhs, int rhs)
{
	plong mask = lhs.mask << (rhs & 31);

	plong data = lhs.data << (rhs & 31);

	return bitfield(data, mask);
}

bitfield operator >>(const bitfield& lhs, int rhs)
{
	plong mask = lhs.mask >> (rhs & 31);

	plong data = lhs.data >> (rhs & 31);

	return bitfield(data, mask);
}

// higher level ops

bitfield operator +(const bitfield& lhs, const bitfield& rhs)
{
	bitfield sum = lhs ^ rhs;
	bitfield carry = lhs & rhs;

	while (carry.data)
	{
		bitfield carry2 = carry << 1;

		bitfield sum2 = sum ^ carry2;
		carry = sum & carry2;

		sum = sum2;
	}

	return sum;
}

bitfield operator -(const bitfield& rhs)
{
	static bitfield one(1);

	return (~rhs) + one;
}

bitfield operator -(const bitfield& lhs, const bitfield& rhs)
{
	return lhs + (-rhs);
}
