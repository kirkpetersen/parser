/*
 * tree_node.c
 */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser_c.h"

struct tree_node * tree_node_new(unsigned count)
{
    struct tree_node * tn;

    tn = (struct tree_node *)calloc(1, sizeof(struct tree_node));
    assert(tn);

    tn->node_count = count;

    if(tn->node_count > 0) {
	tn->nodes = (struct tree_node **)calloc(tn->node_count,
						sizeof(struct tree_node));
	assert(tn->nodes);
    }

    return tn;
}

void tree_node_free(struct tree_node * tn)
{
    unsigned i;

    if(tn->head) {
	free(tn->head);
	tn->head = NULL;
    }

    if(tn->value) {
	free(tn->value);
	tn->value = NULL;
    }

    for(i = 0; i < tn->node_count; i++) {
	struct tree_node * tn2 = tn->nodes[i];

	tn->nodes[i] = NULL;

	if(tn2) {
	    tree_node_free(tn2);
	}
    }

    if(tn->nodes) {
	free(tn->nodes);
	tn->nodes = NULL;
    }

    return;
}

void tree_node_set_head(struct tree_node * tree, const char * h)
{
    tree->head = strdup(h);

    return;
}

void tree_node_set_value(struct tree_node * tree, const char * v)
{
    tree->value = strdup(v);

    return;
}

struct tree_node * tree_node_find(struct tree_node * tn, const char * s)
{
    unsigned i;

    for(i = 0; i < tn->node_count; i++) {
	if(strcmp(s, tn->nodes[i]->head) == 0) {
	    return tn->nodes[i];
	}
    }

    return NULL;
}

void tree_node_dump_below(struct tree_node * tn)
{
    unsigned i;

    if(tn->terminal) {
	printf("%s ", tn->value);
    }

    for(i = 0; i < tn->node_count; i++) {
	tree_node_dump_below(tn->nodes[i]);
    }

    return;
}

void tree_node_dump(struct tree_node * tn, unsigned level)
{
    unsigned i;

    if(tn == NULL) {
	return;
    }

    for(i = 0; i < level; i++) { putchar(' '); }

    tree_node_dump_below(tn);

    printf(" <- %s\n", tn->head);

    for(i = 0; i < tn->node_count; i++) {
	tree_node_dump(tn->nodes[i], level + 1);
    }

    return;
}
