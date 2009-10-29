/*
 * dump.cc
 */

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include "parser.hh"

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

    std::map<std::string, std::list<parser_rule *> >::const_iterator mi;

    // For each production...
    for(mi = productions.begin(); mi != productions.end(); ++mi) {
	const std::string & h = mi->first;
	const std::list<parser_rule *> & l = mi->second;

	std::list<parser_rule *>::const_iterator li;

	// This handles multiple productions with the same head
	for(li = l.begin(); li != l.end(); ++li) {
	    const parser_rule * rule = *li;
	    unsigned size = rule->symbols.size();

	    std::cout << " " << h << " -> ";

	    std::list<std::string>::const_iterator li2;

	    // Iterates through the symbols
	    for(unsigned i = 0; i < size; i++) {
		const std::string & s = rule->symbols[i];

		if(terminal(s)) {
		    std::cout << "'" << s << "'";
		} else {
		    std::cout << s;
		}

		if(i < size - 1) {
		    std::cout << ' ';
		}
	    }

	    std::cout << '\n';
	}
    }

    return;
}

void parser::dump(const char * msg)
{
    if(msg) {
	std::cout << "[" << msg << "]\n";
    }

    if(verbose > 3) {
	dump_grammar();
    }

    std::cout << "parser state:\n";

    std::cout << "current token: " << token
	 << ", value: " << token_value << '\n';

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
    std::set<parser_item *>::const_iterator li;

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
	    // Add the dot
	    std::cout << ". ";
	}

	if(terminal(pi->rule->symbols[i])) {
	    std::cout << "'" << pi->rule->symbols[i] << "'";
	} else {
	    std::cout << pi->rule->symbols[i];
	}

	std::cout << " ";
    }

    if(pi->index == size) {
	std::cout << ".";
    }

    std::cout << " {" << pi->terminal << "}\n";

    return;
}
