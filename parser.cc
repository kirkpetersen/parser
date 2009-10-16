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

ostream & operator<<(ostream & out, const symbol & s)
{
    if(s.value.empty()) {
	out << "[" << s.type << "]";
    } else {
	out << "[" << s.type << "," << s.value << "]";
    }

    return out;
}

void parser::reduce(parser_state & ps)
{
    list<parser_item>::const_iterator ki;

    for(ki = ps.kernel_items.begin(); ki != ps.kernel_items.end(); ki++) {
	const parser_item & pi = *ki;

	if(pi.index != pi.symbols.size()) {
	    continue;
	}

	symbol head = pi.head;
	unsigned n = pi.symbols.size();

	tree_node tn(head, false);

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
	    cout << "reduce: " << head.type << " -> ";

	    tn.dump_below();

	    cout << endl;
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
    list<vector<symbol> > sp = productions[start];

    if(sp.empty()) {
	cerr << "no start state!" << endl;
	return;
    }

    // Create the initial parser state
    parser_state ps;

    list<vector<symbol> >::const_iterator li;

    for(li = sp.begin(); li != sp.end(); li++) {
	parser_item pi = make_item(start, *li);

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
		 << ", token: " << token.type
		 << ", token_value: " << token.value
		 << endl;

	    cout.flush();
	}

	if(verbose > 1) {
	    dump("verbose dump (every loop)");
	}

	int cs = 0, cr = 0, ca = 0;

	check(ps.kernel_items, cs, cr, ca);
	check(ps.nonkernel_items, cs, cr, ca);

	if(verbose > 1) {
	    cout << "shifts: " << cs << ", "
		 << "reduces: " << cr << ", "
		 << "accepts: " << ca << endl;
	}

	if(cs > 0 && cr > 0) {
	    dump("shift/reduce conflict");
	    break;
	}

	if(cr > 1) {
	    dump("reduce/reduce conflict");
	    break;
	}

	if(ca > 0) {
	    cout << "ACCEPT!" << endl;
	    break;
	} else if(cs > 0) {
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

void parser::check(const list<parser_item> & l,
		   int & cs, int & cr, int & ca)
{
    list<parser_item>::const_iterator li;

    bool accept_check = (token == symbol("$"));

    // Check each item in the list
    for(li = l.begin(); li != l.end(); li++) {
	const parser_item & i = *li;

	// TODO also check for reduce whenever the remaining
	// symbols can evaluate to empty

	if(i.index < i.symbols.size()) {
	    // Not at the end of the item, check for shift

	    if(!terminal(i.symbols[i.index])) {
		continue;
	    }

	    if(token == i.symbols[i.index]) {
		cs++;
	    }
	    
	} else {
	    // At the end of this item, check for valid reduce or accept

	    set<symbol> rs;

	    follows(i.head, rs);

	    if(verbose > 2) {
		cout << "FOLLOWS(" << i.head << ")";
		dump_set(": ", rs);
	    }

	    if(rs.count(token) > 0) {
		cr++;
	    }

	    // Is this the correct check for ACCEPT?
	    if(accept_check && i.head == start && rs.count(token) > 0) {
		ca++;
	    }
	}
    }

    return;
}

parser_item parser::make_item(const symbol & h, const vector<symbol> & b)
{
    parser_item pi;

    pi.head = h;
    pi.symbols = b;
    pi.index = 0;

    return pi;
}

parser_item parser::make_item(const symbol & h, const vector<symbol> & b,
			      unsigned st)
{
    parser_item pi;

    pi.head = h;
    pi.index = 0;

    unsigned size = b.size() - st;

    pi.symbols.resize(size);

    for(unsigned i = st; i < size; i++) {
	pi.symbols[i] = b[i];
    }

    return pi;
}

void parser::build_items(const symbol & t, bool tl,
			 const list<parser_item> & l, list<parser_item> & n)
{
    list<parser_item>::const_iterator li;

    for(li = l.begin(); li != l.end(); li++) {
	const parser_item & i = *li;

	if(i.index >= i.symbols.size()) {
	    continue;
	}

	symbol s = i.symbols[i.index];

	if(tl && !terminal(s)) {
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

void parser::shift(const parser_state & ps, const symbol & t)
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
    node_stack.push_back(tree_node(token, true));

    return;
}

bool parser::first(const symbol & h, map<symbol, bool> & v, set<symbol> & rs)
{
    bool se = false;

    // If it is a terminal, it goes on the list and we return
    if(terminal(h)) {
	rs.insert(h);
	return se;
    }

    if(v[h]) {
	return se;
    }

    v[h] = true;

    list<vector<symbol> >::const_iterator li;

    if(productions.count(h) == 0) {
	return se;
    }

    for(li = productions[h].begin(); li != productions[h].end(); li++) {
	const vector<symbol> & b = *li;

	if(b.empty()) {
	    se = true;
	    continue;
	}

	unsigned i;

	for(i = 0; i < b.size(); i++) {
	    const symbol & s = b[i];

	    // Now add the FIRST of the current symbol (minus empty symbol)
	    if(first(s, v, rs)) {
		// If empty symbol was seen, we continue with this body
		se = true;
		continue;
	    } else {
		// Otherwise break and continue with the next body
		break;
	    }
	}

	// If we make it to the end of the body, it means that
	// All of the symbols include a potential empty body, so
	// include it in the set (if specified)
	if(i == b.size()) {
	    se = true;
	}
    }

    return se;
}

bool parser::first(const symbol & h, set<symbol> & rs)
{
    map<symbol, bool> visited;

    // Call the recursive version with the visited map
    return first(h, visited, rs);
}

bool parser::first(const vector<symbol> & b, unsigned st, set<symbol> & rs)
{
    unsigned i;

    // This can be merged with the other FIRST...

    for(i = st; i < b.size(); i++) {
	const symbol & s = b[i];

	if(first(s, rs)) {
	    // If empty body was seen, we continue with this body
	    continue;
	} else {
	    return false;
	}
    }

    // This FIRST simply returns true if all elements contain empty body
    return true;
}

void parser::follows(const symbol & fs, map<symbol, bool> & v, set<symbol> & rs)
{
    // If we've already done FOLLOWS on this symbol, return
    if(v[fs]) {
	return;
    }

    v[fs] = true;

    if(fs == start) {
	rs.insert(symbol("$"));
    }

    map<symbol, list<vector<symbol> > >::const_iterator mi;

    // Iterate over all productions
    for(mi = productions.begin(); mi != productions.end(); mi++) {
	const symbol & head = mi->first;
	const list<vector<symbol> > & body = mi->second;

	list<vector<symbol> >::const_iterator li;

	for(li = body.begin(); li != body.end(); li++) {
	    const vector<symbol> & p = *li;

	    if(p.empty()) {
		continue;
	    }

	    unsigned size = p.size();

	    // All but the last symbol
	    for(unsigned i = 0; i < size - 1; i++) {
		if(p[i] == fs) {
		    // FIRST of the remaining symbols
		    if(first(p, i + 1, rs)) {
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

void parser::follows(const symbol & fs, set<symbol> & rs)
{
    map<symbol, bool> visited;

    follows(fs, visited, rs);

    return;
}

bool parser::empty_check(const symbol & s)
{
    list<vector<symbol> >::const_iterator li;

    if(productions.count(s) == 0) {
	return false;
    }

    for(li = productions[s].begin(); li != productions[s].end(); li++) {
	const vector<symbol> & b = *li;

	if(b.empty()) {
	    return true;
	}
    }

    return false;
}

void parser::closure(parser_state & ps, map<symbol, bool> & added, bool k)
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

	    symbol s = i.symbols[i.index];

	    // No need to close on terminal symbols
	    if(terminal(s)) {
		continue;
	    }

	    // No need to continue if we've already added this symbol
	    if(added[s]) {
		continue;
	    }

	    added[s] = true;

	    if(productions.count(s) == 0) {
		continue;
	    }

	    // Lookup the symbol and create a new nonkernel item for
	    // each of the bodies

	    list<vector<symbol> >::const_iterator li;

	    // For each right side of each production, add a non-kernel item
	    for(li = productions[s].begin();
		li != productions[s].end(); li++) {
		const vector<symbol> & b = *li;

		if(b.empty()) {
		    continue;
		}

		parser_item pi = make_item(s, b);

		ps.nonkernel_items.push_back(pi);

		// ex: s == "NUM", NUM -> WS D WS | WS D NUM WS

		for(unsigned i = 0; i < b.size() - 1; i++) {
		    // Break as soon as a symbol doesn't evaluate
		    // to empty body
		    if(!empty_check(b[i])) {
			break;
		    }

		    parser_item pi2 = make_item(s, b, i + 1);

		    ps.nonkernel_items.push_back(pi2);
		}
	    }

	    x++;
	}

    } while(x > 0);

    return;
}

void parser::closure(parser_state & ps)
{
    map<symbol, bool> added;
    list<parser_item>::const_iterator ki;

    // Kernel items
    closure(ps, added, true);

    // Nonkernel items
    closure(ps, added, false);

    return;
}

void parser::load(const char * filename)
{
    int p = 0, done = 0, state = 0;

    vector<symbol> body;

    fstream fs(filename, fstream::in);

    while(!done) {
	string t;

	fs >> t;

	switch(state) {
	case 0: // start state
	    if(t == "%token") {
		state = 1;
	    } else {
		cerr << "tokenizer error" << endl;
		return;
	    }

	    break;

	case 1: // token names
	    if(t == "%%") {
		done = 1;
		break;
	    } else {
		tokens.insert(t);
	    }

	    break;
	}
    }

    symbol head;

    state = 0;

    for(;;) {
	string t;

	fs >> t;

	if(fs.eof()) {
	    break;
	}

	switch(state) {
	case 0: // start state
	    // always the head
	    head = symbol(t);
	    state = 1;

	    break;

	case 1: // expect a colon
	    if(t == ":") {
		state = 2;
	    } else {
		cerr << "error reading grammar: " << t << endl;
	    }

	    break;

	case 2:
	    if(t == "|") {
		// end of this body, put into productions
		productions[head].push_back(body);
		body.clear();

	    } else if(t == ";") {
		// end of this production
		productions[head].push_back(body);
		body.clear();
		state = 0;

		// If this is the first state...
		if(p++ == 0) {
		    vector<symbol> body2;

		    body2.push_back(head);

		    productions[start].push_back(body2);
		}
	    } else {
		// determine if the symbol is nonterminal,
		// terminal from the token list, or a literal (';')

		// fill in terminal afterwards, based on lhs check?

		// add to the current body
		if(t[0] == '\'' && t[t.size() - 1] == '\'') {
		    string t2 = t.substr(1, t.size() - 2);

		    literals.insert(symbol(t2));

		    body.push_back(symbol(t2));
		} else {
		    body.push_back(symbol(t));
		}
	    }

	    break;
	}
    }

    fs.close();

    return;
}

void parser::dump_set(const char * msg, const set<symbol> & rs)
{
    cout << msg;

    set<symbol>::const_iterator si;

    for(si = rs.begin(); si != rs.end(); si++) {
	cout << *si << " ";
    }

    cout << endl;

    return;
}

void parser::dump(const char * msg)
{
    map<symbol, list<vector<symbol> > >::const_iterator mi;

    if(msg) {
	cout << "[" << msg << "]" << endl;
    }

    dump_set("tokens: ", tokens);
    dump_set("literals: ", literals);

    cout << "productions:" << endl;

    // For each production...
    for(mi = productions.begin(); mi != productions.end(); mi++) {
	const symbol & h = mi->first;
	const list<vector<symbol> > & l = mi->second;

	list<vector<symbol> >::const_iterator li;

	// This handles multiple productions with the same head
	for(li = l.begin(); li != l.end(); li++) {
	    const vector<symbol> & v = *li;
	    unsigned size = v.size();

	    cout << " " << h.type << " -> ";

	    list<symbol>::const_iterator li2;

	    // Iterates through the symbols
	    for(unsigned i = 0; i < size; i++) {
		const symbol & s = v[i];

		if(terminal(s)) {
		    cout << s;
		} else {
		    cout << s.type;
		}

		if(i < size - 1) {
		    cout << " ";
		}
	    }

	    cout << endl;
	}
    }

    cout << "parser state:" << endl;

    cout << "current token: " << token.type
	 << ", value: " << token.value << endl;

    cout << " symbol stack:" << endl;

    list<symbol>::const_iterator sy;

    for(sy = symbol_stack.begin(); sy != symbol_stack.end(); sy++) {
	cout << "  " << *sy << endl;
    }

    cout << " state stack:" << endl;

    list<parser_state>::const_iterator st;

    unsigned sn = 0;

    for(st = state_stack.begin(); st != state_stack.end(); st++) {
	parser_state ps = *st;

	cout << "  state " << sn++ << endl;

	dump_state(ps);
    }

    cout << " parse tree:" << endl;

    if(node_stack.size() > 0) {
	const tree_node & tn = node_stack.back();

	tn.dump();
    }

    return;
}

void parser::dump_state(const parser_state & ps) {
    list<parser_item>::const_iterator li;

    cout << "  kernel items:" << endl;

    for(li = ps.kernel_items.begin(); li != ps.kernel_items.end(); li++) {
	parser_item pi = *li;

	dump_item(pi);
    }

    cout << "  nonkernel items:" << endl;

    for(li = ps.nonkernel_items.begin(); li != ps.nonkernel_items.end(); li++) {
	parser_item pi = *li;

	dump_item(pi);
    }

    return;
}

void parser::dump_item(const parser_item & pi) {
    cout << "   " << pi.head.type << " -> ";

    unsigned size = pi.symbols.size();

    for(unsigned i = 0; i < size; i++) {
	if(i == pi.index) {
	    // Add the dot
	    cout << ". ";
	}

	if(terminal(pi.symbols[i])) {
	    cout << pi.symbols[i];
	} else {
	    cout << pi.symbols[i].type;
	}

	if(i < size - 1) {
	    cout << " ";
	}
    }

    if(pi.index == size) {
	cout << " .";
    }

    cout << endl;

    return;
}

void parser::next_token(void)
{
    int state = 0;
    char c;

    token.value = "";

    for(;;) {
	c = cin.get();

	if(cin.eof()) {
	    token.value = "$";
	    token.type = "$";
	    return;
	}

	switch(state) {
	case 0: // start state
	    if(isspace(c)) {
		// skip white space (for now)
	    } else if(isdigit(c)) {
		token.value += c;
		state = 2;
	    } else if(isalpha(c)) {
		token.value += c;
		state = 1;
	    } else if(c == '\'') {
		// don't capture the opening quote
		state = 4;
	    } else if(c == ';' || c == ':' || c == '{' || c == '}'
		      || c == '(' || c == ')' || c == '?'
		      || c == '+' || c == '-' || c == '*' || c == '/'
		      || c == '=') {
		// single character literal
		token.type = c;
		token.value = c;
		return;
	    } else {
		// misc literal (hack)
		token.value += c;
		state = 3;
	    }
	    break;

	case 1: // ID | <ID-like literal from grammar>
	    if(isalnum(c) || c == '_') {
		token.value += c;
	    } else {
		if(literals.count(token.value) > 0) {
		    // This is an ID-like literal that was specified
		    // in the grammar
		    token.type = token.value;
		} else {
		    // Otherwise, it is an ID
		    token.type = "id";
		}

		cin.unget();
		return;
	    }
	    break;

	case 2: // NUM
	    if(isdigit(c)) {
		token.value += c;
	    } else {
		cin.unget();
		token.type = "num";
		return;
	    }
	    break;

	case 3: // literal
	    // Anything other than whitespace is part of the literal
	    if(isspace(c)) {
		// just consume space
		token.type = token.value;
		return;
	    } else {
		token.value += c;
	    }
	    break;

	case 4: // literal in the form of 'xyz'
	    if(c == '\'') {
		// don't capture the closing quote
		token.type = "literal";
		return;
	    } else {
		token.value += c;
	    }
	    break;
	}
    }

    return;
}
