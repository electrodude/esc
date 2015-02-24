#include "spinc.hpp"

SpinCompiler::SpinCompiler(CompilerRegistry& registry)
{
	printf("Spin compiler module loading!\n");
	registry(new CONBlockFactory());
	registry(new OBJBlockFactory());
	registry(new VARBlockFactory());
	registry(new PUBBlockFactory());
	registry(new PRIBlockFactory());
	registry(new DATBlockFactory());

}


extern "C" CompilerModule* loadModule(CompilerRegistry& registry)
{
	return new SpinCompiler(registry);
}

extern "C" void unloadModule(CompilerModule* mod)
{
	delete mod;
}
