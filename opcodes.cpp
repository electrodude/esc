#include <cstdlib>

#include "parserlib.hpp"

Grammar* register_spin(void)
{
	// basic universal Spin operators

	// grammar object containing things legal in constant expressions
	Grammar* grammar_global = grammar;

	// . and #
	new Operator("." ,  -200,  1,1);
	Operator* op_constant = new Operator("#" ,  -11,  1,1);

	op_constant->lefttypes.acceptsLiteral = false;
	op_constant->righttypes.acceptsLiteral = false;

	new Operator("~" ,   0,  0,1);
	new Operator("~~",   0,  0,1);

	new Operator("@" ,   0,  0,1);
	new Operator("@@",   0,  0,1);

	new Operator("+",    1,  0,1);
	new Operator("-",    1,  0,1);
	new Operator("^^",   1,  0,1);
	new Operator("||",   1,  0,1);
	new Operator("|<",   1,  0,1);
	new Operator(">|",   1,  0,1);
	new Operator("!",    1,  0,1);

	new Operator("<-",   2,  1,1);
	new Operator("->",   2,  1,1);
	new Operator("<<",   2,  1,1);
	new Operator(">>",   2,  1,1);
	new Operator("~>",   2,  1,1);
	new Operator("><",   2,  1,1);

	new Operator("&",    3,  1,1);

	new Operator("|",    4,  1,1);
	new Operator("^",    4,  1,1);

	new Operator("*",    5,  1,1);
	new Operator("**",   5,  1,1);
	new Operator("/",    5,  1,1);
	new Operator("//",   5,  1,1);

	new Operator("+",    6,  1,1);
	new Operator("-",    6,  1,1);

	new Operator("#>",   7,  1,1);
	new Operator("<#",   7,  1,1);

	new Operator("<",    8,  1,1);
	new Operator(">",    8,  1,1);
	new Operator("<>",   8,  1,1);
	new Operator("==",   8,  1,1);
	new Operator("=<",   8,  1,1);
	new Operator("=>",   8,  1,1);


	new Operator("not",  9,  0,1);

	new Operator("and",  10, 1,1);

	new Operator("or",   11, 1,1);

	Operator* op_funccall = new Operator("(", -100, 1,2); // function call
	new Operator("(", -100, 0,2);
	new Operator(")",  100, 2,0);

	Operator* op_arrayidx = new Operator("[", -100, 1,3); // array index
	//new Operator("[", -100, 0,3); // unused [val] notation
	new Operator("]", 100, 3,0);

	Grammar::push(new Grammar());
		BlockDef* objblock = new BlockDef("obj");

		new Operator(":",    16, 1,1);

		new Operator("[",  -100, 1,3, grammar_global); // instance count

	Grammar::pop();



	Grammar::push();
		BlockDef* conblock = new BlockDef("con");

		new Operator("=",    12, 1,1);
		new Operator(",",    13, 1,1);
		new Operator("#" ,   14,  0,1); // check precedence of this

	Grammar::pop();

	Grammar::push(new Grammar());
		BlockDef* varblock = new BlockDef("var");

		new Operator("long", 14.5, 0,1);
		new Operator("word", 14.5, 0,1);
		new Operator("byte", 14.5, 0,1);

		new Operator(",",    13, 1,1);

		Operator* op_var_array = new Operator("[", -100, 1,3); // array size
		new Operator("]",  100, 3,0);

	Grammar::pop();

	op_var_array->localgrammar = grammar;

	Grammar::push();
		BlockDef* pubblock = new BlockDef("pub");
		BlockDef* priblock = new BlockDef("pri");

		grammar->hasindent = 1;

		new Operator("if",        20,  0,1);
		new Operator("ifnot",     20,  0,1);
		new Operator("elseif",    20,  0,1);
		new Operator("elseifnot", 20,  0,1);
		new Operator("else",      20,  0,0);

		new Operator("repeat",    20,  0,1);
		new Operator("repeat",    20,  0,0);
		new Operator("from",      20,  1,1);
		new Operator("to",        20,  1,1);
		new Operator("step",      20,  1,1);
		new Operator("until",     20,  0,1);
		new Operator("while",     20,  0,1);
		new Operator("next",      20,  0,0);
		new Operator("quit",      20,  0,0);

		new Operator("case",      20,  0,1);

		new Operator("return",    20,  0,1);
		new Operator("return",    20,  0,0);

		new Operator("abort",     20,  0,1);
		new Operator("abort",     20,  0,0);



		new Operator("\\" ,  -2,  0,1); // \try

		new Operator("--",   0,  0,1);
		new Operator("--",   0,  1,0);
		new Operator("++",   0,  0,1);
		new Operator("++",   0,  1,0);
		new Operator("~" ,   0,  1,0);
		new Operator("~~",   0,  1,0);
		new Operator("?" ,   0,  0,1);
		new Operator("?" ,   0,  1,0);

		new Operator("<-=",  2,  1,1);
		new Operator("->=",  2,  1,1);
		new Operator("<<=",  2,  1,1);
		new Operator(">>=",  2,  1,1);
		new Operator("~>=",  2,  1,1);
		new Operator("><=",  2,  1,1);

		new Operator("&=",   3,  1,1);

		new Operator("|=",   4,  1,1);
		new Operator("^=",   4,  1,1);

		new Operator("*=",   5,  1,1);
		new Operator("**=",  5,  1,1);
		new Operator("/=",   5,  1,1);
		new Operator("//=",  5,  1,1);

		new Operator("+=",   6,  1,1);
		new Operator("-=",   6,  1,1);

		new Operator("#>=",  7,  1,1);
		new Operator("<#=",  7,  1,1);

		new Operator("<=",   8,  1,1);
		new Operator(">=",   8,  1,1);
		new Operator("<>=",  8,  1,1);
		new Operator("===",  8,  1,1);
		new Operator("=<=",  8,  1,1);
		new Operator("=>=",  8,  1,1);


		new Operator("and=", 10, 1,1);

		new Operator("or=",  11, 1,1);

		new Operator(":=",   12, 1,1);

		new Operator("..",   12.5,1,1);
		new Operator(",",    13, 1,1);
		new Operator("",     21, 1,1);

		Grammar::push();
			new Operator(":",    14, 1,1);

			op_funccall->localgrammar = grammar;
		Grammar::pop();

		new Operator(":",    14, 1,0);

		op_arrayidx->localgrammar = grammar;
	Grammar::pop();

	Grammar::push(new Grammar());

		new Operator("(", -100, 1,2); // function args
		new Operator(")",  100, 2,0);

		new Operator("[", -100, 1,3); // array size
		new Operator("]",  100, 3,0);

		new Operator(",",    0, 1,1);
		new Operator(":",    1, 1,1);
		new Operator("|",    1, 1,1);

		new Operator("",     15, 1,1);


		pubblock->headgrammar = grammar;
		priblock->headgrammar = grammar;
	Grammar::pop();

	Grammar::push();
		BlockDef* datblock = new BlockDef("dat");

		grammar->haslabels = 1;

		new Operator("#" ,   0,  0,1); // check precedence of this
		new Operator(",",    13, 1,1);
		new Operator("",     15, 1,1);

		new Operator("long", 14.5, 0,1);
		new Operator("word", 14.5, 0,1);
		new Operator("byte", 14.5, 0,1);

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

		//modifier_new("wz",           "xxxxxx_1xxx_xxxx_xxxxxxxxx_xxxxxxxxx");
		//modifier_new("nz",           "xxxxxx_0xxx_xxxx_xxxxxxxxx_xxxxxxxxx");
		//modifier_new("wc",           "xxxxxx_x1xx_xxxx_xxxxxxxxx_xxxxxxxxx");
		//modifier_new("nc",           "xxxxxx_x0xx_xxxx_xxxxxxxxx_xxxxxxxxx");
		//modifier_new("wr",           "xxxxxx_xx1x_xxxx_xxxxxxxxx_xxxxxxxxx");
		//modifier_new("nr",           "xxxxxx_xx0x_xxxx_xxxxxxxxx_xxxxxxxxx");

	Grammar::pop();

	Grammar::putblocknames();

	return conblock->bodygrammar;
}
