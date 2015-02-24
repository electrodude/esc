#CFLAGS=-std=c99 -Og -g -fpermissive -fPIC -rdynamic -I../
CXXFLAGS=-std=c++11 -Og -g -fpermissive -fPIC -rdynamic -I../../
LDFLAGS=../../module.a -lm -rdynamic -shared
CC=gcc
CXX=g++
LD=g++
BLOCKS=conblock.o objblock.o varblock.o pubblock.o priblock.o datblock.o

.PHONY:		all clean

.PRECIOUS:	%.o

all:		spinc.so

clean:		
		rm -vf *.o *.so

%.so:		%.o ${BLOCKS}
		@#echo `realpath ../../module.a`
		${LD} -o $@ $^ ${LDFLAGS}

#%.o:		%.c %.h
#		${CC} ${CFLAGS} -c -o $@ $<

%.o:		%.cpp %.hpp
		${CXX} ${CXXFLAGS} -c -o $@ $<

