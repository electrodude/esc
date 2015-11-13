CFLAGS=-g -Og -std=c99
#-fsanitize=address -fno-omit-frame-pointer
LDFLAGS=

CC=gcc
LD=gcc

all:		pasm

clean:		
		rm -vf *.o pasm

pasm:		main.o parser.o parserlib.o stack.o
		${LD} ${LDFLAGS} $^ -o $@

%.o:		%.c
		${CC} ${CFLAGS} $< -c -o $@

.PHONY:		all clean

