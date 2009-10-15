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
	return;
    }

    parser_item(const string h, const vector<string> & v,
		unsigned start, unsigned size) : head(h), index(0) {
	symbols.resize(size);

	for(unsigned i = 0; i < size; i++) {
	    symbols[i] = v[i + start];
	}

	return;
    }

    void dump(void) {
	cout << "   " << head << " -> ";

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

	cout << "  kernel items:" << endl;

	for(li = kernel_items.begin(); li != kernel_items.end(); li++) {
	    parser_item pi = *li;

	    pi.dump();
	}

	cout << "  nonkernel items:" << endl;

	for(li = nonkernel_items.begin(); li != nonkernel_items.end(); li++) {
	    parser_item pi = *li;

	    pi.dump();
	}
    }
};

class tree_node {
    string symbol;
    string value;
    list<tree_node> nodes;

public:
    tree_node(const string & t) : symbol(t), value("") { }
    tree_node(const string & t, const string & v) : symbol(t), value(v) { }

    void insert(tree_node n) {
	nodes.push_front(n);
	return;
    }

    // Assemble a string of everything below this node...
    void dump_below(void) {
	if(symbol_is_terminal(symbol)) {
	    cout << value << " ";
	}

	list<tree_node>::const_iterator ti;

	for(ti = nodes.begin(); ti != nodes.end(); ti++) {
	    tree_node tn = *ti;

	    tn.dump_below();
	}

	return;
    }

    void dump(unsigned level = 0) {
	for(unsigned i = 0; i < level; i++) { cout << ' '; }

	dump_below();

	if(symbol_is_terminal(symbol)) {
	    cout << "[" << symbol << ", " << value << "]" << endl;
	} else {
	    cout << "[" << symbol << "]" << endl;
	}

	list<tree_node>::const_iterator ti;

	for(ti = nodes.begin(); ti != nodes.end(); ti++) {
	    tree_node tn = *ti;

	    tn.dump(level + 1);
	}

	return;
    }
};

class parser {
    map<string, list<vector<string> > > productions;

    list<parser_state> state_stack;
    list<string> symbol_stack;

    list<tree_node> node_stack;

    // These should be cleared whenever the grammar changes
    map<string, set<string> > first_cache;
    map<string, set<string> > follows_cache;

    string empty;

    string token;
    string token_value;

    int verbose;

public:
    parser(int v = 0) : empty(":empty:"), verbose(v) { }

    void run(void);

    void dump(const char * msg = NULL);

    void dump_tree(void) {
	if(!node_stack.empty()) {
	    tree_node tn = node_stack.back();
	    tn.dump();
	}
    }

    void load(const char * filename);

    void check(const string & t, const list<parser_item> & l,
	       int & cs, int & cr, int & ca);

    void build_items(const string & t, bool terminal,
		     const list<parser_item> & l, list<parser_item> & n);

    void shift(const parser_state & ps, const string & t);

    void reduce(parser_state & ps);

    void closure(parser_state & ps, map<string, bool> & added, bool k);
    void closure(parser_state & ps);

    bool first(const string & h, set<string> & rs, bool e = true);
    bool first(const string & h, map<string, bool> & v, set<string> & rs, bool e = true);
    bool first(const vector<string> & b, unsigned st, set<string> & rs, bool e = true);

    void follows(const string & fs, set<string> & rs);
    void follows(const string & fs, map<string, bool> & v, set<string> & rs);

    bool empty_check(const string & s);

    void check_shift(const string & t,
		     const list<parser_item> &l1, list<parser_item> & l2);

    void next_token(void);
};
