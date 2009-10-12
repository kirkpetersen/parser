# parser makefile

CPPFLAGS = -Wall -Werror -g

all:	parser test

parser:	main.cc parser.cc parser.hh

test:	test.cc parser.cc parser.hh
