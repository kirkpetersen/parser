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

int main(int argc, char * argv[])
{
    parser p;

    p.load("test2.grammar");

    set<string>::const_iterator si;

    // FIRST(START)
    {
	set<string> rs;

	cout << "FIRST(START): " << endl;
	p.first(string("START"), rs);

	for(si = rs.begin(); si != rs.end(); si++) {
	    cout << "[" << *si << "]";
	}

	cout << endl;

    }

    // FOLLOWS(START)
    {
	set<string> rs;

	cout << "FOLLOWS(START): " << endl;
	p.follows(string("START"), rs);

	for(si = rs.begin(); si != rs.end(); si++) {
	    cout << "[" << *si << "]";
	}

	cout << endl;

    }

    // FIRST(E)
    {
	set<string> rs;

	cout << "FIRST(E): " << endl;
	p.first(string("E"), rs);

	for(si = rs.begin(); si != rs.end(); si++) {
	    cout << "[" << *si << "]";
	}

	cout << endl;
    }


    // FOLLOWS(E)
    {
	set<string> rs;

	cout << "FOLLOWS(E): " << endl;
	p.follows(string("E"), rs);

	for(si = rs.begin(); si != rs.end(); si++) {
	    cout << "[" << *si << "]";
	}

	cout << endl;
    }

    // FIRST(+)
    {
	set<string> rs;

	cout << "FIRST(+): " << endl;
	p.first(string("+"), rs);

	for(si = rs.begin(); si != rs.end(); si++) {
	    cout << "[" << *si << "]";
	}

	cout << endl;
    }

    return 0;
}
