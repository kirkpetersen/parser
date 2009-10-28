/*
 * misc.cc
 */

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include "parser.hh"

// C interface

struct tree_node * parser_run(const char * grammar, const char * input)
{
    parser p;

    p.load(grammar);

    if(input) {
	std::ifstream fs(input, std::fstream::in);

	p.run(fs);

	fs.close();
    } else {
	p.run(std::cin);
    }

    return p.tree();
}

// C++ code

std::ostream & operator<<(std::ostream & out, const symbol & s)
{
    if(s.value.empty()) {
	out << "[" << s.type << "]";
    } else {
	out << "[" << s.type << "," << s.value << "]";
    }

    return out;
}

bool parser_item_compare::operator()(const parser_item * p1,
				     const parser_item * p2) const
{
    const bool i = p1->index == p2->index;

    if(i) {
	const bool h = p1->head == p2->head;

	if(h) {
	    const bool s = p1->symbols == p2->symbols;

	    if(s) {
		if(p1->terminal < p2->terminal) {
		    return true;
		}

	    } else if(p1->symbols < p2->symbols) {
		return true;
	    }
	} else if(p1->head < p2->head) {
	    return true;
	}
    } else if(p1->index < p2->index) {
	return true;
    }

    return false;
}
