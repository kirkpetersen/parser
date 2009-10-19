/*
 * main.cc
 */

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include "parser.hh"

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

    std::cout << "loading " << argv[optind] << '\n';

    // Load the user's grammar file
    // (this bootstraps a parser to read and parse the grammar)
    p.load(argv[optind]);

    std::cout << "running\n";

    // Now run the user's parser
    p.run(std::cin);

    // Dump the tree
    p.dump_tree();

    p.dump_stats();

    return 0;
}
