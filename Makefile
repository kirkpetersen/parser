# parser makefile

CPPFLAGS = -Wall -Werror -g
CFLAGS = -Wall -Werror -g
LDFLAGS = 

all:	parser test parser.opt

%.o : CPPFLAGS += -pg
%.o : CFLAGS += -pg

%.oo : CPPFLAGS += -O3

# Headers
parser.hh:	parser_c.h

# C++ object files
parser.o:	parser.cc parser.hh
dump.o:	dump.cc parser.hh
misc.o:	misc.cc parser.hh
test.o:	test.cc parser.hh
main.o:	main.cc parser.hh

parser.oo:	parser.cc parser.hh
dump.oo:	dump.cc parser.hh
misc.oo:	misc.cc parser.hh
main.oo:	main.cc parser.hh

# C object files
tree_node.o:	tree_node.c parser_c.h

parser.oo:	parser.cc parser.hh
	g++ $(CPPFLAGS) -o $@ -c $<

dump.oo:	dump.cc parser.hh
	g++ $(CPPFLAGS) -o $@ -c $<

misc.oo:	misc.cc parser.hh
	g++ $(CPPFLAGS) -o $@ -c $<

main.oo:	main.cc parser.hh
	g++ $(CPPFLAGS) -o $@ -c $<

tree_node.oo:	tree_node.c parser_c.h
	gcc $(CFLAGS) -o $@ -c $<

test:	test.o parser.o dump.o misc.o tree_node.o
	g++ $(LDFLAGS) -o $@ $^

parser:	main.o parser.o dump.o misc.o tree_node.o
	g++ $(LDFLAGS) -o $@ $^

parser.opt:	main.oo parser.oo dump.oo misc.oo tree_node.oo
	g++ $(LDFLAGS) -o $@ $^

clean:
	$(RM) test parser parser.opt parser.popt *.o *.oo *.po gmon.out
