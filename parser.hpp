#pragma once

#include <vector>

#include "parallax_types.hpp"

#include "parserlib.hpp"

extern int tabwidth;

void parser_init(void);

std::vector<Block*>* parser(char* p);
