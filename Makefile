#CFLAGS=-std=c99 -Og -g -fpermissive -fPIC
CXXFLAGS=-std=c++11 -Og -g -fpermissive -fPIC
LDFLAGS=-lm
EXFLAGS=-ldl
SOFLAGS=-rdynamic -shared
CC=gcc
CXX=g++
LD=g++

.PHONY:		all clean modules

all:		main modules

clean:
		rm -vf *.o main module.so
		make -C modules clean

modules:	module.so
		make -C modules all

main:		main.o loader.o module.o registry.o
		${LD} -o $@ $^ ${LDFLAGS} ${EXFLAGS}

module.so:	module.o registry.o
		${LD} -o $@ $^ ${LDFLAGS} ${SOFLAGS}

main.hpp:	loader.hpp

module.so:	loader.hpp

loader.hpp:	module.hpp

module.hpp:	registry.hpp

#%.o:		%.c %.h
#		${CC} ${CFLAGS} -c -o $@ $<

%.o:		%.cpp %.hpp
		${CXX} ${CXXFLAGS} -c -o $@ $<
