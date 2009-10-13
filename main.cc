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
    int c, verbose = 0;

    while((c = getopt(argc, argv, "v")) != EOF) {
	switch(c) {
	case 'v':
	    verbose++;
	    break;
	}
    }

    parser p(verbose);

    if(!argv[optind]) {
	return 1;
    }

    p.load(argv[optind]);

    p.dump("after load");

    p.run();

    p.dump("after run");

    return 0;
}
