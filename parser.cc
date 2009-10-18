/*
 * parser.cc
 */

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include "parser.hh"

std::ostream & operator<<(std::ostream & out, const symbol & s)
{
    if(s.value.empty()) {
	out << "[" << s.type << "]";
    } else {
	out << "[" << s.type << "," << s.value << "]";
    }

    return out;
}

bool operator==(const parser_item & p1, const parser_item & p2)
{
    return p1.head == p2.head && p1.symbols == p2.symbols
	&& p1.index == p2.index && p1.terminal == p2.terminal;
}

bool operator<(const parser_item & p1, const parser_item & p2)
{
    if(p1.head == p2.head && p1.symbols == p2.symbols
       && p1.index == p2.index && p1.terminal < p2.terminal) {
	return true;
    } else if(p1.head == p2.head && p1.symbols == p2.symbols
	      && p1.index < p2.index) {
	return true;
    } else if(p1.head == p2.head && p1.symbols < p2.symbols) {
	return true;
    } else if(p1.head < p2.head) {
	return true;
    }

    return false;
}

void parser::reduce(parser_state & ps)
{
    std::list<parser_item>::const_iterator ki;

    // Only kernel items can contain possible reductions
    for(ki = ps.kernel_items.begin(); ki != ps.kernel_items.end(); ++ki) {
	parser_item pi = *ki;

	if(verbose > 2) {
	    std::cout << "trying to reduce by: ";
	    dump_item(pi);
	}

	// Skip any item without the dot at the end
	if(pi.index != pi.symbols.size()) {
	    continue;
	}

	// Skip any item that doesn't match the token
	if(token != pi.terminal) {
	    continue;
	}

	symbol head = pi.head;
	unsigned n = pi.symbols.size();

	tree_node tn(head, false);

	if(verbose > 0) {
	    std::cout << "reduce: " << head.type << " -> ";

	    tn.dump_below();

	    std::cout << '\n';

	    std::cout << "reduce item: ";

	    dump_item(pi);
	}

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

	symbol_stack.push_back(head);

	// Push new tree node
	node_stack.push_back(tn);

	// Now that we've reduced, we need to look at the new
	// symbol on the stack and determine a new transition

	parser_state ns;

	// Fill in the new state
	build_items(head, ps.kernel_items, ns.kernel_items);
	build_items(head, ps.nonkernel_items, ns.kernel_items);

	if(ns.kernel_items.empty()) {
	    std::cout << "no match for " << head << '\n';
	    return;
	}

	if(verbose > 2) {
	    std::cout << "closure post reduce\n";
	}

	closure(ns);

	state_stack.push_back(ns);

	break;
    }

    return;
}

void parser::run(void)
{
    std::list<std::vector<symbol> > sp = productions[start];

    if(sp.empty()) {
	std::cerr << "no start state!\n";
	return;
    }

    // Create the initial parser state
    parser_state ps;

    std::list<std::vector<symbol> >::const_iterator li;

    for(li = sp.begin(); li != sp.end(); ++li) {
	parser_item pi = make_item(start, *li, symbol("$"));
	ps.kernel_items.push_back(pi);
    }

    if(verbose > 2) {
	std::cout << "closure for initial state\n";
    }

    closure(ps);

    state_stack.push_back(ps);

    unsigned loop = 0;

    // Get the first token
    next_token();

    for(;;) {
	if(verbose > 0) {
	    std::cout << "LOOP: " << loop++
		      << ", token: " << token.type
		      << ", token_value: " << token.value
		      << '\n';

	    std::cout.flush();
	}

	if(verbose > 2) {
	    dump("verbose dump (every loop)");
	}

	int cs = 0, cr = 0, ca = 0;

	// Check for shift, reduce, or accept conditions
	check(ps.kernel_items, cs, cr, ca);
	check(ps.nonkernel_items, cs, cr, ca);

	if(verbose > 0) {
	    std::cout << "shifts: " << cs << ", "
		      << "reduces: " << cr << ", "
		      << "accepts: " << ca << '\n';
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
	    std::cout << "ACCEPT!\n";
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
	    dump("ERROR!");
	    break;
	}
    }

    return;
}

void parser::check(const std::list<parser_item> & l,
		   int & cs, int & cr, int & ca)
{
    std::list<parser_item>::const_iterator li;

    // Check each item in the list
    for(li = l.begin(); li != l.end(); ++li) {
	parser_item pi = *li;

	if(pi.index < pi.symbols.size()) {
	    // Not at the end of the item, so we should check for
	    // shift conditions

	    if(!terminal(pi.symbols[pi.index])) {
		continue;
	    }

	    if(token == pi.symbols[pi.index]) {
		if(verbose > 1) {
		    std::cout << "check: shift ";
		    dump_item(pi);
		}

		cs++;
	    }
	    
	} else {
	    // At the end of this item, check for valid reduce or accept

	    if(pi.head == start) {
		if(token == symbol("$")) {
		    if(verbose > 1) {
			std::cout << "check: accept ";
			dump_item(pi);
		    }

		    ca++;
		}
	    } else {
		if(token == pi.terminal) {
		    if(verbose > 1) {
			std::cout << "check: reduce ";
			dump_item(pi);
		    }

		    cr++;
		}
	    }
	}
    }

    return;
}

parser_item parser::make_item(const symbol & h, const std::vector<symbol> & b,
			      const symbol & t)
{
    parser_item pi;

    pi.head = h;
    pi.symbols = b;
    pi.index = 0;
    pi.terminal = t;

    return pi;
}

parser_item parser::make_item(const symbol & h, const std::vector<symbol> & b,
			      const symbol & t, unsigned st)
{
    parser_item pi;

    pi.head = h;
    pi.index = 0;

    unsigned size = b.size() - st;

    pi.symbols.resize(size);

    for(unsigned i = st; i < size; i++) {
	pi.symbols[i] = b[i];
    }

    pi.terminal = t;

    return pi;
}

void parser::build_items(const symbol & t,
			 const std::list<parser_item> & l,
			 std::list<parser_item> & n)
{
    std::list<parser_item>::const_iterator li;

    // For each item from the previous state...
    for(li = l.begin(); li != l.end(); ++li) {
	parser_item pi = *li;

	if(pi.index >= pi.symbols.size()) {
	    continue;
	}

	symbol s = pi.symbols[pi.index];

	// for ( each item [ A -> /a/ . X /B/ , a ] in I )
	//        add item [ A -> /a/ X . /B/ , a ] in J )  
	if(t == s) {
	    parser_item ni = pi;

	    ni.index++;

	    if(verbose > 1) {
		std::cout << "building new item: ";
		dump_item(ni);
	    }

	    // Add the new item to the new state
	    n.push_back(ni);
	}
    }

    return;
}

void parser::shift(const parser_state & ps, const symbol & t)
{
    parser_state ns;

    if(verbose > 0) {
	std::cout << "shifting " << t << '\n';
    }

    // Fill in the new state
    build_items(token, ps.kernel_items, ns.kernel_items);
    build_items(token, ps.nonkernel_items, ns.kernel_items);

    if(ns.kernel_items.empty()) {
	std::cout << "no match for token " << t << '\n';
	return;
    }

    if(verbose > 2) {
	std::cout << "closure post shift\n";
    }

    closure(ns);

    // Finish up shifting
    symbol_stack.push_back(token);
    state_stack.push_back(ns);

    // New node for this symbol
    node_stack.push_back(tree_node(token, true));

    return;
}

bool parser::first(const symbol & h, std::map<symbol, bool> & v,
		   std::set<symbol> & rs)
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

    std::list<std::vector<symbol> >::const_iterator li;

    if(productions.count(h) == 0) {
	return se;
    }

    for(li = productions[h].begin(); li != productions[h].end(); ++li) {
	const std::vector<symbol> & b = *li;

	if(b.empty()) {
	    se = true;
	    continue;
	}

	unsigned i;

	for(i = 0; i < b.size(); i++) {
	    const symbol & s = b[i];

	    // Now add the FIRST of the current symbol
	    if(first(s, v, rs)) {
		// If empty symbol was seen, we continue with this body
		se = true;
		continue;
	    } else {
		// Otherwise break and continue with the next body
		break;
	    }
	}

	// If we make it to the end of the body, it means that All of
	// the symbols include an empty body, so indicate that we've
	// seen it
	if(i == b.size()) {
	    se = true;
	}
    }

    return se;
}

bool parser::first(const symbol & h, std::set<symbol> & rs)
{
    std::map<symbol, bool> visited;

    // Call the recursive version with the visited map
    return first(h, visited, rs);
}

bool parser::first(const std::vector<symbol> & b, unsigned st,
		   std::set<symbol> & rs)
{
    unsigned i;

    // This can possibly be merged with the other FIRST...

    for(i = st; i < b.size(); i++) {
	const symbol & s = b[i];

	if(first(s, rs)) {
	    // If empty body was seen, we continue with this body
	    continue;
	} else {
	    return false;
	}
    }

    // Unlike the other FIRST() functions above, this returns true
    // only if all symbols yield a possible empty
    return true;
}

void parser::follows(const symbol & fs, std::map<symbol, bool> & v,
		     std::set<symbol> & rs)
{
    // If we've already done FOLLOWS on this symbol, return
    if(v[fs]) {
	return;
    }

    v[fs] = true;

    if(fs == start) {
	rs.insert(symbol("$"));
    }

    std::map<symbol, std::list<std::vector<symbol> > >::const_iterator mi;

    // Iterate over all productions
    for(mi = productions.begin(); mi != productions.end(); ++mi) {
	const symbol & head = mi->first;
	const std::list<std::vector<symbol> > & body = mi->second;

	std::list<std::vector<symbol> >::const_iterator li;

	for(li = body.begin(); li != body.end(); ++li) {
	    const std::vector<symbol> & p = *li;

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

void parser::follows(const symbol & fs, std::set<symbol> & rs)
{
    std::map<symbol, bool> visited;

    follows(fs, visited, rs);

    return;
}

unsigned parser::closure(parser_state & ps,
			 std::set<parser_item> & v,
			 std::set<parser_item> & a,
			 const std::list<parser_item> & items)
{
    unsigned x = 0;

    std::list<parser_item>::const_iterator ii;

    // for ( each item [A -> /a/ . B /B/, a] in I )
    for(ii = items.begin(); ii != items.end(); ++ii) {
	const parser_item & pi = *ii;

	if(verbose > 3) {
	    std::cout << "closure trying item: ";
	    dump_item(pi);
	}

	// Only process each item once
	if(v.count(pi) > 0) {
	    continue;
	}

	v.insert(pi);

	// If this is the end of the item, nothing to do
	if(pi.index >= pi.symbols.size()) {
	    continue;
	}

	symbol s = pi.symbols[pi.index];

	// No need to close on terminal symbols
	if(terminal(s)) {
	    continue;
	}

	// Sanity check
	if(productions.count(s) == 0) {
	    std::cerr << "ERROR?\n";
	    continue;
	}

	// Create temporary vector for [/B/, a]
	std::vector<symbol> beta(pi.symbols);

	beta.push_back(pi.terminal);

	std::set<symbol> rs;

	// FIRST(/B/ a)
	first(beta, pi.index + 1, rs);

	if(verbose > 2) {
	    std::cout << "FIRST(";

	    for(unsigned i = pi.index + 1; i < beta.size(); i++) {
		std::cout << beta[i];

		if(i < beta.size() - 1) {
		    std::cout << " ";
		}
	    }

	    std::cout << ")";

	    dump_set(": ", rs);
	}

	std::list<std::vector<symbol> >::const_iterator li;

	// for ( each production B -> y in G' )
	for(li = productions[s].begin(); li != productions[s].end(); ++li) {
	    const std::vector<symbol> & b = *li;

	    if(b.empty()) {
		continue;
	    }

	    std::set<symbol>::const_iterator si;

	    // for ( each terminal b in FIRST(/B/ a) )
	    for(si = rs.begin(); si != rs.end(); ++si) {
		parser_item pi2 = make_item(s, b, *si);

		if(a.count(pi2) > 0) {
		    continue;
		}

		if(verbose > 2) {
		    std::cout << "closure creating: ";
		    dump_item(pi2);
		}

		a.insert(pi2);

		// add [B -> . y, b] to set I
		ps.nonkernel_items.push_back(pi2);
		x++;
	    }
	}
    }

    return x;
}

void parser::closure(parser_state & ps)
{
    std::set<parser_item> visited;
    std::set<parser_item> added;

    closure(ps, visited, added, ps.kernel_items);

    for(;;) {
	if(closure(ps, visited, added, ps.nonkernel_items) == 0) {
	    break;
	}
    }

    return;
}

void parser::load(const char * filename)
{
    int p = 0, done = 0, state = 0;

    std::vector<symbol> body;

    std::fstream fs(filename, std::fstream::in);

    while(!done) {
	std::string t;

	fs >> t;

	switch(state) {
	case 0: // start state
	    if(t == "%%") {
		done = 1;
		break;
	    } else if(t == "%token") {
		state = 1;
	    } else if(t == "%start") {
		state = 2;
	    } else {
		std::cerr << "tokenizer error\n";
		return;
	    }

	    break;

	case 1: // token names
	    if(t == "%%") {
		done = 1;
		break;
	    } else if(t == "%token") {
		// simply skip this item and stay in this state
	    } else if(t == "%start") {
		state = 2;
	    } else {
		tokens.insert(t);
	    }

	    break;

	case 2:
	    // Make sure we don't try to create the start state later
	    p++;

	    // Now create the start state
	    {
		std::vector<symbol> body2;

		body2.push_back(t);

		productions[start].push_back(body2);
	    }

	    state = 0;
	    break;
	}
    }

    symbol head;

    state = 0;

    for(;;) {
	std::string t;

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
		std::cerr << "error reading grammar: " << t << '\n';
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
		    std::vector<symbol> body2;

		    body2.push_back(head);

		    productions[start].push_back(body2);
		}
	    } else {
		// determine if the symbol is nonterminal,
		// terminal from the token list, or a literal (';')

		// fill in terminal afterwards, based on lhs check?

		// add to the current body
		if(t[0] == '\'' && t[t.size() - 1] == '\'') {
		    std::string t2 = t.substr(1, t.size() - 2);

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

void parser::dump_set(const char * msg, const std::set<symbol> & rs)
{
    std::cout << msg;

    std::set<symbol>::const_iterator si;

    for(si = rs.begin(); si != rs.end(); ++si) {
	std::cout << *si << " ";
    }

    std::cout << '\n';

    return;
}

void parser::dump(const char * msg)
{
    std::map<symbol, std::list<std::vector<symbol> > >::const_iterator mi;

    if(msg) {
	std::cout << "[" << msg << "]\n";
    }

    if(verbose > 3) {
	dump_set("tokens: ", tokens);
	dump_set("literals: ", literals);

	std::cout << "productions:\n";

	// For each production...
	for(mi = productions.begin(); mi != productions.end(); ++mi) {
	    const symbol & h = mi->first;
	    const std::list<std::vector<symbol> > & l = mi->second;

	    std::list<std::vector<symbol> >::const_iterator li;

	    // This handles multiple productions with the same head
	    for(li = l.begin(); li != l.end(); ++li) {
		const std::vector<symbol> & v = *li;
		unsigned size = v.size();

		std::cout << " " << h.type << " -> ";

		std::list<symbol>::const_iterator li2;

		// Iterates through the symbols
		for(unsigned i = 0; i < size; i++) {
		    const symbol & s = v[i];

		    if(terminal(s)) {
			std::cout << s;
		    } else {
			std::cout << s.type;
		    }

		    if(i < size - 1) {
			std::cout << " ";
		    }
		}

		std::cout << '\n';
	    }
	}
    }

    std::cout << "parser state:\n";

    std::cout << "current token: " << token.type
	 << ", value: " << token.value << '\n';

    std::cout << " symbol stack:\n";

    std::list<symbol>::const_iterator sy;

    for(sy = symbol_stack.begin(); sy != symbol_stack.end(); ++sy) {
	std::cout << "  " << *sy << '\n';
    }

    std::cout << " state stack:\n";

    std::list<parser_state>::const_iterator st;

    unsigned sn = 0;

    for(st = state_stack.begin(); st != state_stack.end(); ++st) {
	parser_state ps = *st;

	std::cout << "  state " << sn++ << '\n';

	dump_state(ps, 3);
    }

    if(verbose > 3) {
	std::cout << " parse tree:\n";

	if(node_stack.size() > 0) {
	    const tree_node & tn = node_stack.back();

	    tn.dump();
	}
    }

    return;
}

void parser::dump_state(const parser_state & ps, unsigned spaces) {
    std::list<parser_item>::const_iterator li;

    for(unsigned i = 0; i < spaces; i++) { std::cout << ' '; }

    std::cout << "kernel items:\n";

    for(li = ps.kernel_items.begin(); li != ps.kernel_items.end(); ++li) {
	parser_item pi = *li;

	dump_item(pi, spaces + 1);
    }

    for(unsigned i = 0; i < spaces; i++) { std::cout << ' '; }

    std::cout << "nonkernel items:\n";

    for(li = ps.nonkernel_items.begin(); li != ps.nonkernel_items.end(); ++li) {
	parser_item pi = *li;

	dump_item(pi, spaces + 1);
    }

    return;
}

void parser::dump_item(const parser_item & pi, unsigned spaces) {
    for(unsigned i = 0; i < spaces; i++) { std::cout << ' '; }

    std::cout << pi.head.type << " -> ";

    unsigned size = pi.symbols.size();

    for(unsigned i = 0; i < size; i++) {
	if(i == pi.index) {
	    // Add the dot
	    std::cout << ". ";
	}

	if(terminal(pi.symbols[i])) {
	    std::cout << pi.symbols[i];
	} else {
	    std::cout << pi.symbols[i].type;
	}

	if(i < size - 1) {
	    std::cout << " ";
	}
    }

    if(pi.index == size) {
	std::cout << " .";
    }

    std::cout << " {" << pi.terminal.type << "}\n";

    return;
}

void parser::next_token(void)
{
    int state = 0;
    char c;

    token.value = "";

    for(;;) {
	c = std::cin.get();

	if(std::cin.eof()) {
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
	    } else if(c == '"') {
		// don't capture the opening quote
		state = 5;
	    } else if(c == '\'') {
		// don't capture the opening quote
		state = 4;
	    } else if(c == ';' || c == ':' || c == '{' || c == '}'
		      || c == '(' || c == ')' || c == '?'
		      || c == '+' || c == '-' || c == '*' || c == '/'
		      || c == '=' || c == '[' || c == ']') {
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

		std::cin.unget();
		return;
	    }
	    break;

	case 2: // NUM
	    if(isdigit(c)) {
		token.value += c;
	    } else {
		std::cin.unget();
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

	case 5:
	    if(c == '"') {
		// don't capture the closing quote
		token.type = "string";
		return;
	    } else {
		token.value += c;
	    }
	    break;
	}
    }

    return;
}
