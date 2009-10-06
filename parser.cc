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

class parser {
    map<string, list<list<string> > > productions;

    list<int> state_stack;
    list<int> symbol_stack;

public:

    void run(void);
    void dump(void);
    void read(void);
};

void parser::run(void)
{
    // states are kernel + non-kernel items

    // start symbol
    list<list<string> > start = productions[string("start")];

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
