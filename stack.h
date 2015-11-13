#pragma once

typedef struct stack
{
	void** base;
	int top;
	int len;
} stack;

stack* stack_new();
void stack_push(stack* this, void* v);
void* stack_pop(stack* this);
void* stack_peek(stack* this);
