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

struct tree_node {
    symbol head;
    std::list<tree_node> nodes;
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

    void bootstrap(void);

    void run(std::istream & tin);

    void dump_set(const char * msg, const std::set<symbol> & rs);

    void dump(const char * msg = NULL);
    void dump_state(const parser_state & ps, unsigned spaces = 0);
    void dump_item(const parser_item & pi, unsigned spaces = 0);

    void dump_tree(void) const;
    void dump_tree_below(const tree_node & tn) const;
    void dump_tree(const tree_node & tn, unsigned level = 0) const;

    const tree_node & tree(void) const {
	return node_stack.back();
    }

    bool find_node(const tree_node & tn, const symbol & s, tree_node & t) const;

    void load(const char * filename);

    void load_tokenizer(const tree_node & tn);
    void load_token_line_list(const tree_node & tn);
    void load_token_line(const tree_node & tn);
    void load_token_list(const tree_node & tn);

    void load_grammar(const tree_node & tn);
    void load_production_list(const tree_node & tn);
    void load_production(const tree_node & tn);
    void load_body_list(const tree_node & tn,
			std::list<std::vector<symbol> > & p);
    void load_body(const tree_node & tn, std::vector<symbol> & b);
    void load_symbol_list(const tree_node & tn, std::vector<symbol> & b);
    void load_symbol(const tree_node & tn, std::vector<symbol> & b);
    
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

    void next_token(std::istream & tin);
};
