CFLAGS=-g -Og -std=c99
#-fsanitize=address -fno-omit-frame-pointer
LDFLAGS=-lm

CC=gcc
LD=gcc

all:		esc

clean:
		rm -vf *.o esc

esc:		main.o compiler.o parser.o parserlib.o stack.o bitfield.o opcodes.o
		${LD} ${LDFLAGS} $^ -o $@

%.o:		%.c
		${CC} ${CFLAGS} $< -c -o $@

depend:
		$(CC) $(CCFLAGS) -MM {main,compiler,parser,parserlib,stack,bitfield}.c parallax_types.h opcodes.c

.PHONY:		all depend clean

main.o: main.c stack.h parserlib.h parallax_types.h bitfield.h parser.h \
 compiler.h
compiler.o: compiler.c stack.h parserlib.h parallax_types.h bitfield.h \
 parser.h compiler.h
parser.o: parser.c parser.h parallax_types.h stack.h parserlib.h \
 bitfield.h
parserlib.o: parserlib.c parserlib.h parallax_types.h bitfield.h stack.h
stack.o: stack.c stack.h
bitfield.o: bitfield.c bitfield.h parallax_types.h
parallax_types.o: parallax_types.h
opcodes.o: opcodes.c parserlib.h parallax_types.h bitfield.h stack.h
