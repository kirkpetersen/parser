/*
 * dump.cc
 */

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include "parser.hh"

void grammar::dump(void) const
{
    for(unsigned i = 0; i < table_size; i++) {
	for(grammar_node * g = table[i]; g; g = g->next) {
	    parser_rule_list & prl = table[i]->list;
	    parser_rule_iter ri;

	    // This handles multiple productions with the same head
	    for(ri = prl.begin(); ri != prl.end(); ++ri) {
		const parser_rule * rule = *ri;
		unsigned size = rule->symbols.size();

		std::cout << " " << rule->head << " -> ";

		// Iterates through the symbols
		for(unsigned i = 0; i < size; i++) {
		    const std::string & s = rule->symbols[i];

		    std::cout << s;

		    if(i < size - 1) {
			std::cout << ' ';
		    }
		}

		std::cout << '\n';
	    }
	}
    }

    return;
}

void parser::dump_set(const char * msg, const std::set<std::string> & rs)
{
    std::cout << msg;

    std::set<std::string>::const_iterator si;

    for(si = rs.begin(); si != rs.end(); ++si) {
	std::cout << *si << " ";
    }

    std::cout << '\n';

    return;
}

void parser::dump_stats(void)
{
    std::cout << "parser stats:\n";
    std::cout << "build item: " << stats.build_item << '\n';
    std::cout << "build item skips: " << stats.build_item_skips << '\n';
    std::cout << "loops: " << stats.loops << '\n';
    std::cout << "shifts: " << stats.shifts << '\n';
    std::cout << "reduces: " << stats.reduces << '\n';
    std::cout << "accepts: " << stats.accepts << '\n';
    std::cout << "build_items_calls: " << stats.build_items_calls << '\n';
    std::cout << "closure_calls: " << stats.closure_calls << '\n';
    std::cout << "closure_loops: " << stats.closure_loops << '\n';
    std::cout << "closure_item_duplicates: " << stats.closure_item_duplicates << '\n';
    std::cout << "closure_item_non_duplicates: " << stats.closure_item_non_duplicates << '\n';
    std::cout << "closure_skips: " << stats.closure_skips << '\n';

    return;
}

void parser::dump_grammar(void)
{
    dump_set("tokens: ", tokens);
    dump_set("literals: ", literals);

    std::cout << "productions:\n";

    productions.dump();

    return;
}

void parser::dump(void)
{
    const char * d = "debug";
    std::string s1 = "", s2 = "";

    dump(d, s1, s2);

    return;
}

void parser::dump(const char * msg, std::string & t, std::string & tv)
{
    if(msg) {
	std::cout << "[" << msg << "]\n";
    }

    if(verbose > 5) {
	dump_grammar();
    }

    std::cout << "parser state:\n";

    std::cout << "current token: " << t << ", value: " << tv << '\n';

    std::cout << " symbol stack: ";

    std::deque<std::string>::const_iterator sy;

    for(sy = symbol_stack.begin(); sy != symbol_stack.end(); ++sy) {
	std::cout << *sy << ' ';
    }

    std::cout << "\n";

    std::deque<parser_state *>::const_iterator st;

    unsigned sn = 0;

    if(verbose > 3) {
	std::cout << " state stack:\n";

	for(st = state_stack.begin(); st != state_stack.end(); ++st) {
	    const parser_state * ps = *st;

	    std::cout << "  state " << sn++ << '\n';

	    dump_state(ps, 3);
	}
    } else {
	std::cout << " state stack (kernel items of top state only):\n";

	if(!state_stack.empty()) {
	    const parser_state * ps = state_stack.back();

	    std::cout << "  state " << state_stack.size() - 1 << '\n';

	    dump_state(ps, 3);
	}
    }

    if(verbose > 3) {
	std::cout << " parse tree:\n";

	if(node_stack.size() > 0) {
	    tree_node_dump(node_stack.back(), 2);
	}
    }

    if(verbose > 4) {
	dump_stats();
    }

    return;
}

void parser::dump_state(const parser_state * ps, unsigned spaces) {
    parser_item_iter li;

    for(unsigned i = 0; i < spaces; i++) { std::cout << ' '; }

    std::cout << "kernel items:\n";

    for(li = ps->kernel_items.begin();
	li != ps->kernel_items.end(); ++li) {
	const parser_item * pi = *li;

	dump_item(pi, spaces + 1);
    }

    if(verbose > 3) {
	for(unsigned i = 0; i < spaces; i++) { std::cout << ' '; }

	std::cout << "nonkernel items:\n";

	for(li = ps->nonkernel_items.begin();
	    li != ps->nonkernel_items.end(); ++li) {
	    const parser_item * pi = *li;

	    dump_item(pi, spaces + 1);
	}
    }

    return;
}

void parser::dump_item(const parser_item * pi, unsigned spaces) {
    for(unsigned i = 0; i < spaces; i++) { std::cout << ' '; }

    std::cout << pi->rule->head << " -> ";

    unsigned size = pi->rule->symbols.size();

    for(unsigned i = 0; i < size; i++) {
	if(i == pi->index) {
	    std::cout << ". ";
	}

	std::cout << pi->rule->symbols[i] << " ";
    }

    if(pi->index == size) {
	std::cout << ". ";
    }

    std::cout << "{" << pi->lookahead << "}\n";

    return;
}
