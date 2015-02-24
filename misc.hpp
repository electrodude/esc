#include <string>

class Path : public std::string
{
public:
	Path(const char* path);
	Path(std::string path) : Path(path.c_str()) {}
};
