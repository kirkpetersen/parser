/*
 * parser.hh
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

bool symbol_is_terminal(const string & s);

class parser_item {
public:
    string head;
    vector<string> symbols;
    unsigned index;

    parser_item(const string h, const vector<string> & v)
	: head(h), symbols(v), index(0) {
    }

    void dump(void) {
	cout << "  " << head << " -> ";

	unsigned size = symbols.size();

	for(unsigned i = 0; i < size; i++) {
	    if(i == index) {
		cout << ". ";
	    }

	    cout << symbols[i];

	    if(i < size - 1) {
		cout << " ";
	    }
	}

	if(index == size) {
	    cout << " .";
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

	cout << " kernel items:" << endl;

	for(li = kernel_items.begin(); li != kernel_items.end(); li++) {
	    parser_item pi = *li;

	    pi.dump();
	}

	cout << " nonkernel items:" << endl;

	for(li = nonkernel_items.begin(); li != nonkernel_items.end(); li++) {
	    parser_item pi = *li;

	    pi.dump();
	}
    }
};

class tree_node {
    string symbol;
    list<tree_node *> nodes;

public:
    tree_node(const string & t) : symbol(t) { }

    void insert(tree_node * n) {
	nodes.push_front(n);
	return;
    }

    // Assemble a string of everything below this node...
    void dump2(string & s) {
	if(symbol_is_terminal(symbol)) {
	    s += symbol + " ";
	}

	list<tree_node *>::const_iterator ti;

	for(ti = nodes.begin(); ti != nodes.end(); ti++) {
	    tree_node * tn = *ti;

	    if(tn) {
		tn->dump2(s);
	    }
	}

	return;
    }

    void dump(unsigned spaces) {
	for(unsigned i = 0; i < spaces; i++) { cout << ' '; }

	string s;

	dump2(s);

	cout << symbol << " ( " << s << ")" << endl;

	list<tree_node *>::const_iterator ti;

	for(ti = nodes.begin(); ti != nodes.end(); ti++) {
	    tree_node * tn = *ti;

	    if(tn) {
		tn->dump(spaces + 1);
	    }
	}

	return;
    }
};

class parser {
    map<string, list<vector<string> > > productions;

    list<parser_state> state_stack;
    list<string> symbol_stack;

    list<tree_node *> node_stack;

public:

    void run(void);
    void closure(parser_state & ps);
    void dump(const char * msg = NULL);
    void load(const char * filename);

    void check(const string & t, const list<parser_item> & l,
	       int & cs, int & cr, int & ca);

    void build_items(const string & t, bool terminal,
		     const list<parser_item> & l, list<parser_item> & n);

    void shift(const parser_state & ps, const string & t);

    void reduce(parser_state & ps);

    void first(const string & h, set<string> & rs);
    void first(const string & h, map<string, bool> & v, set<string> & rs);

    void follows(const string & fs, set<string> & rs);
    void follows(const string & fs, map<string, bool> & v, set<string> & rs);

    void check_shift(const string & t,
		     const list<parser_item> &l1, list<parser_item> & l2);

    const string token(void);
};
