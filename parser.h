#pragma once

#include "parallax_types.h"

#include "stack.h"

void parser_init(void);

stack* parser(char* p);
