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

    if (input) {
        std::ifstream fs(input, std::fstream::in);

        p.run(fs);

        fs.close();
    } else {
        p.run(std::cin);
    }

    return p.tree();
}

// C++ code

bool parser_item_compare::operator()(const parser_item * p1,
                                     const parser_item * p2) const
{
    if (p1->index == p2->index) {
        if (p1->rule->head == p2->rule->head) {
            if (p1->lookahead == p2->lookahead) {
                if (p1->rule->symbols < p2->rule->symbols) {
                    return true;
                }

            } else if (p1->lookahead < p2->lookahead) {
                return true;
            }
        } else if (p1->rule->head < p2->rule->head) {
            return true;
        }
    } else if (p1->index < p2->index) {
        return true;
    }

    return false;
}
