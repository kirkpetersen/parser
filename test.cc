/*
 * test.cc
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

int run_test(const char * name);

int test_empty(void);
int test_empty2(void);
int test_c(void);
int test_lr1(void);
int test_map(void);
int test_grammar(void);

struct test {
    const char * name;
    int (*fn)(void);
} tests[] = {
    { "empty", test_empty },
    { "empty2", test_empty2 },
    { "c", test_c },
    { "lr1", test_lr1 },
    { "map", test_map },
    { "grammar", test_grammar },
    { NULL, NULL }
};

int main(int argc, char * argv[])
{
    unsigned run = 0;
    int c, ret;

    while((c = getopt(argc, argv, "t:")) != EOF) {
	switch(c) {
	case 't':
	    ret = run_test(optarg);
	    run++;

	    if(ret != 0) {
		return ret;
	    }

	    break;
	}
    }

    if(run > 0) {
	return 0;
    }

    for(unsigned i = 0; tests[i].name; i++) {
	run_test(tests[i].name);
    }

    return 0;
}

int run_test(const char * name)
{
    for(unsigned i = 0; tests[i].name; i++) {
	if(strcmp(name, tests[i].name) == 0) {
	    int ret = tests[i].fn();

	    const char * result = (ret == 0) ? "PASS" : "FAIL";

	    cout << "test " << tests[i].name << ": " << result << endl;
	}
    }

    return 0;
}

int test_empty(void)
{
    parser p;

    set<string>::const_iterator si;

    set<string> rs;
    bool e;

    p.load("tests/test8.grammar");

    e = p.first("F", rs);
    p.dump_set("FIRST(F): ", rs);
    assert(rs.count("(") == 1);
    assert(rs.count("id") == 1);
    assert(e == false);

    rs.clear();

    e = p.first("T", rs);
    p.dump_set("FIRST(T): ", rs);
    assert(rs.count("(") == 1);
    assert(rs.count("id") == 1);
    assert(e == false);

    rs.clear();

    e = p.first("E", rs);
    p.dump_set("FIRST(E): ", rs);
    assert(rs.count("(") == 1);
    assert(rs.count("id") == 1);
    assert(e == false);

    rs.clear();

    e = p.first("E2", rs);
    p.dump_set("FIRST(E2): ", rs);
    assert(rs.count("+") == 1);
    assert(e == true);

    rs.clear();

    e = p.first("T2", rs);
    p.dump_set("FIRST(T2): ", rs);
    assert(rs.count("*") == 1);
    assert(e == true);

    rs.clear();

#if 0
    p.follows("E", rs);
    p.dump_set("FOLLOWS(E): ", rs);

    rs.clear();

    p.follows("E2", rs);
    p.dump_set("FOLLOWS(E2): ", rs);

    rs.clear();

    p.follows("T", rs);
    p.dump_set("FOLLOWS(T): ", rs);

    rs.clear();

    p.follows("T2", rs);
    p.dump_set("FOLLOWS(T2): ", rs);

    rs.clear();

    p.follows("F", rs);
    p.dump_set("FOLLOWS(F): ", rs);
#endif

    rs.clear();

    return 0;
}

int test_empty2(void)
{
    parser p;

    set<string>::const_iterator si;

    set<string> rs;
    bool e;

    p.load("tests/test6.grammar");

    e = p.first("s", rs);
    p.dump_set("FIRST(s): ", rs);
    assert(rs.count("id") == 1);
    assert(rs.count("num") == 1);
    assert(e == false);

    rs.clear();

    e = p.first("x", rs);
    p.dump_set("FIRST(x): ", rs);
    assert(rs.count("id") == 1);
    assert(e == true);

    rs.clear();

    e = p.first("y", rs);
    p.dump_set("FIRST(y): ", rs);
    assert(rs.count("num") == 1);
    assert(e == false);

    rs.clear();

    return 0;
}

int test_c(void)
{
    parser p;

    set<string>::const_iterator si;

    set<string> rs;

    cout << "loading C grammar" << endl;

    p.load("tests/c2.grammar");

    p.first("translation_unit", rs);
    p.dump_set("FIRST(translation_unit): ", rs);
    assert(rs.count("struct") > 0);

    return 0;
}

int test_lr1(void)
{
    parser p;

    set<string>::const_iterator si;

    set<string> rs;

    p.load("tests/test3.grammar");

    return 0;
}

int test_map(void)
{
    map<const char *, bool> m;

    assert(m.count("foo") == 0);

    m["foo"] = false;

    assert(m.count("foo") > 0);

    return 0;
}

int test_grammar(void)
{
    grammar g;

    parser_rule * rule = new parser_rule;

    rule->head = "foo";
    rule->symbols.push_back("bar");

    g["foo"].push_back(rule);

    assert(g.count("foo") == 1);

    return 0;
}
