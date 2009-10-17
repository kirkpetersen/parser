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

class symbol {
public:
    std::string type;
    std::string value;

    symbol(void) : type(""), value("") { }
    symbol(const char * t) : type(t), value(t) { }
    symbol(const std::string & t) : type(t), value(t) { }
    symbol(const std::string & t, const std::string & v) : type(t), value(v) { }

    bool empty(void) const {
	return type.empty();
    }

    bool operator==(const symbol & s) const {
	bool result = type == s.type;
	return result;
    }

    bool operator!=(const symbol & s) const {
	bool result = type != s.type;
	return result;
    }

    bool operator<(const symbol & s) const {
	bool result = type < s.type;
	return result;
    }
};

std::ostream & operator<<(std::ostream & out, const symbol & s);

struct parser_item {
    symbol head;
    std::vector<symbol> symbols;
    unsigned index;
    symbol terminal;
};

bool operator<(const parser_item & p1, const parser_item & p2);
bool operator==(const parser_item & p1, const parser_item & p2);

struct parser_state {
    std::list<parser_item> kernel_items;
    std::list<parser_item> nonkernel_items;
};

class tree_node {
    symbol head;
    std::list<tree_node> nodes;
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
	    std::cout << head.value << " ";
	}

	std::list<tree_node>::const_iterator ti;

	for(ti = nodes.begin(); ti != nodes.end(); ++ti) {
	    tree_node tn = *ti;

	    tn.dump_below();
	}

	return;
    }

    void dump(unsigned level = 0) const {
	for(unsigned i = 0; i < level; i++) { std::cout << ' '; }

	dump_below();

	if(terminal) {
	    std::cout << " <- " << head << '\n';
	} else {
	    std::cout << " <- " << head.type << '\n';
	}

	std::list<tree_node>::const_iterator ti;

	for(ti = nodes.begin(); ti != nodes.end(); ++ti) {
	    tree_node tn = *ti;

	    tn.dump(level + 1);
	}

	return;
    }
};

class parser {
    symbol start;

    std::map<symbol, std::list<std::vector<symbol> > > productions;

    std::list<parser_state> state_stack;
    std::list<symbol> symbol_stack;

    std::list<tree_node> node_stack;

    std::set<symbol> tokens;
    std::set<symbol> literals;

    symbol token;

    int verbose;

public:
    parser(int v = 0) : start("START"), verbose(v) { }

    void run(void);

    void dump_set(const char * msg, const std::set<symbol> & rs);

    void dump(const char * msg = NULL);
    void dump_state(const parser_state & ps, unsigned spaces = 0);
    void dump_item(const parser_item & pi, unsigned spaces = 0);

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

    void check(const std::list<parser_item> & l,
	       int & cs, int & cr, int & ca);

    parser_item make_item(const symbol & h, const std::vector<symbol> & b,
			  const symbol & t);
    parser_item make_item(const symbol & h, const std::vector<symbol> & b,
			  const symbol & t, unsigned st);

    void build_items(const symbol & t,
		     const std::list<parser_item> & l,
		     std::list<parser_item> & n);

    void shift(const parser_state & ps, const symbol & t);

    void reduce(parser_state & ps);

    unsigned closure(parser_state & ps,
		     std::set<parser_item> & v,
		     std::set<parser_item> & a,
		     const std::list<parser_item> & items);
    void closure(parser_state & ps);

    bool first(const symbol & h, std::set<symbol> & rs);
    bool first(const symbol & h, std::map<symbol, bool> & v,
	       std::set<symbol> & rs);
    bool first(const std::vector<symbol> & b, unsigned st,
	       std::set<symbol> & rs);

    void follows(const symbol & fs, std::set<symbol> & rs);
    void follows(const symbol & fs, std::map<symbol, bool> & v,
		 std::set<symbol> & rs);

    void check_shift(const std::string & t,
		     const std::list<parser_item> &l1,
		     std::list<parser_item> & l2);

    void next_token(void);
};
