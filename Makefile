CXXFLAGS=-g -Og
#-fsanitize=address -fno-omit-frame-pointer
LDFLAGS=-lm

CXX=g++
LD=g++

all:		esc

clean:
		rm -vf *.o esc

esc:		main.o compiler.o parser.o parserlib.o bitfield.o opcodes.o
		${LD} ${LDFLAGS} $^ -o $@

%.o:		%.cpp
		${CXX} ${CXXFLAGS} $< -c -o $@

depend:
		$(CXX) $(CXXFLAGS) -MM {main,compiler,parser,parserlib,bitfield}.cpp parallax_types.hpp opcodes.cpp

.PHONY:		all depend clean

main.o: main.cpp parserlib.hpp parallax_types.hpp bitfield.hpp parser.hpp \
 compiler.hpp
compiler.o: compiler.cpp parserlib.hpp parallax_types.hpp bitfield.hpp \
 parser.hpp compiler.hpp
parser.o: parser.cpp parser.hpp parallax_types.hpp parserlib.hpp \
 bitfield.hpp
parserlib.o: parserlib.cpp parserlib.hpp parallax_types.hpp bitfield.hpp
bitfield.o: bitfield.cpp bitfield.hpp parallax_types.hpp
parallax_types.o: parallax_types.hpp
opcodes.o: opcodes.cpp parserlib.hpp parallax_types.hpp bitfield.hpp
