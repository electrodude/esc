#pragma once

#include "parallax_types.h"

#include "stack.h"

extern int tabwidth;

void parser_init(void);

stack* parser(char* p);
