#pragma once

#include <string>
#include <memory>
#include <map>
#include <vector>

class Namespace;

class Object
{

};

class Symbol
{
public:
	Namespace* owner;
	Object* target;
};

class Namespace : public Object
{
public:
	std::map<std::string, Symbol*> table;
};

class Context
{
public:
	std::vector<Namespace> tables;
};
