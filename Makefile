# parser makefile

CPPFLAGS = -Wall -Werror -g
CFLAGS = -Wall -Werror -g
LDFLAGS = 

all:	parser cparser test

popt:	parser.popt

%.po : CPPFLAGS += -pg
%.po : CFLAGS += -pg
%.popt : LDFLAGS += -pg

# C++ object files
parser.o:	parser.cc parser.hh parser_c.h
test.o:	test.cc parser.hh parser_c.h
main.o:	main.cc parser.hh parser_c.h

# C object files
tree_node.o:	tree_node.c parser_c.h
cmain.o:	cmain.c parser_c.h

parser.po:	parser.cc parser.hh parser_c.h
	g++ $(CPPFLAGS) -o $@ -c $<

main.po:	main.cc parser.hh parser_c.h
	g++ $(CPPFLAGS) -o $@ -c $<

tree_node.po:	tree_node.c parser_c.h
	gcc $(CFLAGS) -o $@ -c $<

test:	test.o parser.o tree_node.o
	g++ $(LDFLAGS) -o $@ $^

parser:	main.o parser.o tree_node.o
	g++ $(LDFLAGS) -o $@ $^

cparser:	cmain.o parser.o tree_node.o
	g++ $(LDFLAGS) -o $@ $^

parser.popt:	main.po parser.po tree_node.po
	g++ $(LDFLAGS) -o parser.popt main.po parser.po tree_node.po

clean:
	$(RM) test parser parser.opt parser.popt *.o *.oo *.po
