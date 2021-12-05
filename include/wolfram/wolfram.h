#ifndef WOLFRAM_H
#define WOLFRAM_H

#include <stdlib.h>
#include <stack.h>
#include <wolfram/tree.h>

const int SEED = 0xDED64;

enum node_type {
        NODE_VAR = 0x01,
        NODE_OP  = 0x02,
        NODE_NUM = 0x03,
};


node *diff_tree(FILE *f, node *n);
node *cut_nodes(node *n);
node *optimize_tree(node *n);

void tex_tree_start(FILE *f);
void tex_tree_end(FILE *f);
FILE *open_tex(const char *fname);
void close_tex(FILE *f);
void tex_msg(FILE *f, const char *msg);
void compile_tex(const char *fname);
node *replace_nodes(node *n, stack *const stk);


#endif /* WOLFRAM_H */
