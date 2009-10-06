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
public:
    map<string, list<list<string> > > productions;
};

void run(parser & p)
{
    list<int> state_stack;
    list<int> symbol_stack;

    // states are kernel + non-kernel items

    // start symbol
    list<list<string> > start = p.productions[string("start")];

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

void read(parser & p)
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

	p.productions[head].push_back(symbols);
    }

    return;
}

void dump(parser & p)
{
    map<string, list<list<string> > >::const_iterator mi;

    // For each production...
    for(mi = p.productions.begin(); mi != p.productions.end(); mi++) {
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

    read(p);
    dump(p);

    run(p);

    return 0;
}
