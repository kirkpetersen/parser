/*
 * parser_c.h
 */

struct tree_node * parser_run(const char * grammar, const char * input);

struct tree_node {
    char * head;
    char * value;

    unsigned terminal;

    struct tree_node ** nodes;
    unsigned node_count;

    union {
        int i;
        float f;
    } u;
};

struct tree_node * tree_node_new(unsigned count);

void tree_node_free(struct tree_node * tree);

void tree_node_set_head(struct tree_node * tree, const char * h);
void tree_node_set_value(struct tree_node * tree, const char * v);

struct tree_node * tree_node_find(struct tree_node * tn, const char * s);

void tree_node_dump(struct tree_node * tn, unsigned level);
