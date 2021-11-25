#ifndef WOLFRAM_H
#define WOLFRAM_H

#include <stdlib.h>
#include <wolfram/tree.h>

const int SEED = 0xDED64;

enum node_type {
        NODE_VAR = 0x01,
        NODE_OP  = 0x02,
        NODE_NUM = 0x03,
};


node *diff_tree(node *n);
node *cut_nodes(node *n);


#endif /* WOLFRAM_H */
