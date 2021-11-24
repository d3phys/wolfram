#ifndef TREE_H
#define TREE_H

union wf_value {
        char    var;
        unsigned op;
        double  lit;
};

struct wf_data {
        int type = 0;
        wf_value val;
};

struct node {
        node *left   = nullptr;
        node *right  = nullptr;
        node *parent = nullptr;

        wf_data data;
};

/*
 * Jumps from node to node recursively. 
 * Then applies 'action' to the current node.
 *
 * Note! It calls action() before next jump.
 */
void visit_tree(node *root, void (*action)(node *nd));

void free_tree(node *root);
node *create_node(node *parent = nullptr);
node *copy_tree(node *n);

#define TREE_DEBUG

#ifdef TREE_DEBUG
void dump_tree(node *root);
#else /* TREE_DEBUG */
static inline void dump_tree(node *root) {}
#endif /* TREE_DEBUG */


#endif /* TREE_H */

