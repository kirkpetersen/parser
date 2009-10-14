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

void dump_set(const char * msg, const set<string> & rs)
{
    cout << msg;

    set<string>::const_iterator si;

    for(si = rs.begin(); si != rs.end(); si++) {
	cout << "[" << *si << "]";
    }

    cout << endl;

    return;
}

int test1(void)
{
    parser p;

    p.load("test7.grammar");

    set<string>::const_iterator si;

    set<string> rs;

    p.first(string("START"), rs);
    dump_set("FIRST(START): ", rs);

    rs.clear();

    p.follows(string("START"), rs);
    dump_set("FOLLOWS(START): ", rs);

    rs.clear();

    p.first(string("E"), rs);
    dump_set("FIRST(E): ", rs);

    rs.clear();

    p.follows(string("E"), rs);
    dump_set("FOLLOWS(E): ", rs);

    rs.clear();

    p.first(string("+"), rs);
    dump_set("FIRST(+): ", rs);

    rs.clear();

    p.follows(string("D"), rs);
    dump_set("FOLLOWS(D): ", rs);

    return 0;
}

int test2(void)
{
    parser p;

    set<string>::const_iterator si;

    set<string> rs;

    p.load("test8.grammar");

    p.first(string("F"), rs);
    dump_set("FIRST(F): ", rs);

    rs.clear();

    p.first(string("T"), rs);
    dump_set("FIRST(T): ", rs);

    rs.clear();

    p.first(string("E"), rs);
    dump_set("FIRST(E): ", rs);

    rs.clear();

    p.first(string("E2"), rs);
    dump_set("FIRST(E2): ", rs);

    rs.clear();

    p.first(string("T2"), rs);
    dump_set("FIRST(T2): ", rs);

    rs.clear();

    p.follows(string("E"), rs);
    dump_set("FOLLOWS(E): ", rs);

    rs.clear();

    p.follows(string("E2"), rs);
    dump_set("FOLLOWS(E2): ", rs);

    rs.clear();

    p.follows(string("T"), rs);
    dump_set("FOLLOWS(T): ", rs);

    rs.clear();

    p.follows(string("T2"), rs);
    dump_set("FOLLOWS(T2): ", rs);

    rs.clear();

    p.follows(string("F"), rs);
    dump_set("FOLLOWS(F): ", rs);

    rs.clear();

    vector<string> b(3);

    b[0] = string("+");
    b[1] = string("T");
    b[2] = string("E2");

    p.first(b, 2, rs);
    dump_set("FIRST(E2): ", rs);

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

    return 0;
}
