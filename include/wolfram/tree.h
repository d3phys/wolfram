#ifndef TREE_H
#define TREE_H

struct node {
        node *left   = nullptr;
        node *right  = nullptr;

        int type = 0;

        union {
                char     var;
                unsigned op;
                double   num;
        } data;
};

double   *node_num(node *n);
unsigned *node_op (node *n);
char     *node_var(node *n);


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

int tex_tree(const char *fname, node *tree);


#endif /* TREE_H */

