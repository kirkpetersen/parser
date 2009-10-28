/*
 * parser.cc
 */

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include "parser.hh"

// parser

parser::~parser(void)
{
    struct tree_node * tn;

    // FIXME there are more than one tree on the stack
    tn = node_stack.front();
    node_stack.pop_front();
    tree_node_free(tn);

    return;
}

void parser::build_rule(const std::string & head, ...)
{
    parser_rule * rule = new parser_rule;
    va_list ap;

    // add to body with va_args
    va_start(ap, head);

    for(;;) {
	const char * p = va_arg(ap, const char *);

	if(!p) {
	    break;
	}

	rule->symbols.push_back(p);
    }

    va_end(ap);

    productions[head].push_back(rule);

    return;
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

    // Start production
    build_rule(start, "tokenizer_and_grammar", 0);

    build_rule("tokenizer_and_grammar", "tokenizer", "%%", "grammar", 0);
    build_rule("tokenizer", "token_line_list", 0);
    build_rule("token_line_list", "token_line", 0);
    build_rule("token_line_list", "token_line", "token_line_list", 0);
    build_rule("token_line", "%token", "token_list", 0);
    build_rule("token_line", "%start", "id", 0);
    build_rule("token_list", 0); // empty
    build_rule("token_list", "id", "token_list", 0);

    build_rule("grammar", "production_list", 0);
    build_rule("production_list", "production", ";", 0);
    build_rule("production_list", "production", ";", "production_list", 0);
    build_rule("production", "id", ":", "body_list", 0);
    build_rule("body_list", "body", "action", 0);
    build_rule("body_list", "body", "action", "|", "body_list", 0);
    build_rule("body_list", 0); // empty
    build_rule("body", "symbol_list", 0);
    build_rule("symbol_list", "symbol", 0);
    build_rule("symbol_list", "symbol", 0);
    build_rule("symbol_list", "symbol", "symbol_list", 0);
    build_rule("symbol", "id", 0);
    build_rule("symbol", "literal", 0);
    build_rule("symbol", "[", "num", "]", 0);
    build_rule("action", "{", "production", "}", 0);
    build_rule("action", "{", "}", 0);
    build_rule("action", 0); // empty

    return;
}

void parser::load_tokenizer(struct tree_node * tn)
{
    struct tree_node * tll = tn;

    while((tll = tree_node_find(tll, "token_line_list"))) {
	struct tree_node * line;

	if((line = tree_node_find(tll, "token_line"))) {
	    struct tree_node * x;

	    if((x = tree_node_find(line, "%token"))) {
		struct tree_node * tl = x;

		while((tl = tree_node_find(tl, "token_list"))) {
		    struct tree_node * id = tree_node_find(tl, "id");
		    tokens.insert(symbol(id->value));
		}
	    } else if((x = tree_node_find(line, "%start"))) {
		struct tree_node * s = tree_node_find(line, "id");
		parser_rule * rule = new parser_rule;

		rule->symbols.push_back(s->value);

		productions[start].push_back(rule);
	    }
	}
    }

    return;
}

void parser::load_production(struct tree_node * tn)
{
    struct tree_node * id;

    if((id = tree_node_find(tn, "id"))) {
	struct tree_node * bl = tn;

	// load body list into productions[id.head.value]

	while((bl = tree_node_find(bl, "body_list"))) {
	    parser_rule * rule = new parser_rule;
	    struct tree_node * body = bl;

	    while((body = tree_node_find(body, "body"))) {
		struct tree_node * sl = body;
		
		while((sl = tree_node_find(sl, "symbol_list"))) {
		    struct tree_node * s;

		    if((s = tree_node_find(sl, "symbol"))) {
			struct tree_node * x;

			if((x = tree_node_find(s, "id"))) {
			    rule->symbols.push_back(x->value);
			} else if((x = tree_node_find(s, "literal"))) {
			    literals.insert(x->value);
			    rule->symbols.push_back(x->value);
			}
			// action...
		    }
		}
	    }

	    productions[id->value].push_back(rule);
	}
    }

    return;
}

void parser::load_grammar(struct tree_node * tn)
{
    struct tree_node * p;

    if((p = tree_node_find(tn, "production"))) {
	load_production(p);
    }

    struct tree_node * pl = tn;

    while((pl = tree_node_find(pl, "production_list"))) {
	if((p = tree_node_find(pl, "production"))) {
	    load_production(p);
	}
    }

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

#if 0
    tree_node tg, tn = bp.tree();

    // TODO fix this to work...
    if(!find_node(tn, "tokenizer_and_grammar", tg)) {
	std::cerr << "invalid grammar\n";
	return;
    }
#endif

    struct tree_node * tg = bp.tree();

    struct tree_node * t, * g;

    if((t = tree_node_find(tg, "tokenizer"))) {
	load_tokenizer(t);
    }

    if((g = tree_node_find(tg, "grammar"))) {
	load_grammar(g);
    }

    fs.close();

    return;
}

parser_state * parser::sr(parser_state * ps, const symbol & t)
{
    // TODO build a temporary set of kernel items first and look them
    // up in the (kernel items -> state cache) if they exist, we can
    // skip the closure() step, which takes 50% of the time

    parser_state * ns = new parser_state;

    // Fill in the new state
    build_items(t, ps->kernel_items, ns->kernel_items);
    build_items(t, ps->nonkernel_items, ns->kernel_items);

    assert(!ns->kernel_items.empty());

    if(verbose > 2) {
	std::cout << "closure post shift\n";
    }

    closure(ns);

    return ns;
}

bool parser::reduce(parser_state * ps, const symbol & t, bool k)
{
    const std::set<parser_item *, parser_item_compare> & items = k ? ps->kernel_items : ps->nonkernel_items;

    std::set<parser_item *, parser_item_compare>::const_iterator ki;

    for(ki = items.begin(); ki != items.end(); ++ki) {

	// This can't be a reference or bad things happen when the
	// state is popped off the stack
	parser_item * pi = *ki;

	// Skip any item without the dot at the end
	if(pi->index != pi->symbols.size()) {
	    continue;
	}

	// Skip any item that doesn't match the token
	if(t != pi->terminal) {
	    continue;
	}

	unsigned n = pi->symbols.size();

	struct tree_node * pn = tree_node_new(n);

	pn->terminal = (unsigned)terminal(pi->head);
	tree_node_set_head(pn, pi->head.type.c_str());
	tree_node_set_value(pn, pi->head.value.c_str());

	// Pop all the appropriate symbols off the stack
	for(unsigned i = 0; i < n; i++) {
	    symbol_stack.pop_back();

	    // Pop this state off the stack
	    state_stack.pop_back();

	    // Set the current state to the previous
	    ps = state_stack.back();
	}

	symbol_stack.push_back(pi->head);

	// Now build the tree node
	for(unsigned i = n; i > 0; i--) {
	    pn->nodes[i - 1] = node_stack.back();
	    node_stack.pop_back();
	}

	// Push new tree node
	node_stack.push_back(pn);

	if(verbose > 0) {
	    std::cout << "reduce: " << pi->head.type << " -> ";

	    // FIXME tree_node_dump_below(pn)
	    // dump_tree_below(pn);

	    std::cout << '\n';

	    std::cout << "reduce item: ";

	    dump_item(pi);
	}

#if 0
	// FIXME
	// Replace this with generic handling of grammar actions
	//
	// experiment with syntax definition
	// this is probably where post actions would fire...
	if(head.type == "syntax") {
	    const tree_node & sn = node_stack.back();
	    tree_node h;

	    std::cout << "SYNTAX!\n";

	    if(h = find_node(sn, "id")) {
		std::cout << "head: " << h.head << '\n';
	    }

	    parser_rule * rule = new parser_rule;

	    // This should do the trick...
	    load_body(sn, rule->symbols);

	    productions[h.head.value].push_back(rule);
	}
#endif

	// Now that we've reduced, we need to look at the new
	// symbol on the stack and determine a new transition

	parser_state * ns = sr(ps, pi->head);

	state_stack.push_back(ns);

	return true;
    }

    return false;
}

void parser::run(std::istream & tin)
{
    std::list<parser_rule *> sp = productions[start];

    if(sp.empty()) {
	std::cerr << "no start state!\n";
	return;
    }

    // Get the first token immediately
    token = next_token(tin);

    // Create the initial parser state
    parser_state * ps = new parser_state;

    std::list<parser_rule *>::const_iterator li;

    for(li = sp.begin(); li != sp.end(); ++li) {
	parser_item * pi = make_item(start, (*li)->symbols, symbol("$"));
	ps->kernel_items.insert(pi);
    }

    if(verbose > 2) {
	std::cout << "closure for initial state\n";
    }

    closure(ps);

    state_stack.push_back(ps);

    unsigned loop = 0;

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
	check(token, ps->kernel_items, cs, cr, ca);
	check(token, ps->nonkernel_items, cs, cr, ca);

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

void parser::check(const symbol & t,
		   const std::set<parser_item *, parser_item_compare> & l,
		   int & cs, int & cr, int & ca)
{
    std::set<parser_item *>::const_iterator li;

    // Check each item in the list
    for(li = l.begin(); li != l.end(); ++li) {
	const parser_item * pi = *li;

	if(pi->index < pi->symbols.size()) {
	    // Not at the end of the item, so we should check for
	    // shift conditions

	    if(!terminal(pi->symbols[pi->index])) {
		continue;
	    }

	    if(t == pi->symbols[pi->index]) {
		if(verbose > 1) {
		    std::cout << "check: shift ";
		    dump_item(pi);
		}

		cs++;
	    }
	    
	} else {
	    // At the end of this item, check for valid reduce or accept

	    if(pi->head == start) {
		if(t == symbol("$")) {
		    if(verbose > 1) {
			std::cout << "check: accept ";
			dump_item(pi);
		    }

		    ca++;
		}
	    } else {
		if(t == pi->terminal) {
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

parser_item * parser::make_item(const symbol & h,
				const std::vector<symbol> & b,
				const symbol & t)
{
    parser_item * pi = new parser_item;

    pi->head = h;
    pi->symbols = b;
    pi->index = 0;
    pi->terminal = t;

    return pi;
}

void parser::build_items(const symbol & t,
			 const std::set<parser_item *, parser_item_compare> & l,
			 std::set<parser_item *, parser_item_compare> & n)
{
    std::set<parser_item *>::const_iterator li;

    stats.build_items_calls++;

    // For each item from the previous state...
    for(li = l.begin(); li != l.end(); ++li) {
	const parser_item * pi = *li;

	if(pi->index >= pi->symbols.size()) {
	    continue;
	}

	const symbol & s = pi->symbols[pi->index];

	// for ( each item [ A -> /a/ . X /B/ , a ] in I )
	//        add item [ A -> /a/ X . /B/ , a ] in J )  
	if(t == s) {
#if 1
	    // Is this cost effective?  Does it mess anything up?
	    if(pi->index < pi->symbols.size() - 1) {
		std::set<symbol> rs;

		first(pi, pi->index + 1, rs);

		if(rs.count(token) == 0) {
		    if(verbose > 4) {
			std::cout << "skipping item [token " << token << "]: ";
			dump_item(pi);
		    }

		    stats.build_item_skips++;

		    continue;
		}
	    }
#endif

	    parser_item * ni = new parser_item;

	    // Copy the item
	    *ni = *pi;

	    ni->index++;

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

void parser::shift(parser_state * ps, const symbol & t)
{
    if(verbose > 0) {
	std::cout << "shifting " << t << '\n';
    }

    symbol_stack.push_back(t);

    // New node for this symbol
    struct tree_node * tn = tree_node_new(0);

    tn->terminal = (unsigned)terminal(t);
    tree_node_set_head(tn, t.type.c_str());
    tree_node_set_value(tn, t.value.c_str());

    parser_state * ns = sr(ps, t);

    // Finish up shifting
    state_stack.push_back(ns);
    node_stack.push_back(tn);

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

    std::list<parser_rule *>::const_iterator li;

    if(productions.count(h.type) == 0) {
	std::cerr << "error!\n";
	return se;
    }

    for(li = productions[h.type].begin();
	li != productions[h.type].end(); ++li) {
	const parser_rule * rule = *li;

	if(rule->symbols.empty()) {
	    se = true;
	    continue;
	}

	unsigned i;

	for(i = 0; i < rule->symbols.size(); i++) {
	    const symbol & s = rule->symbols[i];

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
	if(i == rule->symbols.size()) {
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

void parser::first(const parser_item * pi, unsigned idx, std::set<symbol> & rs)
{
    unsigned size = pi->symbols.size();

    for(unsigned i = idx; i < size; i++) {
	const symbol & s = pi->symbols[i];

	if(first(s, rs)) {
	    // If empty body was seen, we continue with this body
	    continue;
	} else {
	    return;
	}
    }

    rs.insert(pi->terminal);

    return;
}

void parser::closure(parser_state * ps)
{
    std::queue<parser_item *> queue;

    std::set<parser_item *>::const_iterator li;

    stats.closure_calls++;

    // Push kernel items onto queue
    for(li = ps->kernel_items.begin(); li != ps->kernel_items.end(); ++li) {
	queue.push(*li);
    }

    while(!queue.empty()) {
	stats.closure_loops++;

	parser_item * pi = queue.front();
	queue.pop();

	if(pi->index >= pi->symbols.size()) {
	    continue;
	}

#if 1
	// Another possible optimization by comparing possible next
	// symbols in the item against the upcoming token
	{
	    std::set<symbol> rs0;

	    first(pi, pi->index, rs0);

	    if(rs0.count(token) == 0) {
		if(verbose > 4) {
		    std::cout << "closure: skipping item [token " << token << "]: ";
		    dump_item(pi);
		}

		stats.closure_skips++;

		continue;
	    }
	}
#endif

	if(verbose > 4) {
	    std::cout << "closing item: ";
	    dump_item(pi);
	}

	const symbol & s = pi->symbols[pi->index];

	if(terminal(s)) {
	    continue;
	}

	if(productions.count(s.type) == 0) {
	    std::cerr << "ERROR?\n";
	    continue;
	}

	std::set<symbol> rs;

	// FIRST() after the current symbol in the item
	// Given [ A -> /a/ . X /B/ , a ], find FIRST(/B/ a)
	first(pi, pi->index + 1, rs);

	if(verbose > 4) {
	    std::cout << "closure: FIRST(";

	    for(unsigned i = pi->index + 1; i < pi->symbols.size(); i++) {
		if(terminal(pi->symbols[i])) {
		    std::cout << pi->symbols[i] << ' ';
		} else {
		    std::cout << pi->symbols[i].type << ' ';
		}
	    }

	    std::cout << pi->terminal;

	    dump_set("): ", rs);
	}

	std::list<parser_rule *>::const_iterator li;

	// for ( each production B -> y in G' )
	for(li = productions[s.type].begin();
	    li != productions[s.type].end(); ++li) {
	    parser_rule * rule = *li;

	    if(verbose > 4) {
		std::cout << "body: " << s.type << " -> ";

		for(unsigned i = 0; i < rule->symbols.size(); i++) {
		    if(terminal(rule->symbols[i])) {
			std::cout << rule->symbols[i];
		    } else {
			std::cout << rule->symbols[i].type;
		    }

		    if(i < rule->symbols.size() - 1) {
			std::cout << ' ';
		    }
		}

		std::cout << '\n';

		dump_set("terminals: ", rs);
	    }

	    std::set<symbol>::const_iterator si;

	    bool dup_test = false;

	    // for ( each terminal b in FIRST(/B/ a) )
	    for(si = rs.begin(); si != rs.end(); ++si) {
		parser_item * pi2 = make_item(s, rule->symbols, *si);

		if(ps->nonkernel_items.count(pi2) > 0) {
		    stats.closure_item_duplicates++;
		    if(verbose > 4) {
			std::cout << "dup: ";
			dump_item(pi2);
		    }
		    dup_test = true;
		    continue;
		}

		if(verbose > 4) {
		    std::cout << "new: ";
		    dump_item(pi2);

		    if(dup_test) {
			std::cout << "caught!\n";
		    }
		}

		queue.push(pi2);

		stats.closure_item_non_duplicates++;

		// add [B -> . y, b] to set I
		ps->nonkernel_items.insert(pi2);
	    }
	}
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
