#include <cstdio>

#include "misc.hpp"

Path::Path(const char* path)
{
	char* path2 = realpath(path, NULL);

	if (path2 == NULL)
	{
		throw "Path not found!";
	}

	assign(path2);

	free(path2);
}
