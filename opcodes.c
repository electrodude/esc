#include <stdlib.h>

#include "parserlib.h"

grammardef* register_spin(void)
{
	// basic universal Spin operators

	// . and #
	operator_new("." ,  -11,  1,1);
	operator_new("#" ,  -11,  -1,1);

	operator_new("~" ,   0,  0,1);
	operator_new("~~",   0,  0,1);

	operator_new("@" ,   0,  0,1);
	operator_new("@@",   0,  0,1);

	operator_new("+",    1,  0,1);
	operator_new("-",    1,  0,1);
	operator_new("^^",   1,  0,1);
	operator_new("||",   1,  0,1);
	operator_new("|<",   1,  0,1);
	operator_new(">|",   1,  0,1);
	operator_new("!",    1,  0,1);

	operator_new("<-",   2,  1,1);
	operator_new("->",   2,  1,1);
	operator_new("<<",   2,  1,1);
	operator_new(">>",   2,  1,1);
	operator_new("~>",   2,  1,1);
	operator_new("><",   2,  1,1);

	operator_new("&",    3,  1,1);

	operator_new("|",    4,  1,1);
	operator_new("^",    4,  1,1);

	operator_new("*",    5,  1,1);
	operator_new("**",   5,  1,1);
	operator_new("/",    5,  1,1);
	operator_new("//",   5,  1,1);

	operator_new("+",    6,  1,1);
	operator_new("-",    6,  1,1);

	operator_new("#>",   7,  1,1);
	operator_new("<#",   7,  1,1);

	operator_new("<",    8,  1,1);
	operator_new(">",    8,  1,1);
	operator_new("<>",   8,  1,1);
	operator_new("==",   8,  1,1);
	operator_new("=<",   8,  1,1);
	operator_new("=>",   8,  1,1);


	operator_new("not",  9,  0,1);

	operator_new("and",  10, 1,1);

	operator_new("or",   11, 1,1);

	operator_new("(",   -10, 1,2); // function call
	operator_new("(",    10, 0,2);
	operator_new(")",    10, 2,0)->push = 0;

	operator_new("[",  -10, 1,3); // array index
	//operator_new("[",   10, 0,3); // unused [val] notation
	operator_new("]",   10, 3,0);

	grammar_push(grammar_new(grammar->symbols, NULL, NULL, 0, 0));
		blockdef* objblock = blockdef_new("obj");

		operator_new(":",    16, 1,1);

		operator_new("[",  -10, 1,3)->grammar = grammar; // instance count

	grammar_pop();



	grammar_push(NULL);
		blockdef* conblock = blockdef_new("con");

		operator_new("#" ,   0,  0,1); // check precedence of this
		operator_new("=",    12, 1,1);
		operator_new(",",    13, 1,1);

	grammar_pop();

	grammar_push(NULL);
		blockdef* varblock = blockdef_new("var");

		operator_new(",",    13, 1,1);
		operator_new("",     15, 1,1);

		/*
		label_new("byte");
		label_new("word");
		label_new("long");
		*/

	grammar_pop();

	grammar_push(NULL);
		blockdef* pubblock = blockdef_new("pub");
		blockdef* priblock = blockdef_new("pri");

		grammar->hasindent = 1;

		operator_new("\\" ,  -2,  0,1); // \try

		operator_new("--",   0,  0,1);
		operator_new("--",   0,  1,0);
		operator_new("++",   0,  0,1);
		operator_new("++",   0,  1,0);
		operator_new("~" ,   0,  1,0);
		operator_new("~~",   0,  1,0);
		operator_new("?" ,   0,  0,1);
		operator_new("?" ,   0,  1,0);

		operator_new("<-=",  2,  1,1);
		operator_new("->=",  2,  1,1);
		operator_new("<<=",  2,  1,1);
		operator_new(">>=",  2,  1,1);
		operator_new("~>=",  2,  1,1);
		operator_new("><=",  2,  1,1);

		operator_new("&=",   3,  1,1);

		operator_new("|=",   4,  1,1);
		operator_new("^=",   4,  1,1);

		operator_new("*=",   5,  1,1);
		operator_new("**=",  5,  1,1);
		operator_new("/=",   5,  1,1);
		operator_new("//=",  5,  1,1);

		operator_new("+=",   6,  1,1);
		operator_new("-=",   6,  1,1);

		operator_new("#>=",  7,  1,1);
		operator_new("<#=",  7,  1,1);

		operator_new("<=",   8,  1,1);
		operator_new(">=",   8,  1,1);
		operator_new("<>=",  8,  1,1);
		operator_new("===",  8,  1,1);
		operator_new("=<=",  8,  1,1);
		operator_new("=>=",  8,  1,1);


		operator_new("and=", 10, 1,1);

		operator_new("or=",  11, 1,1);

		operator_new(":=",   12, 1,1);

		operator_new("..",   12.5,1,1);
		operator_new(",",    13, 1,1);
		operator_new(":",    14, 1,-1);
		operator_new("",     15, 1,1);

	grammar_pop();

	grammar_push(grammar_new(grammar->symbols, NULL, NULL, 0, 0));

		//operator_new("(",   -10, 1,2); // function args
		operator_new("(",    10, 0,2);
		operator_new(")",    10, 2,0);

		operator_new("[",  -10, 1,3)->grammar = pubblock->grammar; // array size
		operator_new(",",    0, 1,1);
		operator_new(":",    1, 0,1);
		operator_new("|",    1, 0,1);

		operator_new("",     15, 1,1);


		pubblock->headergrammar = grammar;
		priblock->headergrammar = grammar;
	grammar_pop();

	grammar_push(NULL);
		blockdef* datblock = blockdef_new("dat");

		grammar->haslabels = 1;

		operator_new("#" ,   0,  0,1); // check precedence of this
		operator_new(",",    13, 1,1);
		operator_new("",     15, 1,1);

		opcode_new("org", "");
		opcode_new("res", "");
		opcode_new("fit", "");

		opcode_new("byte", "");
		opcode_new("word", "");
		opcode_new("long", "");

		label_new("$");



		opcode_new("nop",            "xxxxxx_xxxx_0000_xxxxxxxxx_xxxxxxxxx");

		// hub
		opcode_new("rdbyte",         "000000_xx1x_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("wrbyte",         "000000_xx0x_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("rdword",         "000001_xx1x_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("wrword",         "000001_xx0x_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("rdlong",         "000010_xx1x_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("wrlong",         "000010_xx0x_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("hubop",          "000011_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("clkset",         "000011_xxx1_yyyy_aaaaaaaaa_xxxxxx000");
		opcode_new("cogid",          "000011_xxy1_yyyy_aaaaaaaaa_xxxxxx001");
		opcode_new("coginit",        "000011_xxx1_yyyy_aaaaaaaaa_xxxxxx010");
		opcode_new("cogstop",        "000011_xxx1_yyyy_aaaaaaaaa_xxxxxx011");
		opcode_new("locknew",        "000011_xxy1_yyyy_aaaaaaaaa_xxxxxx100");
		opcode_new("lockret",        "000011_xxx1_yyyy_aaaaaaaaa_xxxxxx101");
		opcode_new("lockset",        "000011_xxx1_yyyy_aaaaaaaaa_xxxxxx110");
		opcode_new("lockclr",        "000011_xxx1_yyyy_aaaaaaaaa_xxxxxx111");

		// unused
		/*
		opcode_new("mul",            "000100_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("muls",           "000101_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("enc",            "000110_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("ones",           "000111_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		*/

		// shifter
		opcode_new("ror",            "001000_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("rol",            "001001_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("shr",            "001010_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("shl",            "001011_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("rcr",            "001100_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("rcl",            "001101_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("sar",            "001110_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("rev",            "001111_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");

		opcode_new("mins",           "010000_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("maxs",           "010001_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("min",            "010010_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("max",            "010011_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");

		opcode_new("movs",           "010100_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("movd",           "010101_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("movi",           "010110_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");

		opcode_new("jmp",            "010111_xxxx_yyyy_xxxxxxxxx_aaaaaaaaa");
		opcode_new("ret",            "010111_xx01_yyyy_xxxxxxxxx_xxxxxxxxx");
		opcode_new("call",           "010111_xx11_yyyy_xxxxxxxxx_xxxxxxxxx");
		opcode_new("jmpret",         "010111_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");

		opcode_new("and",            "011000_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("test",           "011000_xxxx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("andn",           "011001_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("testn",          "011001_xxxx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("or",             "011010_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("xor",            "011011_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");

		opcode_new("muxc",           "011100_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("muxnc",          "011101_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("muxz",           "011110_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("muxnz",          "011111_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");

		opcode_new("add",            "100000_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("sub",            "100001_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("cmp",            "100001_xxxx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("addabs",         "100010_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("subabs",         "100011_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");

		opcode_new("sumc",           "100100_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("sumnc",          "100101_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("sumz",           "100110_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("sumnz",          "100111_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");

		opcode_new("mov",            "101000_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("neg",            "101001_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("abs",            "101010_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("absneg",         "101011_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");

		opcode_new("negc",           "101100_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("negnc",          "101101_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("negz",           "101110_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("negnz",          "101111_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");

		opcode_new("cmps",           "110000_xxxx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("cmpsx",          "110001_xxxx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("addx",           "110010_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("subx",           "110011_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("cmpx",           "110011_xxxx_yyyy_aaaaaaaaa_bbbbbbbbb");

		opcode_new("adds",           "110100_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("subs",           "110101_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("addsx",          "110110_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("subsx",          "110111_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");

		opcode_new("cmpsub",         "111000_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("djnz",           "111001_xxyx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("tjnz",           "111010_xxxx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("tjz",            "111011_xxxx_yyyy_aaaaaaaaa_bbbbbbbbb");

		opcode_new("waitpeq",        "111100_xxxx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("waitpne",        "111101_xxxx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("waitcnt",        "111110_xxxx_yyyy_aaaaaaaaa_bbbbbbbbb");
		opcode_new("waitvid",        "111111_xxxx_yyyy_aaaaaaaaa_bbbbbbbbb");



		modifier_new("if_never",     "xxxxxx_xxxx_0000_xxxxxxxxx_xxxxxxxxx");

		modifier_new("if_nc_and_nz", "xxxxxx_xxxx_0001_xxxxxxxxx_xxxxxxxxx");
		modifier_new("if_nz_and_nc", "xxxxxx_xxxx_0001_xxxxxxxxx_xxxxxxxxx");
		modifier_new("if_a        ", "xxxxxx_xxxx_0001_xxxxxxxxx_xxxxxxxxx");

		modifier_new("if_nc_and_z",  "xxxxxx_xxxx_0010_xxxxxxxxx_xxxxxxxxx");
		modifier_new("if_z_and_nc",  "xxxxxx_xxxx_0010_xxxxxxxxx_xxxxxxxxx");

		modifier_new("if_nc",        "xxxxxx_xxxx_0011_xxxxxxxxx_xxxxxxxxx");
		modifier_new("if_ae",        "xxxxxx_xxxx_0011_xxxxxxxxx_xxxxxxxxx");

		modifier_new("if_c_and_nz",  "xxxxxx_xxxx_0100_xxxxxxxxx_xxxxxxxxx");
		modifier_new("if_nz_and_c",  "xxxxxx_xxxx_0100_xxxxxxxxx_xxxxxxxxx");

		modifier_new("if_nz",        "xxxxxx_xxxx_0101_xxxxxxxxx_xxxxxxxxx");
		modifier_new("if_ne",        "xxxxxx_xxxx_0101_xxxxxxxxx_xxxxxxxxx");

		modifier_new("if_c_ne_z",    "xxxxxx_xxxx_0110_xxxxxxxxx_xxxxxxxxx");
		modifier_new("if_z_ne_c",    "xxxxxx_xxxx_0110_xxxxxxxxx_xxxxxxxxx");

		modifier_new("if_nc_or_nz",  "xxxxxx_xxxx_0111_xxxxxxxxx_xxxxxxxxx");
		modifier_new("if_nz_or_nc",  "xxxxxx_xxxx_0111_xxxxxxxxx_xxxxxxxxx");

		modifier_new("if_c_and_z",   "xxxxxx_xxxx_1000_xxxxxxxxx_xxxxxxxxx");
		modifier_new("if_z_and_c",   "xxxxxx_xxxx_1000_xxxxxxxxx_xxxxxxxxx");

		modifier_new("if_c_eq_z",    "xxxxxx_xxxx_1001_xxxxxxxxx_xxxxxxxxx");
		modifier_new("if_z_eq_c",    "xxxxxx_xxxx_1001_xxxxxxxxx_xxxxxxxxx");

		modifier_new("if_z",         "xxxxxx_xxxx_1010_xxxxxxxxx_xxxxxxxxx");
		modifier_new("if_e",         "xxxxxx_xxxx_1010_xxxxxxxxx_xxxxxxxxx");

		modifier_new("if_z_or_nc",   "xxxxxx_xxxx_1011_xxxxxxxxx_xxxxxxxxx");
		modifier_new("if_nc_or_z",   "xxxxxx_xxxx_1011_xxxxxxxxx_xxxxxxxxx");

		modifier_new("if_c",         "xxxxxx_xxxx_1100_xxxxxxxxx_xxxxxxxxx");
		modifier_new("if_b",         "xxxxxx_xxxx_1100_xxxxxxxxx_xxxxxxxxx");

		modifier_new("if_c_or_nz",   "xxxxxx_xxxx_1101_xxxxxxxxx_xxxxxxxxx");
		modifier_new("if_nz_or_c",   "xxxxxx_xxxx_1101_xxxxxxxxx_xxxxxxxxx");

		modifier_new("if_c_or_z",    "xxxxxx_xxxx_1110_xxxxxxxxx_xxxxxxxxx");
		modifier_new("if_z_or_c",    "xxxxxx_xxxx_1110_xxxxxxxxx_xxxxxxxxx");
		modifier_new("if_be",        "xxxxxx_xxxx_1110_xxxxxxxxx_xxxxxxxxx");

		modifier_new("if_always",    "xxxxxx_xxxx_1111_xxxxxxxxx_xxxxxxxxx");

		modifier_new("wz",           "xxxxxx_1xxx_xxxx_xxxxxxxxx_xxxxxxxxx");
		//modifier_new("nz",           "xxxxxx_0xxx_xxxx_xxxxxxxxx_xxxxxxxxx");
		modifier_new("wc",           "xxxxxx_x1xx_xxxx_xxxxxxxxx_xxxxxxxxx");
		//modifier_new("nc",           "xxxxxx_x0xx_xxxx_xxxxxxxxx_xxxxxxxxx");
		modifier_new("wr",           "xxxxxx_xx1x_xxxx_xxxxxxxxx_xxxxxxxxx");
		modifier_new("nr",           "xxxxxx_xx0x_xxxx_xxxxxxxxx_xxxxxxxxx");

	grammar_pop();

	return conblock->grammar;
}
