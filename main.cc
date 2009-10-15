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
    int c, bnf = 0, verbose = 0;

    while((c = getopt(argc, argv, "bv")) != EOF) {
	switch(c) {
	case 'b':
	    bnf = 1;
	    break;

	case 'v':
	    verbose++;
	    break;
	}
    }

    parser p(verbose);

    if(!argv[optind]) {
	return 1;
    }

    if(bnf) {
	p.load_bnf(argv[optind]);
    } else {
	p.load(argv[optind]);
    }

    p.run();

    p.dump_tree();

    return 0;
}
