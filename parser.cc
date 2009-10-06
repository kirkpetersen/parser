/*
 * parser.cc
 */

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <list>
#include <map>
#include <queue>

using namespace std;

// parser item: symbols from production, current index into symbols
// parser state: list of kernel and list of non-kernel items
// 

class parser_item {
public:
    vector<string> symbols;
    unsigned index;

    parser_item(const list<string> & l);
};

parser_item::parser_item(const list<string> & l)
{
    list<string>::const_iterator li;
    unsigned i = 0;

    symbols.resize(l.size());

    for(li = l.begin(); li != l.end(); li++) {
	symbols[i++] = *li;
    }

    index = 0;

    return;
}

class parser_state {
public:
    list<parser_item> kernel_items;
    list<parser_item> nonkernel_items;
};

class parser {
    map<string, list<list<string> > > productions;

    list<parser_state> state_stack;
    list<string> symbol_stack;

public:

    void run(void);
    void closure(parser_state & ps);
    void dump(void);
    void read(void);
};

void parser::run(void)
{
    // states are kernel + non-kernel items

    // start symbol
    list<list<string> > start = productions[string("start")];

    if(start.empty()) {
	cerr << "no start state!" << endl;
	return;
    }

    parser_state ps;

    list<list<string> >::const_iterator li;

    for(li = start.begin(); li != start.end(); li++) {
	parser_item pi(*li);

	ps.kernel_items.push_back(pi);
    }

    closure(ps);

    // Create initial kernel item ("start") and push it onto the state stack

    // Grab the first token

    // Enter main loop
    // - look for reductions
    //   - if any "possible" items (?) are at the end, reduce
    //   - for N symbols in this item, pop off symbol stack
    //   - delete state off stack
    //   - get the top of state stack
    //   - push non-terminal onto symbol stack
    // - shift
    //   - find all possible matches for the token in current items
    //   - no matches means a parse error
    //   - shift new token/symbol onto stack
    //   - push new state (possible matches) onto state stack
    // - get next token and loop

    return;
}

void parser::closure(parser_state & ps)
{
    map<string, bool> added;
    list<parser_item>::const_iterator ki;

    for(ki = ps.kernel_items.begin(); ki != ps.kernel_items.end(); ki++) {
	parser_item i = *ki;

	// If this is the end of the item, nothing to do
	if(i.index == i.symbols.size()) {
	    continue;
	}

	// This is meant to test if the symbols is terminal
	if(islower(i.symbols[i.index][0])) {
	    continue;
	}

	string head = i.symbols[i.index];

	if(added[head]) {
	    continue;
	}

	list<list<string> >::const_iterator li;

	// For each right side of each production, add a non-kernel item
	for(li = productions[head].begin();
	    li != productions[head].end(); li++) {
	    parser_item pi(*li);

	    ps.nonkernel_items.push_back(pi);
	}

	added[head] = true;
    }

    unsigned x;

    do {
	// set to 0 and increment for each new item
	x = 0;

	// for each nonkernel item...

    } while(x > 0);

    return;
}

void parser::read(void)
{
    char line[80];

    while(cin.getline(line, sizeof(line))) {
	istringstream iss(line);
	list<string> symbols;
	string head;

	iss >> head;

	string sym;

	while(iss >> sym) {
	    symbols.push_back(sym);
	}

	productions[head].push_back(symbols);
    }

    return;
}

void parser::dump(void)
{
    map<string, list<list<string> > >::const_iterator mi;

    // For each production...
    for(mi = productions.begin(); mi != productions.end(); mi++) {
	string h = mi->first;
	list<list<string> > l = mi->second;

	list<list<string> >::const_iterator li;

	// This handles multiple productions with the same head
	for(li = l.begin(); li != l.end(); li++) {
	    list<string> l2 = *li;

	    cout << h << ": ";

	    list<string>::const_iterator li2;

	    // Iterates through the symbols
	    for(li2 = l2.begin(); li2 != l2.end(); li2++) {
		string s = *li2;

		cout << "[" << s << "]";
	    }

	    cout << endl;
	}
    }

    return;
}

int main(int argc, char * argv[])
{
    parser p;

    p.read();
    p.dump();

    p.run();

    return 0;
}
