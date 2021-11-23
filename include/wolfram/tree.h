#ifndef TREE_H
#define TREE_H


union wf_data {
        char    var;
        unsigned op;
        double  lit;
};

struct node {
        node *left   = nullptr;
        node *right  = nullptr;
        node *parent = nullptr;

        int type = 0;
        wf_data data;
};

/*
 * Jumps from node to node recursively. 
 * Then applies 'action' to the current node.
 *
 * Note! It calls action() before next jump.
 */
void visit_tree(node *root, void (*action)(node *nd));

node *create_node(node *parent, int type);
void delete_node(node *vet);

#define TREE_DEBUG

#ifdef TREE_DEBUG
void dump_tree(node *root);
#else /* TREE_DEBUG */
static inline void dump_tree(node *root) {}
#endif /* TREE_DEBUG */

#endif /* TREE_H */

