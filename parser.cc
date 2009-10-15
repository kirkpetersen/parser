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

#include "parser.hh"

using namespace std;

bool symbol_is_terminal(const string & s)
{
    if(s == ":empty:") {
	return false;
    } else if(s == ":num:") {
	return true;
    } else if(s == ":id:") {
	return true;
    } else if(isupper(s[0])) {
	return false;
    }

    return true;
}

void parser::reduce(parser_state & ps)
{
    list<parser_item>::const_iterator ki;

    for(ki = ps.kernel_items.begin(); ki != ps.kernel_items.end(); ki++) {
	const parser_item & pi = *ki;

	if(pi.index != pi.symbols.size()) {
	    continue;
	}

	string head = pi.head;
	unsigned n = pi.symbols.size();

	tree_node tn(head);

	// Pop all the appropriate symbols off the stack
	for(unsigned i = 0; i < n; i++) {
	    // Add to the tree
	    tn.insert(node_stack.back());
	    node_stack.pop_back();

	    symbol_stack.pop_back();

	    // Pop this state off the stack
	    state_stack.pop_back();

	    // Set the current state to the previous
	    ps = state_stack.back();
	}

	if(verbose > 0) {
	    cout << "reduce: " << head << " -> ... " << endl;
	}

	symbol_stack.push_back(head);

	// Push new tree node
	node_stack.push_back(tn);

	// Now that we've reduced, we need to look at the new
	// symbol on the stack and determine a new transition

	parser_state ns;

	// Fill in the new state
	build_items(head, false, ps.kernel_items, ns.kernel_items);
	build_items(head, false, ps.nonkernel_items, ns.kernel_items);

	// Is this correct?
	closure(ns);

	state_stack.push_back(ns);

	break;
    }

    return;
}

void parser::run(void)
{
    // start symbol
    string START = "START";

    list<vector<string> > start = productions[START];

    if(start.empty()) {
	cerr << "no start state!" << endl;
	return;
    }

    // Create the initial parser state
    parser_state ps;

    list<vector<string> >::const_iterator li;

    for(li = start.begin(); li != start.end(); li++) {
	parser_item pi(START, *li);

	ps.kernel_items.push_back(pi);
    }

    closure(ps);

    state_stack.push_back(ps);

    unsigned loop = 0;

    if(verbose > 1) {
	dump("dumping before executing parser");
    }

    if(verbose > 0) {
	cout << "[starting the parse]" << endl;
    }

    // Get the first token
    next_token();

    for(;;) {
	if(verbose > 0) {
	    cout << "LOOP: " << loop++
		 << ", token: " << token
		 << ", token_value: " << token_value
		 << endl;

	    cout.flush();
	}

	if(verbose > 1) {
	    dump("verbose dump (every loop)");
	}

	int cs = 0, cr = 0, ca = 0;

	check(token, ps.kernel_items, cs, cr, ca);
	check(token, ps.nonkernel_items, cs, cr, ca);

	if(verbose > 0) {
	    cout << "shifts: " << cs << ", "
		 << "reduces: " << cr << ", "
		 << "accepts: " << ca << endl;
	}

	// ACCEPT
	if(ca > 0) {
	    cout << "ACCEPT!" << endl;
	    break;
	}

	if(cs > 0 && cr > 0) {
	    dump("shift/reduce conflict");
	    break;
	}

	if(cr > 1) {
	    dump("reduce/reduce conflict");
	    break;
	}

	if(cs > 0) {
	    shift(ps, token);

	    // Set the current state to the one shift() created
	    ps = state_stack.back();

	    next_token();
	} else if(cr > 0) {
	    reduce(ps);

	    ps = state_stack.back();

	} else {
	    dump("no action!");
	    break;
	}
    }

    return;
}

void parser::check(const string & t, const list<parser_item> & l,
		   int & cs, int & cr, int & ca)
{
    list<parser_item>::const_iterator li;

    bool accept_check = (t == "$");

    // Check each item in the list
    for(li = l.begin(); li != l.end(); li++) {
	const parser_item & i = *li;

	// TODO also check for reduce whenever the remaining
	// symbols can evaluate to :empty:

	if(i.index < i.symbols.size()) {
	    // Not at the end of the item, check for shift

	    if(!symbol_is_terminal(i.symbols[i.index])) {
		continue;
	    }

	    if(t == i.symbols[i.index]) {
		cs++;
	    }
	    
	} else {
	    // At the end of this item, check for valid reduce or accept

	    set<string> rs;

	    follows(i.head, rs);

	    if(rs.count(t) > 0) {
		cr++;
	    }

	    // Is this the correct check for ACCEPT?
	    if(accept_check && i.head == "START" && rs.count(t) > 0) {
		ca++;
	    }
	}
    }

    return;
}

void parser::build_items(const string & t, bool terminal,
			 const list<parser_item> & l, list<parser_item> & n)
{
    list<parser_item>::const_iterator li;

    for(li = l.begin(); li != l.end(); li++) {
	const parser_item & i = *li;

	if(i.index >= i.symbols.size()) {
	    continue;
	}

	string s = i.symbols[i.index];

	if(terminal && !symbol_is_terminal(s)) {
	    continue;
	}

	if(t == s) {
	    parser_item ni = i;

	    ni.index++;

	    n.push_back(ni);
	}
    }

    return;
}

void parser::shift(const parser_state & ps, const string & t)
{
    parser_state ns;

    if(verbose > 0) {
	cout << "shifting '" << t << "'" << endl;
    }

    // Fill in the new state
    build_items(token, true, ps.kernel_items, ns.kernel_items);
    build_items(token, true, ps.nonkernel_items, ns.kernel_items);

    if(ns.kernel_items.empty()) {
	cout << "no match for token " << t << endl;
	return;
    }

    closure(ns);

    // Finish up shifting
    symbol_stack.push_back(token);
    state_stack.push_back(ns);

    // New node for this symbol
    node_stack.push_back(tree_node(token, token_value));

    return;
}

bool parser::first(const string & h, map<string, bool> & v, set<string> & rs, bool e)
{
    bool se = false;

    // If it is a terminal, it goes on the list and we return
    if(symbol_is_terminal(h)) {
	rs.insert(h);
	return se;
    }

    if(v[h]) {
	return se;
    }

    v[h] = true;

    list<vector<string> >::const_iterator li;

    for(li = productions[h].begin(); li != productions[h].end(); li++) {
	const vector<string> & b = *li;

	if(b[0] == empty) {
	    // Indicate that we saw :empty:
	    se = true;

	    // But only add it to the set if requested
	    if(e) {
		rs.insert(empty);
	    }

	    continue;
	}

	unsigned i;

	for(i = 0; i < b.size(); i++) {
	    const string & s = b[i];

	    // Now add the FIRST of the current symbol (minus :empty:)
	    if(first(s, v, rs, false)) {
		// If :empty: was seen, we continue with this body
		se = true;
		continue;
	    } else {
		// Otherwise break and continue with the next body
		break;
	    }
	}

	// If we make it to the end of the body, it means that
	// All of the symbols include a potential :empty:, so
	// include it in the set (if specified)
	if(i == b.size()) {
	    se = true;

	    if(e) {
		rs.insert(empty);
	    }
	}
    }

    return se;
}

bool parser::first(const string & h, set<string> & rs, bool e)
{
    map<string, bool> visited;

    // Call the recursive version with the visited map
    return first(h, visited, rs, e);
}

bool parser::first(const vector<string> & b, unsigned st, set<string> & rs, bool e)
{
    unsigned i;

    // This can be merged with the other FIRST...

    for(i = st; i < b.size(); i++) {
	const string & s = b[i];

	if(first(s, rs, false)) {
	    // If :empty: was seen, we continue with this body
	    continue;
	} else {
	    return false;
	}
    }

    if(e) {
	rs.insert(empty);
    }

    // This FIRST simply returns true if all elements contain :empty:
    return true;
}

void parser::follows(const string & fs, map<string, bool> & v, set<string> & rs)
{
    // If we've already done FOLLOWS on this symbol, return
    if(v[fs]) {
	return;
    }

    v[fs] = true;

    if(fs == "START") {
	rs.insert(string("$"));
    }

    map<string, list<vector<string> > >::const_iterator mi;

    // Iterate over all productions
    for(mi = productions.begin(); mi != productions.end(); mi++) {
	const string & head = mi->first;
	const list<vector<string> > & body = mi->second;

	list<vector<string> >::const_iterator li;

	for(li = body.begin(); li != body.end(); li++) {
	    const vector<string> & p = *li;
	    unsigned size = p.size();

	    // All but the last symbol
	    for(unsigned i = 0; i < size - 1; i++) {
		if(p[i] == fs) {
		    // FIRST of the remaining symbols
		    if(first(p, i + 1, rs, false)) {
			follows(head, v, rs);
		    }
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

bool parser::empty_check(const string & s)
{
    list<vector<string> >::const_iterator li;

    for(li = productions[s].begin(); li != productions[s].end(); li++) {
	const vector<string> & b = *li;

	if(b[0] == ":empty:") {
	    return true;
	}
    }

    return false;
}

void parser::closure(parser_state & ps, map<string, bool> & added, bool k)
{
    const list<parser_item> & items = k ? ps.kernel_items : ps.nonkernel_items;
    list<parser_item>::const_iterator ii;

    unsigned x;

    do {
	x = 0;

	for(ii = items.begin(); ii != items.end(); ii++) {
	    const parser_item & i = *ii;

	    // If this is the end of the item, nothing to do
	    if(i.index == i.symbols.size()) {
		continue;
	    }

	    // We are currently looking at something like:
	    //   E -> X . Y
	    // In this case, the symbol to investigate is Y

	    string s = i.symbols[i.index];

	    // No need to close on terminal symbols
	    if(symbol_is_terminal(s)) {
		continue;
	    }

	    // No need to continue if we've already added this symbol
	    if(added[s]) {
		continue;
	    }

	    // Lookup the symbol and create a new nonkernel item for
	    // each of the bodies

	    list<vector<string> >::const_iterator li;

	    // For each right side of each production, add a non-kernel item
	    for(li = productions[s].begin();
		li != productions[s].end(); li++) {
		const vector<string> & b = *li;
		parser_item pi(s, b);

		ps.nonkernel_items.push_back(pi);

		// ex: s == "NUM", NUM -> WS D WS | WS D NUM WS

		for(unsigned i = 0; i < b.size() - 1; i++) {
		    // Break as soon as a symbol doesn't evaluate
		    // to :empty:
		    if(!empty_check(b[i])) {
			break;
		    }

		    parser_item pi2(s, b, i + 1, b.size() - (i + 1));

		    ps.nonkernel_items.push_back(pi2);
		}
	    }

	    added[s] = true;

	    x++;
	}

    } while(x > 0);

    return;
}

void parser::closure(parser_state & ps)
{
    map<string, bool> added;
    list<parser_item>::const_iterator ki;

    // Kernel items
    closure(ps, added, true);

    // Nonkernel items
    closure(ps, added, false);

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

void parser::dump(const char * msg)
{
    map<string, list<vector<string> > >::const_iterator mi;

    if(msg) {
	cout << "[" << msg << "]" << endl;
    }

    cout << "productions:" << endl;

    // For each production...
    for(mi = productions.begin(); mi != productions.end(); mi++) {
	const string & h = mi->first;
	const list<vector<string> > & l = mi->second;

	list<vector<string> >::const_iterator li;

	// This handles multiple productions with the same head
	for(li = l.begin(); li != l.end(); li++) {
	    const vector<string> & v = *li;
	    unsigned size = v.size();

	    cout << " " << h << " -> ";

	    list<string>::const_iterator li2;

	    // Iterates through the symbols
	    for(unsigned i = 0; i < size; i++) {
		const string & s = v[i];

		cout << s;

		if(i < size - 1) {
		    cout << " ";
		}
	    }

	    cout << endl;
	}
    }

    cout << "parser state:" << endl;

    cout << "current token: " << token
	 << ", value: " << token_value << endl;

    cout << " symbol stack:" << endl;

    list<string>::const_iterator sy;

    for(sy = symbol_stack.begin(); sy != symbol_stack.end(); sy++) {
	cout << "  " << *sy << endl;
    }

    cout << " state stack:" << endl;

    list<parser_state>::const_iterator st;

    unsigned sn = 0;

    for(st = state_stack.begin(); st != state_stack.end(); st++) {
	parser_state ps = *st;

	cout << "  state " << sn++ << endl;

	ps.dump();
    }

    cout << " parse tree:" << endl;

    if(node_stack.size() > 0) {
	tree_node tn = node_stack.back();
	node_stack.pop_back();

	tn.dump();
    }

    return;
}

void parser::next_token(void)
{
    int state = 0;
    char c;

    token_value = "";

    for(;;) {
	c = cin.get();

	if(cin.eof()) {
	    token_value = "$";
	    token = "$";
	    return;
	}

	switch(state) {
	case 0: // start state
	    if(isspace(c)) {
		// skip white space (for now)
	    } else if(isdigit(c)) {
		token_value += c;
		state = 2;
	    } else if(isalpha(c)) {
		token_value += c;
		state = 1;
	    } else {
		token_value += c;
		token = c;
		return;
	    }
	    break;

	case 1: // ID
	    if(isalnum(c)) {
		token_value += c;
	    } else {
		cin.unget();
		token = ":id:";
		return;
	    }
	    break;

	case 2: // NUM
	    if(isdigit(c)) {
		token_value += c;
	    } else {
		cin.unget();
		token = ":num:";
		return;
	    }
	    break;
	}
    }

    return;
}
