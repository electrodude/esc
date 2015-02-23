#CFLAGS=-std=c99 -Og -g -fpermissive -fPIC -rdynamic -I../
CXXFLAGS=-std=c++11 -Og -g -fpermissive -fPIC -rdynamic -I../../
LDFLAGS=../../module.o ../../registry.o -lm -rdynamic -shared
CC=gcc
CXX=g++
LD=g++

.PHONY:		all clean

all:		spinc.so

clean:		
		rm -vf *.o *.so

%.so:		%.o
		@#echo `realpath ../../module.so`
		${LD} -o $@ $^ ${LDFLAGS}

#%.o:		%.c %.h
#		${CC} ${CFLAGS} -c -o $@ $<

%.o:		%.cpp %.hpp
		${CXX} ${CXXFLAGS} -c -o $@ $<

