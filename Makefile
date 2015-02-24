#CFLAGS=-std=c99 -Og -g -fpermissive -fPIC
CXXFLAGS=-std=c++11 -Og -g -fpermissive -fPIC
LDFLAGS=-lm
EXFLAGS=-ldl
SOFLAGS=-rdynamic -shared
CC=gcc
CXX=g++
LD=g++
AR=ar

.PHONY:		all clean modules

all:		main modules

clean:
		rm -vf *.o main module.so
		make -C modules clean

modules:	module.a
		make -C modules all

main:		main.o loader.o compiler.o symtab.o misc.o module.a
		${LD} -o $@ $^ ${LDFLAGS} ${EXFLAGS}

module.a:	module.o registry.o
		${AR} rcs $@ $^

main.hpp:	loader.hpp

module.a:	loader.hpp

compiler.hpp:	misc.hpp symtab.hpp

loader.hpp:	module.hpp

module.hpp:	registry.hpp

#%.o:		%.c %.h
#		${CC} ${CFLAGS} -c -o $@ $<

%.o:		%.cpp %.hpp
		${CXX} ${CXXFLAGS} -c -o $@ $<
