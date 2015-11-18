CFLAGS=-g -Og -std=c99
#-fsanitize=address -fno-omit-frame-pointer
LDFLAGS=-lm

CC=gcc
LD=gcc

all:		pasm

clean:
		rm -vf *.o pasm

pasm:		main.o parser.o parserlib.o stack.o
		${LD} ${LDFLAGS} $^ -o $@

%.o:		%.c
		${CC} ${CFLAGS} $< -c -o $@

depend:
		$(CC) $(CCFLAGS) -MM main.c {parser,parserlib,stack}.{c,h} parallax_types.h

.PHONY:		all depend clean

main.o: main.c parser.h parallax_types.h stack.h parserlib.h opcodes.c
parser.o: parser.c parser.h parallax_types.h stack.h parserlib.h
parser.o: parser.h parallax_types.h stack.h
parserlib.o: parserlib.c parserlib.h parallax_types.h
parserlib.o: parserlib.h parallax_types.h
stack.o: stack.c stack.h
stack.o: stack.h
parallax_types.o: parallax_types.h

