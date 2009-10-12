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
    if(isupper(s[0])) {
	return false;
    } else {
	return true;
    }
}

void parser::reduce(parser_state & ps)
{
    list<parser_item>::const_iterator ki;

    for(ki = ps.kernel_items.begin(); ki != ps.kernel_items.end(); ki++) {
	parser_item pi = *ki;

	if(pi.index != pi.symbols.size()) {
	    continue;
	}

	string head = pi.head;
	unsigned n = pi.symbols.size();

	tree_node * tn = new tree_node(head);

	// Pop all the appropriate symbols off the stack
	for(unsigned i = 0; i < n; i++) {
	    string s = symbol_stack.back();

	    // Add to the tree
	    tn->insert(node_stack.back());
	    node_stack.pop_back();

	    symbol_stack.pop_back();

	    // Pop this state off the stack
	    state_stack.pop_back();

	    // Set the current state to the previous
	    ps = state_stack.back();
	}

	cout << "reduce: " << head << " -> ... " << endl;

	symbol_stack.push_back(head);

	// Push new tree node
	node_stack.push_back(tn);

	// Now that we've reduced, we need to look at the new
	// symbol on the stack and determine a new transition

	parser_state ns;

	// Fill in the new state
	build_items(head, false, ps.kernel_items, ns.kernel_items);
	build_items(head, false, ps.nonkernel_items, ns.kernel_items);

#if 0
	// This should never be necessary (?)
	closure(ns);
#endif

	state_stack.push_back(ns);

	break;
    }

    return;
}

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

    dump("dumping before executing parser");

    cout << "[starting the parse]" << endl;

    // Get the first token
    string t = token();

    for(;;) {
	cout << "LOOP: " << loop++ << " [token: " << t << "]" << endl;

	int cs = 0, cr = 0, ca = 0;

	check(t, ps.kernel_items, cs, cr, ca);
	check(t, ps.nonkernel_items, cs, cr, ca);

	cout << "shifts: " << cs << ", "
	     << "reduces: " << cr << ", "
	     << "accepts: " << ca << endl;

	// ACCEPT
	if(ca > 0) {
	    cout << "ACCEPT!" << endl;
	    break;
	}

	if(cs > 0 && cr > 0) {
	    dump("shift/reduce conflict");
	    break;
	}

	if(cs > 1) {
	    dump("shift/shift conflict");
	    break;
	}

	if(cr > 1) {
	    dump("reduce/reduce conflict");
	    break;
	}

	if(cs > 0) {
	    shift(ps, t);

	    // Set the current state to the one shift() created
	    ps = state_stack.back();

	    // Time to read a new token...
	    t = token();

	    cout << "new token '" << t << "'" << endl;
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
	parser_item i = *li;

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
	parser_item i = *li;

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

    cout << "shifting '" << t << "'" << endl;

    // Fill in the new state
    build_items(t, true, ps.kernel_items, ns.kernel_items);
    build_items(t, true, ps.nonkernel_items, ns.kernel_items);

    if(ns.kernel_items.empty()) {
	cout << "no match for token " << t << endl;
	return;
    }

    closure(ns);

    // Finish up shifting
    symbol_stack.push_back(t);
    state_stack.push_back(ns);

    // New node for this symbol
    node_stack.push_back(new tree_node(t));

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

    if(fs == "START") {
	rs.insert(string("$"));
    }

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

void parser::dump(const char * msg)
{
    map<string, list<vector<string> > >::const_iterator mi;

    if(msg) {
	cout << "[" << msg << "]" << endl;
    }

    cout << "productions:" << endl;

    // For each production...
    for(mi = productions.begin(); mi != productions.end(); mi++) {
	string h = mi->first;
	list<vector<string> > l = mi->second;

	list<vector<string> >::const_iterator li;

	// This handles multiple productions with the same head
	for(li = l.begin(); li != l.end(); li++) {
	    vector<string> v = *li;
	    unsigned size = v.size();

	    cout << " " << h << " -> ";

	    list<string>::const_iterator li2;

	    // Iterates through the symbols
	    for(unsigned i = 0; i < size; i++) {
		string s = v[i];

		cout << s;

		if(i < size - 1) {
		    cout << " ";
		}
	    }

	    cout << endl;
	}
    }

    cout << "parser state:" << endl;

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

	cout << " state " << sn++ << endl;

	ps.dump();
    }

    cout << " parse tree:" << endl;

    if(node_stack.size() == 1) {
	tree_node * tn = node_stack.back();

	tn->dump(2);
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
