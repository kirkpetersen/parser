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

class symbol {
public:
    string type;
    string value;

    symbol(void) : type(""), value("") { }
    symbol(const char * t) : type(t), value(t) { }
    symbol(const string & t) : type(t), value(t) { }
    symbol(const string & t, const string & v) : type(t), value(v) { }

    bool empty(void) const {
	return type.empty();
    }

    bool operator==(const symbol & s) const {
	bool result = type == s.type;
	return result;
    }

    bool operator<(const symbol & s) const {
	bool result = type < s.type;
	return result;
    }
};

ostream & operator<<(ostream & out, const symbol & s);

struct parser_item {
    symbol head;
    vector<symbol> symbols;
    unsigned index;
};

struct parser_state {
    list<parser_item> kernel_items;
    list<parser_item> nonkernel_items;
};

class tree_node {
    symbol head;
    list<tree_node> nodes;
    bool terminal;

public:
    tree_node(const symbol & t, bool tl) : head(t), terminal(tl) { }

    void insert(tree_node n) {
	nodes.push_front(n);
	return;
    }

    // Assemble a string of everything below this node...
    void dump_below(void) const {
	if(terminal) {
	    cout << head.value << " ";
	}

	list<tree_node>::const_iterator ti;

	for(ti = nodes.begin(); ti != nodes.end(); ti++) {
	    tree_node tn = *ti;

	    tn.dump_below();
	}

	return;
    }

    void dump(unsigned level = 0) const {
	for(unsigned i = 0; i < level; i++) { cout << ' '; }

	dump_below();

	if(terminal) {
	    cout << " <- " << head << endl;
	} else {
	    cout << " <- " << head.type << endl;
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
    symbol start;

    map<symbol, list<vector<symbol> > > productions;

    list<parser_state> state_stack;
    list<symbol> symbol_stack;

    list<tree_node> node_stack;

    symbol empty;

    set<symbol> tokens;
    set<symbol> literals;

    symbol token;

    int verbose;

public:
    parser(int v = 0) : empty(":empty:"), verbose(v) { }

    void run(void);

    void dump_set(const char * msg, const set<symbol> & rs);

    void dump(const char * msg = NULL);
    void dump_state(const parser_state & ps);
    void dump_item(const parser_item & pi);

    void dump_tree(void) {
	if(!node_stack.empty()) {
	    tree_node tn = node_stack.back();
	    tn.dump();
	}
    }

    void load(const char * filename);

    bool terminal(const symbol & s) const {
	if(productions.count(s) > 0) {
	    return false;
	} else {
	    return true;
	}
    }

    void check(const list<parser_item> & l,
	       int & cs, int & cr, int & ca);

    parser_item make_item(const symbol & h, const vector<symbol> & b);
    parser_item make_item(const symbol & h, const vector<symbol> & b,
			  unsigned st);

    void build_items(const symbol & t, bool tl,
		     const list<parser_item> & l, list<parser_item> & n);

    void shift(const parser_state & ps, const symbol & t);

    void reduce(parser_state & ps);

    void closure(parser_state & ps, map<symbol, bool> & added, bool k);
    void closure(parser_state & ps);

    bool first(const symbol & h, set<symbol> & rs);
    bool first(const symbol & h, map<symbol, bool> & v, set<symbol> & rs);
    bool first(const vector<symbol> & b, unsigned st, set<symbol> & rs);

    void follows(const symbol & fs, set<symbol> & rs);
    void follows(const symbol & fs, map<symbol, bool> & v, set<symbol> & rs);

    bool empty_check(const symbol & s);

    void check_shift(const string & t,
		     const list<parser_item> &l1, list<parser_item> & l2);

    void next_token(void);
};
