# parser makefile

CPPFLAGS = -Wall -Werror -g -O3
LDFLAGS = 

all:	parser

popt:	parser.popt

%.po : CPPFLAGS += -pg
%.popt : LDFLAGS += -pg

parser.o:	parser.cc parser.hh
test.o:	test.cc parser.hh
main.o:	main.cc parser.hh

parser.po:	parser.cc parser.hh
	g++ $(CPPFLAGS) -o $@ -c $<

main.po:	main.cc parser.hh
	g++ $(CPPFLAGS) -o $@ -c $<

test:	test.o parser.o
	g++ $(LDFLAGS) -o test test.o parser.o

parser:	main.o parser.o
	g++ $(LDFLAGS) -o parser main.o parser.o

parser.popt:	main.po parser.po
	g++ $(LDFLAGS) -o parser.popt main.po parser.po

clean:
	$(RM) test parser parser.opt parser.popt *.o *.oo *.po
