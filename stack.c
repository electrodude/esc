#include <stdlib.h>

#include "stack.h"

stack* stack_new()
{
	stack* this = malloc(sizeof(stack));

	this->len = 16;
	this->base = malloc(this->len*sizeof(void*));
	this->top = 0;

	return this;
}

void stack_push(stack* this, void* v)
{
	this->top++;

	if (this->top >= this->len)
	{
		this->len *= 2;
		this->base = realloc(this->base, this->len*sizeof(void*));
#if LIBDEBUG
		printf("resize stack to %d\n", this->len);
#endif
	}

	this->base[this->top] = v;
}

void* stack_pop(stack* this)
{
	if (this->top <= 0)
	{
		return NULL;
	}

	return this->base[this->top--];
}

void* stack_peek(stack* this)
{
	if (this->top <= 0)
	{
		return NULL;
	}

	return this->base[this->top];
}
