# parser makefile

CPPFLAGS = -Wall -Werror -g

all:	parser test

parser.o:	parser.cc parser.hh
test.o:	test.cc parser.hh
main.o:	main.cc parser.hh

test:	test.o parser.o
	g++ -o test test.o parser.o

parser:	main.o parser.o
	g++ -o parser main.o parser.o

clean:
	$(RM) test parser *.o
