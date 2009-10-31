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

typedef std::set<std::string> string_set;

struct parser_rule {
    std::string head;
    std::vector<std::string> symbols;
};

typedef std::list<parser_rule *> parser_rule_list;
typedef parser_rule_list::const_iterator parser_rule_iter;

struct parser_item {
    const parser_rule * rule;
    unsigned index;
    std::string lookahead;
};

struct parser_item_compare {
    bool operator()(const parser_item * p1, const parser_item * p2) const;
};

typedef std::set<parser_item *, parser_item_compare> parser_item_set;
typedef parser_item_set::const_iterator parser_item_iter;

struct parser_state {
    parser_item_set kernel_items;
    parser_item_set nonkernel_items;
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

struct grammar_node {
    grammar_node * next;
    char * key;
    parser_rule_list list;
};

class grammar {
    static const unsigned table_size = 311;
    grammar_node * table[table_size];

public:
    grammar(void) {
	for(unsigned i = 0; i < table_size; i++) {
	    table[i] = NULL;
	}
    }

    // TODO need to properly cleanup everything

    unsigned hash(const char * k) const {
	unsigned h = 0;

	for(unsigned i = 0; k[i]; i++) {
	    h += k[i];
	}

	return h;
    }

    unsigned count(const char * k) const;
    parser_rule_list & operator[](const char * k);

    void dump(void) const;
};

class parser {
    parser_stats stats;

    std::string start;

    grammar productions;

    std::deque<parser_state *> state_stack;
    std::deque<std::string> symbol_stack;

    std::deque<struct tree_node *> node_stack;

    string_set tokens;
    string_set literals;

    int verbose;

    bool error;

public:
    parser(int v = 0) : start("START"), verbose(v), error(false) {
	memset(&stats, 0, sizeof(stats));
    }

    ~parser(void);

    void verbosity_increment(void);

    bool get_error(void) const { return error; }

    void build_rule(const std::string & head, ...);

    void bootstrap(void);

    void init_state(void);

    void expect(const parser_item_set & items, string_set & ss,
		bool nt = false);
    void expect(string_set & ss, bool nt = false);

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
	if(productions.count(s.c_str()) > 0) {
	    return false;
	} else {
	    return true;
	}
    }

    void check(const std::string & t, const parser_item_set & l,
	       int & cs, int & cr, int & ca);

    parser_item * make_item(const parser_rule * r, const std::string & t);

    void build_items(const std::string & t, const parser_item_set & l,
		     parser_item_set & n);

    void shift(parser_state * ps,
	       const std::string & t, const std::string & tv);
    bool reduce(parser_state * ps,
		const std::string & t, const std::string & tv,
		parser_item_set & items, bool noshift = false);

    // Helper function for shift and reduce
    parser_state * sr(parser_state * ps, const std::string & t);

    unsigned closure(parser_state * ps);

    bool first(const std::string & h, string_set & rs);
    bool first(const std::string & h, string_set & v, string_set & rs);
    void first(const parser_item * pi, unsigned idx, string_set & rs);

    void check_shift(const std::string & t,
		     const parser_item_set & l1, parser_item_set & l2);

    void next_token(std::istream & tin, std::string & t, std::string & tv);

    // All of these are in dump.cc
    void dump_stats(void);

    void dump_set(const char * msg, const string_set & rs);

    void dump(const char * msg, std::string & t, std::string & tv);
    void dump(void);
    void dump_grammar(void);
    void dump_state(const parser_state * ps, unsigned spaces = 0);
    void dump_item(const parser_item * pi, unsigned spaces = 0);
};
