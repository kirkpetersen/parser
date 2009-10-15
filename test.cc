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

int test1(void)
{
    parser p;

    p.load("test5.grammar");

    set<symbol>::const_iterator si;

    set<symbol> rs;
    bool e;

    e = p.first(symbol("START"), rs);
    p.dump_set("FIRST(START): ", rs);

    rs.clear();

    p.follows(symbol("START"), rs);
    p.dump_set("FOLLOWS(START): ", rs);
    cout << (e ? "[empty]" : "[not empty]") << endl;

    rs.clear();

    p.first(symbol("E"), rs);
    p.dump_set("FIRST(E): ", rs);

    rs.clear();

    p.follows(symbol("E"), rs);
    p.dump_set("FOLLOWS(E): ", rs);

    rs.clear();

    p.first(symbol("+"), rs);
    p.dump_set("FIRST(+): ", rs);

    rs.clear();

    p.follows(symbol("D"), rs);
    p.dump_set("FOLLOWS(D): ", rs);

    return 0;
}

int test2(void)
{
    parser p;

    set<symbol>::const_iterator si;

    set<symbol> rs;
    bool e;

    p.load("test8.grammar");

    e = p.first(symbol("F"), rs);
    p.dump_set("FIRST(F): ", rs);

    rs.clear();

    e = p.first(symbol("T"), rs);
    p.dump_set("FIRST(T): ", rs);

    rs.clear();

    e = p.first(symbol("E"), rs);
    p.dump_set("FIRST(E): ", rs);

    rs.clear();

    e = p.first(symbol("E2"), rs);
    p.dump_set("FIRST(E2): ", rs);

    rs.clear();

    e = p.first(symbol("T2"), rs);
    p.dump_set("FIRST(T2): ", rs);

    rs.clear();

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

    vector<symbol> b(3);

    b[0] = symbol("+");
    b[1] = symbol("T");
    b[2] = symbol("E2");

    e = p.first(b, 2, rs);
    p.dump_set("FIRST(E2): ", rs);

    return 0;
}

int test3(void)
{
    parser p;

    set<symbol>::const_iterator si;

    set<symbol> rs;

    cout << "loading C grammar" << endl;

    p.load("c.bnf");

    p.first(symbol("translation_unit"), rs);
    p.dump_set("FIRST(translation_unit): ", rs);

    p.dump("bnf dump");

    return 0;
}

int test4(void)
{
    parser p;

    set<symbol>::const_iterator si;

    set<symbol> rs;

    p.load("test3.grammar");

    p.follows(symbol("R"), rs);
    p.dump_set("FOLLOWS(R): ", rs);

    rs.clear();

    p.follows(symbol("L"), rs);
    p.dump_set("FOLLOWS(L): ", rs);

    rs.clear();

    p.dump("shift/reduce dump");

    return 0;
}

int main(int argc, char * argv[])
{
    int ret;

#if 0
    ret = test1();
    assert(ret == 0);
#endif

    ret = test2();
    assert(ret == 0);

    ret = test3();
    assert(ret == 0);

    ret = test4();
    assert(ret == 0);

    return 0;
}
