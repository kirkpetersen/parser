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
#include <set>

using namespace std;

bool symbol_is_terminal(const string & s)
{
    if(isupper(s[0])) {
	return false;
    } else {
	return true;
    }
}

class parser_item {
public:
    string head;
    vector<string> symbols;
    unsigned index;

    parser_item(const string h, const vector<string> & v)
	: head(h), symbols(v), index(0) {
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
    map<string, list<vector<string> > > productions;

    list<parser_state> state_stack;
    list<string> symbol_stack;

public:

    void run(void);
    void closure(parser_state & ps);
    void dump(void);
    void load(const char * filename);

    void first(const string & h, set<string> & rs);
    void first(const string & h, map<string, bool> & v, set<string> & rs);

    void follows(const string & fs, set<string> & rs);
    void follows(const string & fs, map<string, bool> & v, set<string> & rs);

    void check_shift(const string & t,
		     const list<parser_item> &l1, list<parser_item> & l2);

    const string token(void);
};

void parser::run(void)
{
    // states are kernel + non-kernel items

    // start symbol
    string START = "START";

    list<vector<string> > start = productions[START];

    if(start.empty()) {
	cerr << "no start state!" << endl;
	return;
    }

    parser_state ps;

    list<vector<string> >::const_iterator li;

    for(li = start.begin(); li != start.end(); li++) {
	parser_item pi(START, *li);

	ps.kernel_items.push_back(pi);
    }

    closure(ps);

    state_stack.push_back(ps);

    unsigned loop = 0;

#if 1
    cout << "[dumping before executing parser]" << endl;
    dump();
#endif

#if 1
    cout << "[starting the parse]" << endl;
#endif

    // Get the first token
    string t = token();

    cout << "new token: " << t << endl;

    for(;;) {
	cout << "LOOP: " << loop++ << endl;

	// First check for reductions
	list<parser_item>::const_iterator ki;

	int reduced = 0;

	for(ki = ps.kernel_items.begin(); ki != ps.kernel_items.end(); ki++) {
	    parser_item pi = *ki;

	    if(pi.index != pi.symbols.size()) {
		continue;
	    }

	    reduced = 1;

	    string head = pi.head;
	    unsigned n = pi.symbols.size();

	    cout << "reduce by " << head << " -> ";

	    for(unsigned i = 0; i < n; i++) {
		// Pop symbols off the stack and print
		cout << "[" << symbol_stack.back() << "]";

		symbol_stack.pop_back();

		// Pop this state off the stack
		ps = state_stack.back();
		state_stack.pop_back();
	    }

	    cout << endl;

	    cout << "new symbol on stack: " << head << endl;

	    symbol_stack.push_back(head);

	    break;
	}

	if(reduced) {
	    continue;
	}

	parser_state ns;

	cout << "shifting" << endl;

	// check shift on kernel and nonkernel items

	check_shift(t, ps.kernel_items, ns.kernel_items);
	check_shift(t, ps.nonkernel_items, ns.kernel_items);

	if(ns.kernel_items.empty()) {
	    cout << "no match for token " << t << endl;
	    break;
	}

	// TODO call closure(ns) to properly fill in state?
	closure(ns);

	symbol_stack.push_back(t);
	state_stack.push_back(ns);

	ps = ns;

	t = token();

	cout << "new token: " << t << endl;
    }

    return;
}

void parser::check_shift(const string & t,
			 const list<parser_item> & l1, list<parser_item> & l2)
{
    list<parser_item>::const_iterator li;

    for(li = l1.begin(); li != l1.end(); li++) {
	parser_item i = *li;
	string s = i.symbols[i.index];

	if(!symbol_is_terminal(s)) {
	    continue;
	}

	if(t == s) {
	    parser_item ni = i;

	    ni.index++;

	    l2.push_back(ni);
	}
    }

    return;
}

void parser::first(const string & h, map<string, bool> & v, set<string> & rs)
{
    // If it is a terminal, it goes on the list
    if(symbol_is_terminal(h)) {
	rs.insert(h);
	return;
    }

    if(v[h]) {
	return;
    }

    v[h] = true;

    list<vector<string> >::const_iterator li;

    for(li = productions[h].begin(); li != productions[h].end(); li++) {
	string s = (*li)[0];

	if(symbol_is_terminal(s)) {
	    rs.insert(s);
	} else {
	    first(s, v, rs);
	}
    }

    return;
}

void parser::first(const string & h, set<string> & rs)
{
    map<string, bool> visited;

    // Call the recursive version with the visited map
    first(h, visited, rs);

    return;
}

void parser::follows(const string & fs, map<string, bool> & v, set<string> & rs)
{
    // If we've already done FOLLOWS on this symbol, return
    if(v[fs]) {
	return;
    }

    v[fs] = true;

    map<string, list<vector<string> > >::const_iterator mi;

    // Iterate over all productions
    for(mi = productions.begin(); mi != productions.end(); mi++) {
	const string head = mi->first;
	list<vector<string> > body = mi->second;

	list<vector<string> >::const_iterator li;

	for(li = body.begin(); li != body.end(); li++) {
	    vector<string> p = *li;
	    unsigned size = p.size();

	    // All but the last symbol
	    for(unsigned i = 0; i < size - 1; i++) {
		if(p[i] == fs) {
		    // FIRST of the next symbol
		    first(p[i + 1], rs);
		}
	    }

	    // Handle the last symbol
	    if(p[size - 1] == fs) {
		follows(head, v, rs);
	    }
	}
    }

    return;
}

void parser::follows(const string & fs, set<string> & rs)
{
    map<string, bool> visited;

    follows(fs, visited, rs);

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
	if(symbol_is_terminal(i.symbols[i.index])) {
	    continue;
	}

	string head = i.symbols[i.index];

	if(added[head]) {
	    continue;
	}

	list<vector<string> >::const_iterator li;

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

	    if(symbol_is_terminal(i.symbols[i.index])) {
		continue;
	    }

	    string head = i.symbols[i.index];

	    if(added[head]) {
		continue;
	    }

	    list<vector<string> >::const_iterator li;

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
	vector<string> symbols;
	string head;

	iss >> head;

	if(head.empty()) {
	    continue;
	}

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
    map<string, list<vector<string> > >::const_iterator mi;

    cout << "* productions:" << endl;

    // For each production...
    for(mi = productions.begin(); mi != productions.end(); mi++) {
	string h = mi->first;
	list<vector<string> > l = mi->second;

	list<vector<string> >::const_iterator li;

	// This handles multiple productions with the same head
	for(li = l.begin(); li != l.end(); li++) {
	    vector<string> v = *li;

	    cout << h << ": ";

	    list<string>::const_iterator li2;

	    // Iterates through the symbols
	    for(unsigned i = 0; i < v.size(); i++) {
		string s = v[i];

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

const string parser::token(void)
{
    // Simply return the next token

    string s;

    cin >> s;

    return s;
}

int test(parser & p)
{
    set<string>::const_iterator si;

    // FIRST(START)
    {
	set<string> rs;

	cout << "FIRST(START): " << endl;
	p.first(string("START"), rs);

	for(si = rs.begin(); si != rs.end(); si++) {
	    cout << "[" << *si << "]";
	}

	cout << endl;

    }

    // FIRST(E)
    {
	set<string> rs;

	cout << "FIRST(E): " << endl;
	p.first(string("E"), rs);

	for(si = rs.begin(); si != rs.end(); si++) {
	    cout << "[" << *si << "]";
	}

	cout << endl;
    }


    // FOLLOWS(E)
    {
	set<string> rs;

	cout << "FOLLOWS(E): " << endl;
	p.follows(string("E"), rs);

	for(si = rs.begin(); si != rs.end(); si++) {
	    cout << "[" << *si << "]";
	}

	cout << endl;
    }

    // FIRST(+)
    {
	set<string> rs;

	cout << "FIRST(+): " << endl;
	p.first(string("+"), rs);

	for(si = rs.begin(); si != rs.end(); si++) {
	    cout << "[" << *si << "]";
	}

	cout << endl;
    }

    return 0;
}

int main(int argc, char * argv[])
{
    parser p;

    if(!argv[1]) {
	return 1;
    }

    p.load(argv[1]);

    cout << "[first run tests]" << endl;

    if(test(p) != 0) {
	cerr << "test: FAIL" << endl;
    }

    cout << "[dumping after read]" << endl;

    p.dump();

    p.run();

    cout << "[dumping after run]" << endl;

    p.dump();

    return 0;
}
