/*
 * main.c
 */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parser_c.h"

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

    struct tree_node * tn;

    if(!argv[optind]) {
        return 1;
    }

    tn = parser_run(argv[optind], NULL);

    tree_node_dump(tn, 0);

    return 0;
}
