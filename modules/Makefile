#CFLAGS=-std=c99 -Og -g -fpermissive -fPIC -rdynamic -I../
CXXFLAGS=-std=c++11 -Og -g -fpermissive -fPIC -rdynamic -I../
LDFLAGS=-lm -lpthread -ldl -rdynamic -shared
CC=gcc
CXX=g++
LD=g++

.PHONY:		all clean modules

all:		modules

clean:		
		rm -fv *.o *.so

modules:	spinc.so

%.so:		%.o ../module.o ../registry.o
		${LD} -o $@ $^ ${LDFLAGS}

#%.o:		%.c %.h
#		${CC} ${CFLAGS} -c -o $@ $<

%.o:		%.cpp %.hpp
		${CXX} ${CXXFLAGS} -c -o $@ $<
