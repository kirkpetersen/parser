# parser makefile

CPPFLAGS = -Wall -Werror -g
LDFLAGS = 

all:	parser

popt:	parser.popt

%.po : CPPFLAGS += -pg
%.popt : LDFLAGS += -pg

parser.o:	parser.cc parser.hh
tree_node.o:	tree_node.c parser_c.h
test.o:	test.cc parser.hh
main.o:	main.cc parser.hh
cmain.o:	cmain.c parser_c.h

parser.po:	parser.cc parser.hh
	g++ $(CPPFLAGS) -o $@ -c $<

main.po:	main.cc parser.hh
	g++ $(CPPFLAGS) -o $@ -c $<

test:	test.o parser.o
	g++ $(LDFLAGS) -o $@ $^

parser:	main.o parser.o tree_node.o
	g++ $(LDFLAGS) -o $@ $^

cparser:	cmain.o parser.o tree_node.o
	g++ $(LDFLAGS) -o $@ $^

parser.popt:	main.po parser.po
	g++ $(LDFLAGS) -o parser.popt main.po parser.po

clean:
	$(RM) test parser parser.opt parser.popt *.o *.oo *.po
