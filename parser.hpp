#pragma once

#include <vector>

#include "parallax_types.hpp"

#include "parserlib.hpp"

extern int tabwidth;

void parser_init(void);

Line* parser(char* p);
