CFLAGS=-std=c99 -Og -g -fpermissive -fPIC -rdynamic
CXXFLAGS=-std=c++11 -Og -g -fpermissive -fPIC -rdynamic
LDFLAGS=-lm -lpthread -ldl -rdynamic
SOFLAGS=-shared
CC=gcc
CXX=g++
LD=g++

all:		main spinc.so

main:		main.o loader.o module.o registry.o
		${LD} -o $@ $^ ${LDFLAGS}

main.hpp:	loader.hpp

loader.hpp:	module.hpp

module.hpp:	registry.hpp

%.so:		%.o module.o registry.o
		${LD} -o $@ $^ ${LDFLAGS} ${SOFLAGS}

%.o:		%.c %.h
		${CC} ${CFLAGS} -c -o $@ $<

%.o:		%.cpp %.hpp
		${CXX} ${CXXFLAGS} -c -o $@ $<
