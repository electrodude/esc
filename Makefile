#CFLAGS=-std=c99 -Og -g -fpermissive -fPIC
CXXFLAGS=-std=c++11 -Og -g -fpermissive -fPIC
LDFLAGS=-lm -lpthread -ldl
CC=gcc
CXX=g++
LD=g++

.PHONY:		all clean modules

all:		main modules

clean:
		rm -fv *.o main
		make -C modules clean

modules:	
		make -C modules all

main:		main.o loader.o module.o registry.o
		${LD} -o $@ $^ ${LDFLAGS}

main.hpp:	loader.hpp

loader.hpp:	module.hpp

module.hpp:	registry.hpp

#%.o:		%.c %.h
#		${CC} ${CFLAGS} -c -o $@ $<

%.o:		%.cpp %.hpp
		${CXX} ${CXXFLAGS} -c -o $@ $<
