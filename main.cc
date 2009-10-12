/*
 * main.cc
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

    if(!argv[1]) {
	return 1;
    }

    p.load(argv[1]);

    p.dump("after load");

    p.run();

    p.dump("after run");

    return 0;
}
