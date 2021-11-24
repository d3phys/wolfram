#ifndef WOLFRAM_H
#define WOLFRAM_H

#include <stdlib.h>
#include <wolfram/tree.h>

const int SEED = 0xDED64;

enum wf_type {
        WF_VARIABLE = 0x01,
        WF_OPERATOR = 0x02,
        WF_LITERAL  = 0x03,
};


node *diff_tree(node *n);


#endif /* WOLFRAM_H */
