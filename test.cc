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
int test_c(void);
int test_lr1(void);
int test_map(void);

struct test {
    const char * name;
    int (*fn)(void);
} tests[] = {
    { "empty", test_empty },
    { "c", test_c },
    { "lr1", test_lr1 },
    { "map", test_map },
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

    set<symbol>::const_iterator si;

    set<symbol> rs;
    bool e;

    p.load("test8.grammar");

    e = p.first(symbol("F"), rs);
    p.dump_set("FIRST(F): ", rs);
    assert(e == false);

    rs.clear();

    e = p.first(symbol("T"), rs);
    p.dump_set("FIRST(T): ", rs);
    assert(e == false);

    rs.clear();

    e = p.first(symbol("E"), rs);
    p.dump_set("FIRST(E): ", rs);
    assert(e == false);

    rs.clear();

    e = p.first(symbol("E2"), rs);
    p.dump_set("FIRST(E2): ", rs);
    assert(e == true);

    rs.clear();

    e = p.first(symbol("T2"), rs);
    p.dump_set("FIRST(T2): ", rs);
    assert(e == true);

    rs.clear();

#if 0
    p.follows(symbol("E"), rs);
    p.dump_set("FOLLOWS(E): ", rs);

    rs.clear();

    p.follows(symbol("E2"), rs);
    p.dump_set("FOLLOWS(E2): ", rs);

    rs.clear();

    p.follows(symbol("T"), rs);
    p.dump_set("FOLLOWS(T): ", rs);

    rs.clear();

    p.follows(symbol("T2"), rs);
    p.dump_set("FOLLOWS(T2): ", rs);

    rs.clear();

    p.follows(symbol("F"), rs);
    p.dump_set("FOLLOWS(F): ", rs);

    rs.clear();
#endif

    vector<symbol> b(3);

    b[0] = symbol("+");
    b[1] = symbol("T");
    b[2] = symbol("E2");

    e = p.first(b, 2, rs);
    p.dump_set("FIRST(E2): ", rs);

    return 0;
}

int test_c(void)
{
    parser p;

    set<symbol>::const_iterator si;

    set<symbol> rs;

    cout << "loading C grammar" << endl;

    p.load("c.grammar");

    p.first(symbol("translation_unit"), rs);
    p.dump_set("FIRST(translation_unit): ", rs);

    p.dump("dump");

    return 0;
}

int test_lr1(void)
{
    parser p;

    set<symbol>::const_iterator si;

    set<symbol> rs;

    p.load("test3.grammar");

#if 0
    p.follows(symbol("R"), rs);
    p.dump_set("FOLLOWS(R): ", rs);

    rs.clear();

    p.follows(symbol("L"), rs);
    p.dump_set("FOLLOWS(L): ", rs);

    rs.clear();
#endif

    p.dump("shift/reduce dump");

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
