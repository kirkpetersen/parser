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

struct parser_rule {
    std::string head;
    std::vector<std::string> symbols;
};

struct parser_item {
    const parser_rule * rule;

    unsigned index;
    std::string terminal;
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

class parser {
    parser_stats stats;

    std::string start;

    std::map<std::string, std::list<parser_rule *> > productions;

    std::deque<parser_state *> state_stack;
    std::deque<std::string> symbol_stack;

    std::deque<struct tree_node *> node_stack;

    std::set<std::string> tokens;
    std::set<std::string> literals;

    int verbose;

public:
    parser(int v = 0) : start("START"), verbose(v) {
	memset(&stats, 0, sizeof(stats));
    }

    ~parser(void);

    void build_rule(const std::string & head, ...);

    void bootstrap(void);

    void init_state(void);

    void expect(const std::set<parser_item *, parser_item_compare> & items,
		std::set<std::string> & ss, bool nt = false);
    void expect(std::set<std::string> & ss, bool nt = false);

    bool step(std::string & t, std::string & tv);
    void run(std::istream & tin);

    struct tree_node * tree(void) {
	if(node_stack.empty()) {
	    return NULL;
	} else {
	    return node_stack.back();
	}
    }

    // FIXME tweak this...
    struct tree_node * find_node(struct tree_node * tn, const std::string & s);

    void load(const char * filename);

    void load_tokenizer(struct tree_node * tn);

    void load_grammar(struct tree_node * tn);
    void load_production(struct tree_node * tn);
    
    bool terminal(const std::string & s) const {
	if(productions.count(s) > 0) {
	    return false;
	} else {
	    return true;
	}
    }

    void check(const std::string & t,
	       const std::set<parser_item *, parser_item_compare> & l,
	       int & cs, int & cr, int & ca);

    parser_item * make_item(const parser_rule * r, const std::string & t);

    void build_items(const std::string & t,
		     const std::set<parser_item *, parser_item_compare> & l,
		     std::set<parser_item *, parser_item_compare> & n);

    void shift(parser_state * ps,
	       const std::string & t, const std::string & tv);
    bool reduce(parser_state * ps,
		const std::string & t, const std::string & tv,
		std::set<parser_item *, parser_item_compare> & items,
		bool noshift = false);

    // Helper function for shift and reduce
    parser_state * sr(parser_state * ps, const std::string & t);

    void closure(parser_state * ps);

    bool first(const std::string & h, std::set<std::string> & rs);
    bool first(const std::string & h, std::set<std::string> & v,
	       std::set<std::string> & rs);
    void first(const parser_item * pi, unsigned idx, std::set<std::string> & rs);

    void check_shift(const std::string & t,
		     const std::set<parser_item *> & l1,
		     std::set<parser_item *> & l2);

    void next_token(std::istream & tin, std::string & t, std::string & tv);

    // All of these are in dump.cc
    void dump_stats(void);

    void dump_set(const char * msg, const std::set<std::string> & rs);

    void dump(const char * msg, std::string & t, std::string & tv);
    void dump_grammar(void);
    void dump_state(const parser_state * ps, unsigned spaces = 0);
    void dump_item(const parser_item * pi, unsigned spaces = 0);
};
