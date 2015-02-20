#include "loader.hpp"

#include "main.hpp"


int main(int argc, char** argv)
{
	module spinc = module("./spinc.so");

	printf("module: %016X\n", &spinc);

	CompilerModule* mod = spinc.getModule();

}
