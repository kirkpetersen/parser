# parser makefile

CPPFLAGS = -Wall -Werror -g -pg -O
LDFLAGS = -pg

all:	parser test

parser.o:	parser.cc parser.hh
test.o:	test.cc parser.hh
main.o:	main.cc parser.hh

test:	test.o parser.o
	g++ $(LDFLAGS) -o test test.o parser.o

parser:	main.o parser.o
	g++ $(LDFLAGS) -o parser main.o parser.o

clean:
	$(RM) test parser *.o
