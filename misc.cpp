#include <cstdio>

#include "misc.hpp"

Path::Path(const char* path)
{
	char* path2 = realpath(path, NULL);

	assign(path2);

	free(path2);
}
