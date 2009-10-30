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
    int c, mode = 1, verbose = 0;

    while((c = getopt(argc, argv, "mv")) != EOF) {
	switch(c) {
	case 'm':
	    mode = 0;
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

    std::cout << "loading " << argv[optind] << '\n';

    // Load the user's grammar file
    // (this bootstraps a parser to read and parse the grammar)
    p.load(argv[optind]);

    std::cout << "running\n";

    if(mode) {
	std::string t, tv;
	bool nt;

	p.next_token(std::cin, t, tv);

	for(;;) {
	    nt = p.step(t, tv);

	    if(nt) {
		if(t == "$") {
		    break;
		}

		p.next_token(std::cin, t, tv);
	    }
	}
    } else {
	// Now run the user's parser
	p.run(std::cin);
    }

    tree_node_dump(p.tree(), 0);

    p.dump_stats();

    return 0;
}
