#include <stdlib.h>
#include <stdio.h>

#include "stack.h"

#define LIBDEBUG 0

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
	if (this->top >= this->len)
	{
		this->len *= 2;
		this->base = realloc(this->base, this->len*sizeof(void*));
#if LIBDEBUG
		printf("resize stack to %d\n", this->len);
#endif
	}

#if LIBDEBUG >= 2
	printf("push: (%p)[%u] = %p\n", this, this->top, v);
#endif

	this->base[this->top++] = v;
}

void* stack_pop(stack* this)
{
	if (this->top <= 0)
	{
		return NULL;
	}

#if LIBDEBUG >= 2
	printf("pop: (%p)[%u] = %p\n", this, this->top-1, this->base[this->top-1]);
#endif

	return this->base[--this->top];
}

void* stack_peek(stack* this)
{
	if (this->top <= 0)
	{
		return NULL;
	}

	return this->base[this->top-1];
}
