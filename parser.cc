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

void parser::bootstrap()
{
    tokens.insert(symbol("id"));
    tokens.insert(symbol("literal"));

    literals.insert(symbol("%%"));
    literals.insert(symbol("%token"));
    literals.insert(symbol(":"));
    literals.insert(symbol(";"));
    literals.insert(symbol("|"));
    literals.insert(symbol("{"));
    literals.insert(symbol("}"));

    std::vector<symbol> body;

    // Start production
    body.push_back(symbol("tokenizer_and_grammar"));
    productions[start].push_back(body);

    body.clear();

    body.push_back(symbol("tokenizer"));
    body.push_back(symbol("%%"));
    body.push_back(symbol("grammar"));
    productions["tokenizer_and_grammar"].push_back(body);

    body.clear();

    body.push_back(symbol("token_line_list"));
    productions["tokenizer"].push_back(body);

    body.clear();

    body.push_back(symbol("token_line"));
    productions["token_line_list"].push_back(body);

    body.clear();

    body.push_back(symbol("token_line"));
    body.push_back(symbol("token_line_list"));
    productions["token_line_list"].push_back(body);

    body.clear();

    body.push_back(symbol("%token"));
    body.push_back(symbol("token_list"));
    productions["token_line"].push_back(body);

    body.clear();

    body.push_back(symbol("%start"));
    body.push_back(symbol("id"));
    productions["token_line"].push_back(body);

    body.clear();

    // empty
    productions["token_list"].push_back(body);

    body.clear();

    body.push_back(symbol("id"));
    body.push_back(symbol("token_list"));
    productions["token_list"].push_back(body);

    body.clear();

    body.push_back(symbol("production_list"));
    productions["grammar"].push_back(body);

    body.clear();

    body.push_back(symbol("production"));
    productions["production_list"].push_back(body);

    body.clear();

    body.push_back(symbol("production"));
    body.push_back(symbol("production_list"));
    productions["production_list"].push_back(body);

    body.clear();

    body.push_back(symbol("id"));
    body.push_back(symbol(":"));
    body.push_back(symbol("body_list"));
    body.push_back(symbol(";"));
    productions["production"].push_back(body);

    body.clear();

    body.push_back(symbol("body"));
    productions["body_list"].push_back(body);

    body.clear();

    body.push_back(symbol("body"));
    body.push_back(symbol("|"));
    body.push_back(symbol("body_list"));
    productions["body_list"].push_back(body);

    body.clear();

    // empty
    productions["body_list"].push_back(body);

    body.clear();

    body.push_back(symbol("symbol_list"));
    productions["body"].push_back(body);

    body.clear();

    body.push_back(symbol("symbol"));
    productions["symbol_list"].push_back(body);

    body.clear();

    body.push_back(symbol("symbol"));
    body.push_back(symbol("symbol_list"));
    productions["symbol_list"].push_back(body);

    body.clear();

    body.push_back(symbol("id"));
    productions["symbol"].push_back(body);

    body.clear();

    body.push_back(symbol("literal"));
    productions["symbol"].push_back(body);

    body.clear();

    body.push_back(symbol("{"));
    body.push_back(symbol("}"));
    productions["symbol"].push_back(body);

    return;
}

bool parser::find_node(const tree_node & tn, const std::string & s,
		       tree_node & t) const {
    std::list<tree_node>::const_iterator ti;

    for(ti = tn.nodes.begin(); ti != tn.nodes.end(); ++ti) {
	const tree_node & tn2 = *ti;

	if(s == tn2.head.type) {
	    t = tn2;
	    return true;
	}
    }

    return false;
}

void parser::load_symbol(const tree_node & tn, std::vector<symbol> & b)
{
    tree_node i, l;

    if(find_node(tn, "id", i)) {
	b.push_back(symbol(i.head.value));
    } else if(find_node(tn, "literal", l)) {
	literals.insert(symbol(l.head.value));
	b.push_back(symbol(l.head.value));
    }

    return;
}

void parser::load_symbol_list(const tree_node & tn, std::vector<symbol> & b)
{
    tree_node s, sl;

    if(find_node(tn, "symbol", s)) {
	load_symbol(s, b);
    }

    if(find_node(tn, "symbol_list", sl)) {
	load_symbol_list(sl, b);
    }

    return;
}

void parser::load_body(const tree_node & tn, std::vector<symbol> & b)
{
    tree_node sl;

    if(find_node(tn, "symbol_list", sl)) {
	load_symbol_list(sl, b);
    }

    return;
}

void parser::load_body_list(const tree_node & tn,
			    std::list<std::vector<symbol> > & p)
{
    tree_node b, bl, pipe;

    std::vector<symbol> body;

    // Look for the body
    if(find_node(tn, "body", b)) {
	load_body(b, body);
    }

    // Either push the body or <empty> onto the production list
    p.push_back(body);

    // Continue with the body_list if there is one
    if(find_node(tn, "body_list", bl)) {
	load_body_list(bl, p);
    }

    return;
}

void parser::load_production(const tree_node & tn)
{
    tree_node id, bl;

    if(find_node(tn, "id", id)) {
	if(find_node(tn, "body_list", bl)) {
	    load_body_list(bl, productions[id.head.value]);
	}
    }

    return;
}

void parser::load_production_list(const tree_node & tn)
{
    tree_node p, pl;

    if(find_node(tn, "production", p)) {
	load_production(p);
    }

    if(find_node(tn, "production_list", pl)) {
	load_production_list(pl);
    }

    return;
}

void parser::load_token_list(const tree_node & tn)
{
    tree_node id, tl;

    if(find_node(tn, "id", id)) {
	tokens.insert(symbol(id.head.value));
    }

    if(find_node(tn, "token_list", tl)) {
	load_token_list(tl);
    }

    return;
}

void parser::load_token_line(const tree_node & tn)
{
    tree_node t, tl, s, ss;

    if(find_node(tn, "%token", t)) {
	if(find_node(tn, "token_list", tl)) {
	    load_token_list(tl);
	}
    }

    if(find_node(tn, "%start", s)) {
	if(find_node(tn, "id", ss)) {
	    std::vector<symbol> body;

	    body.push_back(ss.head.value);

	    productions[start].push_back(body);
	}
    }

    return;
}

void parser::load_token_line_list(const tree_node & tn)
{
    tree_node tl, tll;

    if(find_node(tn, "token_line", tl)) {
	load_token_line(tl);
    }

    if(find_node(tn, "token_line_list", tll)) {
	load_token_line_list(tll);
    }

    return;
}

void parser::load_tokenizer(const tree_node & tn)
{
    tree_node tll;

    if(find_node(tn, "token_line_list", tll)) {
	load_token_line_list(tn);
    }

    return;
}

void parser::load_grammar(const tree_node & tn)
{
    load_production_list(tn);

    return;
}

void parser::load(const char * filename)
{
    const char * env = getenv("PARSER_BOOTSTRAP_VERBOSE");
    unsigned v = env ? atoi(env) : 0;

    parser bp(v);

    // This parser is used to load the user's grammar file
    bp.bootstrap();

    std::ifstream fs(filename, std::fstream::in);

    bp.run(fs);

#if 1
    tree_node tg = bp.tree();
#else
    tree_node tg, tn = bp.tree();

    // TODO fix this to work...
    if(!find_node(tn, "tokenizer_and_grammar", tg)) {
	std::cerr << "invalid grammar\n";
	return;
    }
#endif

    std::list<tree_node>::const_iterator ti;

    tree_node t, g;

    if(find_node(tg, "tokenizer", t)) {
	load_tokenizer(t);
    }

    if(find_node(tg, "grammar", g)) {
	load_grammar(g);
    }

    fs.close();

    return;
}

bool parser::reduce(parser_state & ps, const symbol & t, bool k)
{
    const std::set<parser_item> & items = k ? ps.kernel_items : ps.nonkernel_items;
    std::set<parser_item>::const_iterator ki;

    for(ki = items.begin(); ki != items.end(); ++ki) {

	// This can't be a reference or bad things happen when the
	// state is popped off the stack
	parser_item pi = *ki;

	// Skip any item without the dot at the end
	if(pi.index != pi.symbols.size()) {
	    continue;
	}

	// Skip any item that doesn't match the token
	if(t != pi.terminal) {
	    continue;
	}

	symbol head = pi.head;
	unsigned n = pi.symbols.size();

	tree_node pn;

	pn.head = head;

	// Pop all the appropriate symbols off the stack
	for(unsigned i = 0; i < n; i++) {
	    // Add to the tree
	    pn.nodes.push_front(node_stack.back());

	    node_stack.pop_back();

	    symbol_stack.pop_back();

	    // Pop this state off the stack
	    state_stack.pop_back();

	    // Set the current state to the previous
	    ps = state_stack.back();
	}

	symbol_stack.push_back(head);

	// Push new tree node
	node_stack.push_back(pn);

	if(verbose > 0) {
	    std::cout << "reduce: " << head.type << " -> ";

	    dump_tree_below(pn);

	    std::cout << '\n';

	    std::cout << "reduce item: ";

	    dump_item(pi);
	}

#if 1
	// FIXME
	// experiment with syntax definition
	// this is probably where post actions would fire...
	if(head.type == "syntax") {
	    const tree_node & sn = node_stack.back();
	    tree_node h, sl;

	    std::cout << "SYNTAX!\n";

	    if(find_node(sn, "id", h)) {
		std::cout << "head: " << h.head << '\n';
	    }

	    std::vector<symbol> body;

	    // This should do the trick...
	    load_body(sn, body);

	    productions[h.head.value].push_back(body);
	}
#endif

	// Now that we've reduced, we need to look at the new
	// symbol on the stack and determine a new transition

	parser_state ns;

	// Fill in the new state
	build_items(head, ps.kernel_items, ns.kernel_items);
	build_items(head, ps.nonkernel_items, ns.kernel_items);

	if(ns.kernel_items.empty()) {
	    std::cout << "no match for " << head << '\n';
	    return false;
	}

	if(verbose > 2) {
	    std::cout << "closure post reduce\n";
	}

	closure(ns);

	state_stack.push_back(ns);

	return true;
    }

    return false;
}

void parser::run(std::istream & tin)
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
	const parser_item & pi = make_item(start, *li, symbol("$"));
	ps.kernel_items.insert(pi);
    }

    if(verbose > 2) {
	std::cout << "closure for initial state\n";
    }

    closure(ps);

    state_stack.push_back(ps);

    unsigned loop = 0;

    // Get the first token
    token = next_token(tin);

    for(;;) {
	stats.loops++;

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
	check(token, ps.kernel_items, cs, cr, ca);
	check(token, ps.nonkernel_items, cs, cr, ca);

	stats.shifts += cs;
	stats.reduces += cr;
	stats.accepts += ca;

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
	    break;
	} else if(cs > 0) {
	    symbol old = token;

	    // Grab the next token now
	    // This might help us optimize creation of states/items
	    token = next_token(tin);

	    shift(ps, old);

	    // Set the current state to the one shift() created
	    ps = state_stack.back();
	} else if(cr > 0) {
	    bool match = reduce(ps, token);

	    // Only check nonkernel items if there isn't a match with
	    // the kernel items. Most reduces are in the kernel items.
	    if(!match) {
		if(verbose > 3) {
		    std::cout << "checking nonkernel items for reduce\n";
		}

		reduce(ps, token, false);
	    }

	    ps = state_stack.back();

	} else {
	    dump("ERROR!");
	    break;
	}
    }

    return;
}

void parser::check(const symbol & t, const std::set<parser_item> & l,
		   int & cs, int & cr, int & ca)
{
    std::set<parser_item>::const_iterator li;

    // Check each item in the list
    for(li = l.begin(); li != l.end(); ++li) {
	const parser_item & pi = *li;

	if(pi.index < pi.symbols.size()) {
	    // Not at the end of the item, so we should check for
	    // shift conditions

	    if(!terminal(pi.symbols[pi.index])) {
		continue;
	    }

	    if(t == pi.symbols[pi.index]) {
		if(verbose > 1) {
		    std::cout << "check: shift ";
		    dump_item(pi);
		}

		cs++;
	    }
	    
	} else {
	    // At the end of this item, check for valid reduce or accept

	    if(pi.head == start) {
		if(t == symbol("$")) {
		    if(verbose > 1) {
			std::cout << "check: accept ";
			dump_item(pi);
		    }

		    ca++;
		}
	    } else {
		if(t == pi.terminal) {
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

void parser::build_items(const symbol & t,
			 const std::set<parser_item> & l,
			 std::set<parser_item> & n)
{
    std::set<parser_item>::const_iterator li;

    stats.build_items_calls++;

    // For each item from the previous state...
    for(li = l.begin(); li != l.end(); ++li) {
	const parser_item & pi = *li;

	if(pi.index >= pi.symbols.size()) {
	    continue;
	}

	const symbol & s = pi.symbols[pi.index];

	// for ( each item [ A -> /a/ . X /B/ , a ] in I )
	//        add item [ A -> /a/ X . /B/ , a ] in J )  
	if(t == s) {
	    parser_item ni = pi;

	    ni.index++;

	    // TODO can certain states be skipped?
	    // 'token' is the next applicable token
	    // if new item is [ A -> /a/ X . /B/ , a ]
	    // if ni.symbols[ni.index] is terminal but doesn't
	    // match token, skip it
	    //
	    // or would FIRST(/B/ a) be worth it, to eliminate all
	    // items? does that change the algorithm?

	    if(verbose > 2) {
		std::cout << "building new item: ";
		dump_item(ni);
	    }

	    stats.build_item++;

	    // Add the new item to the new state
	    n.insert(ni);
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
    build_items(t, ps.kernel_items, ns.kernel_items);
    build_items(t, ps.nonkernel_items, ns.kernel_items);

    if(ns.kernel_items.empty()) {
	std::cout << "no match for token " << t << '\n';
	return;
    }

    if(verbose > 2) {
	std::cout << "closure post shift\n";
    }

    closure(ns);

    // Finish up shifting
    symbol_stack.push_back(t);
    state_stack.push_back(ns);

    // New node for this symbol
    {
	tree_node tn;

	tn.head = t;

	node_stack.push_back(tn);
    }

    return;
}

bool parser::first(const symbol & h, std::set<symbol> & v,
		   std::set<symbol> & rs)
{
    bool se = false;

    // If it is a terminal, it goes on the list and we return
    if(terminal(h)) {
	rs.insert(h);
	return se;
    }

    if(v.count(h) > 0) {
	return se;
    }

    v.insert(h);

    std::list<std::vector<symbol> >::const_iterator li;

    if(productions.count(h.type) == 0) {
	std::cerr << "error!\n";
	return se;
    }

    for(li = productions[h.type].begin();
	li != productions[h.type].end(); ++li) {
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
		continue;
	    } else {
		// Otherwise break and continue with the next body
		break;
	    }
	}

	// If we make it to the end of the body, it means that all of
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
    std::set<symbol> visited;

    // Call the recursive version with the visited map
    return first(h, visited, rs);
}

void parser::first(const parser_item & pi, std::set<symbol> & rs)
{
    unsigned size = pi.symbols.size();

    for(unsigned i = pi.index + 1; i < size; i++) {
	const symbol & s = pi.symbols[i];

	if(first(s, rs)) {
	    // If empty body was seen, we continue with this body
	    continue;
	} else {
	    return;
	}
    }

    rs.insert(pi.terminal);

    return;
}

void parser::closure(parser_state & ps)
{
    std::queue<parser_item> queue;

    std::set<parser_item>::const_iterator li;

    stats.closure_calls++;

    // Push kernel items onto queue
    for(li = ps.kernel_items.begin(); li != ps.kernel_items.end(); ++li) {
	queue.push(*li);
    }

    while(!queue.empty()) {
	stats.closure_loops++;

	parser_item pi = queue.front();
	queue.pop();

	if(pi.index >= pi.symbols.size()) {
	    continue;
	}

	if(verbose > 4) {
	    std::cout << "closing item: ";
	    dump_item(pi);
	}

	const symbol & s = pi.symbols[pi.index];

	if(terminal(s)) {
	    continue;
	}

	if(productions.count(s.type) == 0) {
	    std::cerr << "ERROR?\n";
	    continue;
	}

	std::set<symbol> rs;

	// FIRST(/B/ a)
	first(pi, rs);

	if(verbose > 4) {
	    std::cout << "closure: FIRST(";

	    for(unsigned i = pi.index + 1; i < pi.symbols.size(); i++) {
		if(terminal(pi.symbols[i])) {
		    std::cout << pi.symbols[i] << ' ';
		} else {
		    std::cout << pi.symbols[i].type << ' ';
		}
	    }

	    std::cout << pi.terminal;

	    dump_set("): ", rs);
	}

	std::list<std::vector<symbol> >::const_iterator li;

	// for ( each production B -> y in G' )
	for(li = productions[s.type].begin();
	    li != productions[s.type].end(); ++li) {
	    const std::vector<symbol> & b = *li;

	    if(verbose > 4) {
		std::cout << "body: " << s.type << " -> ";

		for(unsigned i = 0; i < b.size(); i++) {
		    if(terminal(b[i])) {
			std::cout << b[i];
		    } else {
			std::cout << b[i].type;
		    }

		    if(i < b.size() - 1) {
			std::cout << ' ';
		    }
		}

		std::cout << '\n';

		dump_set("terminals: ", rs);
	    }

	    std::set<symbol>::const_iterator si;

	    // for ( each terminal b in FIRST(/B/ a) )
	    for(si = rs.begin(); si != rs.end(); ++si) {
		parser_item pi2 = make_item(s, b, *si);

		if(ps.nonkernel_items.count(pi2) > 0) {
		    stats.closure_item_duplicates++;
		    if(verbose > 4) {
			std::cout << "dup: ";
			dump_item(pi2);
		    }
		    continue;
		}

		if(verbose > 4) {
		    std::cout << "new: ";
		    dump_item(pi2);
		}

		queue.push(pi2);

		stats.closure_item_non_duplicates++;

		// add [B -> . y, b] to set I
		ps.nonkernel_items.insert(pi2);
	    }
	}
    }

    return;
}

void parser::dump_set(const char * msg, const std::set<symbol> & rs)
{
    std::cout << msg;

    std::set<symbol>::const_iterator si;

    for(si = rs.begin(); si != rs.end(); ++si) {
	const symbol & s = *si;
	std::cout << s.type << " ";
    }

    std::cout << '\n';

    return;
}

void parser::dump_stats(void)
{
    std::cout << "parser stats:\n";
    std::cout << "build item: " << stats.build_item << '\n';
    std::cout << "loops: " << stats.loops << '\n';
    std::cout << "shifts: " << stats.shifts << '\n';
    std::cout << "reduces: " << stats.reduces << '\n';
    std::cout << "accepts: " << stats.accepts << '\n';
    std::cout << "build_items_calls: " << stats.build_items_calls << '\n';
    std::cout << "closure_calls: " << stats.closure_calls << '\n';
    std::cout << "closure_loops: " << stats.closure_loops << '\n';
    std::cout << "closure_item_duplicates: " << stats.closure_item_duplicates << '\n';
    std::cout << "closure_item_non_duplicates: " << stats.closure_item_non_duplicates << '\n';

    return;
}

void parser::dump_grammar(void)
{
    dump_set("tokens: ", tokens);
    dump_set("literals: ", literals);

    std::cout << "productions:\n";

    std::map<std::string, std::list<std::vector<symbol> > >::const_iterator mi;

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
		    std::cout << ' ';
		}
	    }

	    std::cout << '\n';
	}
    }

    return;
}

void parser::dump(const char * msg)
{
    if(msg) {
	std::cout << "[" << msg << "]\n";
    }

    if(verbose > 3) {
	dump_grammar();
    }

    std::cout << "parser state:\n";

    std::cout << "current token: " << token.type
	 << ", value: " << token.value << '\n';

    std::cout << " symbol stack:\n";

    std::deque<symbol>::const_iterator sy;

    for(sy = symbol_stack.begin(); sy != symbol_stack.end(); ++sy) {
	const symbol & s = *sy;

	if(terminal(s)) {
	    std::cout << "  " << s << '\n';
	} else {
	    std::cout << "  " << s.type << '\n';
	}
    }

    std::deque<parser_state>::const_iterator st;

    unsigned sn = 0;

    if(verbose > 3) {
	std::cout << " state stack:\n";

	for(st = state_stack.begin(); st != state_stack.end(); ++st) {
	    const parser_state & ps = *st;

	    std::cout << "  state " << sn++ << '\n';

	    dump_state(ps, 3);
	}
    } else {
	std::cout << " state stack (kernel items of top state only):\n";

	if(!state_stack.empty()) {
	    const parser_state & ps = state_stack.back();

	    std::cout << "  state " << state_stack.size() - 1 << '\n';

	    dump_state(ps, 3);
	}
    }

    if(verbose > 3) {
	std::cout << " parse tree:\n";

	if(node_stack.size() > 0) {
	    dump_tree(node_stack.back());
	}
    }

    return;
}

void parser::dump_state(const parser_state & ps, unsigned spaces) {
    std::set<parser_item>::const_iterator li;

    for(unsigned i = 0; i < spaces; i++) { std::cout << ' '; }

    std::cout << "kernel items:\n";

    for(li = ps.kernel_items.begin(); li != ps.kernel_items.end(); ++li) {
	const parser_item & pi = *li;

	dump_item(pi, spaces + 1);
    }

    if(verbose > 3) {
	for(unsigned i = 0; i < spaces; i++) { std::cout << ' '; }

	std::cout << "nonkernel items:\n";

	for(li = ps.nonkernel_items.begin(); li != ps.nonkernel_items.end(); ++li) {
	    const parser_item & pi = *li;

	    dump_item(pi, spaces + 1);
	}
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

	std::cout << " ";
    }

    if(pi.index == size) {
	std::cout << ".";
    }

    std::cout << " {" << pi.terminal.type << "}\n";

    return;
}

void parser::dump_tree(void) const
{
    if(!node_stack.empty()) {
	dump_tree(node_stack.back());
    }

    return;
}

void parser::dump_tree_below(const tree_node & tn) const
{
    if(terminal(tn.head)) {
	std::cout << tn.head.value << " ";
    }

    std::list<tree_node>::const_iterator ti;

    for(ti = tn.nodes.begin(); ti != tn.nodes.end(); ++ti) {
	dump_tree_below(*ti);
    }

    return;
}

void parser::dump_tree(const tree_node & tn, unsigned level) const
{
    for(unsigned i = 0; i < level; i++) { std::cout << ' '; }

    dump_tree_below(tn);

    std::cout << " <- " << tn.head << '\n';

    std::list<tree_node>::const_iterator ti;

    for(ti = tn.nodes.begin(); ti != tn.nodes.end(); ++ti) {
	dump_tree(*ti, level + 1);
    }

    return;
}

symbol parser::next_token(std::istream & tin)
{
    symbol t;
    int state = 0;
    char c;

    t.value = "";

    for(;;) {
	c = tin.get();

	if(tin.eof()) {
	    return symbol("$", "$");
	}

	switch(state) {
	case 0: // start state
	    if(isspace(c)) {
		// skip white space (for now)
	    } else if(isdigit(c)) {
		t.value += c;
		state = 2;
	    } else if(isalpha(c)) {
		t.value += c;
		state = 1;
	    } else if(c == '"') {
		t.value += c;
		state = 5;
	    } else if(c == '\'') {
		// don't capture the opening quote
		state = 4;
	    } else if(c == '-') {
		// could be '-' or '->'
		t.value += c;
		state = 6;
	    } else if(c == ';' || c == ':' || c == '{' || c == '}'
		      || c == '(' || c == ')' || c == '?' || c == '&'
		      || c == '+' || c == '*' || c == '/'
		      || c == '=' || c == '[' || c == ']') {
		// single character literal
		t.type = c;
		t.value = c;
		return t;
	    } else {
		// misc literal (hack)
		t.value += c;
		state = 3;
	    }
	    break;

	case 1: // ID | <ID-like literal from grammar>
	    if(isalnum(c) || c == '_') {
		t.value += c;
	    } else {
		if(literals.count(t.value) > 0) {
		    // This is an ID-like literal that was specified
		    // in the grammar
		    t.type = t.value;
		} else {
		    // Otherwise, it is an ID
		    t.type = "id";
		}

		tin.unget();
		return t;
	    }
	    break;

	case 2: // NUM
	    if(isdigit(c)) {
		t.value += c;
	    } else {
		tin.unget();
		t.type = "num";
		return t;
	    }
	    break;

	case 3: // literal
	    // Anything other than whitespace is part of the literal
	    if(isspace(c)) {
		// just consume space
		t.type = t.value;
		return t;
	    } else {
		t.value += c;
	    }
	    break;

	case 4: // literal in the form of 'xyz'
	    if(c == '\'') {
		// don't capture the closing quote
		t.type = "literal";
		return t;
	    } else {
		t.value += c;
	    }
	    break;

	case 5:
	    if(c == '"') {
		t.value += c;
		t.type = "string";
		return t;
	    } else {
		t.value += c;
	    }
	    break;

	case 6:
	    if(c == '>') {
		t.value += c;
		t.type = t.value;
		return t;
	    } else {
		tin.unget();
		t.type = t.value;
		return t;
	    }

	    break;
	}
    }

    return t;
}
