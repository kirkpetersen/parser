/*
 * parser.hh
 */

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <deque>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <map>
#include <queue>
#include <set>

extern "C" {
#include "parser_c.h"
}

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

struct parser_item_compare {
    bool operator()(const parser_item * p1, const parser_item * p2) const;
};

struct parser_state {
    std::set<parser_item *, parser_item_compare> kernel_items;
    std::set<parser_item *, parser_item_compare> nonkernel_items;
};

struct parser_stats {
    unsigned build_item;
    unsigned build_item_skips;
    unsigned loops;
    unsigned shifts;
    unsigned reduces;
    unsigned accepts;
    unsigned build_items_calls;
    unsigned closure_calls;
    unsigned closure_loops;
    unsigned closure_item_duplicates;
    unsigned closure_item_non_duplicates;
    unsigned closure_skips;
};

struct parser_rule {
    symbol head;
    std::vector<symbol> symbols;
};

class parser {
    parser_stats stats;

    std::string start;

    std::map<std::string, std::list<parser_rule *> > productions;

    std::deque<parser_state *> state_stack;
    std::deque<symbol> symbol_stack;

    std::deque<struct tree_node *> node_stack;

    std::set<symbol> tokens;
    std::set<symbol> literals;

    symbol token;

    int verbose;

public:
    parser(int v = 0) : start("START"), verbose(v) {
	memset(&stats, 0, sizeof(stats));
    }

    ~parser(void);

    void build_rule(const std::string & head, ...);

    void bootstrap(void);

    void run(std::istream & tin);

    struct tree_node * tree(void) {
	return node_stack.back();
    }

    // FIXME tweak this...
    struct tree_node * find_node(struct tree_node * tn, const std::string & s);

    void load(const char * filename);

    void load_tokenizer(struct tree_node * tn);

    void load_grammar(struct tree_node * tn);
    void load_production(struct tree_node * tn);
    
    bool terminal(const symbol & s) const {
	if(productions.count(s.type) > 0) {
	    return false;
	} else {
	    return true;
	}
    }

    void check(const symbol & t,
	       const std::set<parser_item *, parser_item_compare> & l,
	       int & cs, int & cr, int & ca);

    parser_item * make_item(const symbol & h, const std::vector<symbol> & b,
			    const symbol & t);

    void build_items(const symbol & t,
		     const std::set<parser_item *, parser_item_compare> & l,
		     std::set<parser_item *, parser_item_compare> & n);

    void shift(parser_state * ps, const symbol & t);
    bool reduce(parser_state * ps, const symbol & t, bool k = true);

    // Helper function for shift and reduce
    parser_state * sr(parser_state * ps, const symbol & t);

    void closure(parser_state * ps);

    bool first(const symbol & h, std::set<symbol> & rs);
    bool first(const symbol & h, std::set<symbol> & v,
	       std::set<symbol> & rs);
    void first(const parser_item * pi, unsigned idx, std::set<symbol> & rs);

    void check_shift(const std::string & t,
		     const std::set<parser_item *> & l1,
		     std::set<parser_item *> & l2);

    symbol next_token(std::istream & tin);

    // All of these are in dump.cc
    void dump_stats(void);

    void dump_set(const char * msg, const std::set<symbol> & rs);

    void dump(const char * msg = NULL);
    void dump_grammar(void);
    void dump_state(const parser_state * ps, unsigned spaces = 0);
    void dump_item(const parser_item * pi, unsigned spaces = 0);
};
