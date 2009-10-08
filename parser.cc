/*
 * parser.cc
 */

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>
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
    string head;
    vector<string> symbols;
    unsigned index;

    parser_item(const string h, const list<string> & l)
	: head(h), symbols(l.size()), index(0) {
	list<string>::const_iterator li;
	unsigned i = 0;

	for(li = l.begin(); li != l.end(); li++) {
	    symbols[i++] = *li;
	}

	return;
    }

    void dump(void) {
	cout << head << ": ";

	for(unsigned i = 0; i < symbols.size(); i++) {
	    if(i == index) {
		cout << "[" << symbols[i] << "]";
	    } else {
		cout << "(" << symbols[i] << ")";
	    }
	}

	cout << endl;
    }
};

class parser_state {
public:
    list<parser_item> kernel_items;
    list<parser_item> nonkernel_items;

    void dump(void) {
	list<parser_item>::const_iterator li;

	cout << "kernel items:" << endl;

	for(li = kernel_items.begin(); li != kernel_items.end(); li++) {
	    parser_item pi = *li;

	    pi.dump();
	}

	cout << "nonkernel items:" << endl;

	for(li = nonkernel_items.begin(); li != nonkernel_items.end(); li++) {
	    parser_item pi = *li;

	    pi.dump();
	}
    }
};

class parser {
    map<string, list<list<string> > > productions;

    list<parser_state> state_stack;
    list<string> symbol_stack;

public:

    void run(void);
    void closure(parser_state & ps);
    void dump(void);
    void load(const char * filename);

    void check_shift(const list<parser_item> &l1, list<parser_item> & l2);

    string token(void);
};

void parser::run(void)
{
    // states are kernel + non-kernel items

    // start symbol
    string START = "S";

    list<list<string> > start = productions[START];

    if(start.empty()) {
	cerr << "no start state!" << endl;
	return;
    }

    parser_state ps;

    list<list<string> >::const_iterator li;

    for(li = start.begin(); li != start.end(); li++) {
	parser_item pi(START, *li);

	ps.kernel_items.push_back(pi);
    }

    closure(ps);

    state_stack.push_back(ps);

    unsigned loop = 0;

    int reduced = 0;

    string t = token();

    cout << "new token: " << t << endl;

    // TODO need to work on tokenizer

    for(unsigned foo = 0; foo < 0; foo++) {
	cout << "LOOP: " << loop++;

	// First check for reductions
	list<parser_item>::const_iterator ki;

	for(ki = ps.kernel_items.begin(); ki != ps.kernel_items.end(); ki++) {
	    parser_item pi = *ki;

	    if(pi.index != pi.symbols.size()) {
		continue;
	    }

	    reduced = 1;

	    string head = pi.head;
	    unsigned ns = pi.symbols.size();

	    cout << "reduce by %s -> " << head;

	    for(unsigned i = 0; i < ns; i++) {
		// Pop symbols off the stack and print
		cout << symbol_stack.back();

		symbol_stack.pop_back();

		if(i < ns - 1) {
		    cout << " ";
		}

		// Pop this state off the stack
		ps = state_stack.back();
		state_stack.pop_back();
	    }

	    cout << endl;

	    cout << "new symbol on stack: %s" << head;

	    symbol_stack.push_back(head);

	    break;
	}

	if(reduced) {
	    continue;
	}

	parser_state ns;

	cout << "shifting" << endl;

	// check shift on kernel and nonkernel items

	check_shift(ps.kernel_items, ns.kernel_items);
	check_shift(ps.nonkernel_items, ns.kernel_items);

	if(ns.kernel_items.empty()) {
	    cout << "no match for token " << t << endl;
	    break;
	}

	t = token();

	cout << "new token: " << t << endl;
    }

    return;
}

void parser::check_shift(const list<parser_item> & l1, list<parser_item> & l2)
{
    list<parser_item>::const_iterator li;

    for(li = l1.begin(); li != l1.end(); li++) {
	parser_item i = *li;
	string s = i.symbols[i.index];

	if(islower(s[0])) {
	    continue;
	}

	if("x" == s) {
	    parser_item ni = i;

	    ni.index++;

	    l2.push_back(ni);
	}
    }

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
	    parser_item pi(head, *li);

	    ps.nonkernel_items.push_back(pi);
	}

	added[head] = true;
    }

    unsigned x;

    do {
	// set to 0 and increment for each new item
	x = 0;

	for(ki = ps.nonkernel_items.begin();
	    ki != ps.nonkernel_items.end(); ki++) {
	    parser_item i = *ki;

	    if(islower(i.symbols[i.index][0])) {
		continue;
	    }

	    string head = i.symbols[i.index];

	    if(added[head]) {
		continue;
	    }

	    list<list<string> >::const_iterator li;

	    for(li = productions[head].begin();
		li != productions[head].end(); li++) {
		parser_item pi(head, *li);

		ps.nonkernel_items.push_back(pi);
	    }

	    added[head] = true;

	    x++;
	}

    } while(x > 0);

    return;
}

void parser::load(const char * filename)
{
    char line[80];

    fstream fs(filename, fstream::in);

    while(fs.getline(line, sizeof(line))) {
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

    fs.close();

    return;
}

void parser::dump(void)
{
    map<string, list<list<string> > >::const_iterator mi;

    cout << "* productions:" << endl;

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

    cout << "* parser state:" << endl;

    cout << "symbol stack:" << endl;

    list<string>::const_iterator sy;

    for(sy = symbol_stack.begin(); sy != symbol_stack.end(); sy++) {
	cout << *sy << endl;
    }

    cout << "state stack:" << endl;

    list<parser_state>::const_iterator st;

    for(st = state_stack.begin(); st != state_stack.end(); st++) {
	parser_state ps = *st;

	ps.dump();
    }

    return;
}

string parser::token(void)
{
    // Simply return the next token

    string s;

    cin >> s;

    return s;
}

int main(int argc, char * argv[])
{
    parser p;

    if(!argv[1]) {
	return 1;
    }

    p.load(argv[1]);

    cout << "[dumping after read]" << endl;

    p.dump();

    p.run();

    cout << "[dumping after run]" << endl;

    p.dump();

    return 0;
}
