#include <assert.h>
#include <stdio.h>
#include <wolfram/wolfram.h>
#include <wolfram/tree.h>
#include <wolfram/operators.h>

static node *create_lit(double value);
static node *create_var(char name);
static node *create_op(unsigned opcode, node *left, node *right);
static node *diff_op(node *n);

static node *create_lit(double value) 
{
        node *newbie = create_node();
        if (!newbie)
                return nullptr;

        newbie->type     = WF_LITERAL;
        newbie->data.lit = value;

        return newbie;
}

static node *create_var(char name) 
{
        node *newbie = create_node();
        if (!newbie)
                return nullptr;

        newbie->type     = WF_VARIABLE;
        newbie->data.var = name;

        return newbie;
}

static node *create_op(unsigned opcode, node *left, node *right) 
{
        if (!right) 
                return nullptr;

        node *newbie = create_node();
        if (!newbie)
                return nullptr;

        newbie->type    = WF_OPERATOR;
        newbie->data.op = opcode;

        if (left) {
                newbie->left  = left;
                left->parent  = newbie;
        }

        if (right) {
                newbie->right = right;
                right->parent = newbie;
        }

        return newbie;
}

static node *diff_op(node *n) 
{
        assert(n);

        #define L n->left
        #define R n->right

        #define C(t) copy_tree(t)
        #define D(t) diff_tree(t)

        #define OP(op, lc, rc) create_op(op, lc, rc)
        #define LIT(val)       create_lit(val)

        #define ADD(lc, rc) create_op(OP_ADD, lc, rc)
        #define SUB(lc, rc) create_op(OP_SUB, lc, rc)
        #define MUL(lc, rc) create_op(OP_MUL, lc, rc)
        #define DIV(lc, rc) create_op(OP_DIV, lc, rc)

        switch (n->data.op) {
        case OP_ADD:
                return ADD(D(L), D(R));
        case OP_SUB:
                return SUB(D(L), D(R));
        case OP_MUL:
                return ADD(MUL(D(L), C(R)), MUL(C(L), D(R)));
        case OP_DIV:
                return DIV(SUB(MUL(D(L), C(R)), MUL(C(L), D(R))), OP(OP_POW, C(R), LIT(2)));
        case OP_POW:
                return MUL(OP(OP_POW, C(L), C(R)), ADD(MUL(D(R), OP(OP_LN, nullptr, C(L))), DIV(MUL(C(R), D(L)), C(L))));
        case OP_SIN:
                return MUL(OP(OP_COS, nullptr, C(R)), D(R));
        case OP_COS:
                return MUL(MUL(LIT(-1), OP(OP_SIN, nullptr, C(R))), D(R));
        case OP_SH:
                return MUL(OP(OP_CH, nullptr, C(R)), D(R));
        case OP_CH:
                return MUL(OP(OP_SH, nullptr, C(R)), D(R));
        case OP_LN:
                return DIV(D(R), C(R));
        default:
                return nullptr;
        }

#undef L
#undef R
#undef C
#undef D
#undef OP 
#undef ADD
#undef SUB
#undef MUL 
#undef DIV 

}

node *diff_tree(node *n)
{
        assert(n);

        switch (n->type) {
        case WF_LITERAL:
                printf("lit: %lg\n", n->data.lit);
                return create_lit(0);
        case WF_VARIABLE:
                printf("var: %c\n",  n->data.var);
                return create_lit(1);
        case WF_OPERATOR:
                printf("hash: %x\n", n->data.op);
                return diff_op(n);
        default:
                fprintf(stderr, "DIFF TREE FAILED whatufuck? :|\n");
                return nullptr;
        }
}

node *optimize_tree(node *n)
{
        assert(n);
}

/*
node *cut_nodes(node *n)
{
        assert(n);

        if (n->type != WF_OPERATOR)
                return nullptr;

        if (n->left) 
                cut_nodes(n->left);

        if (n->right) 
                cut_nodes(n->right);

        switch (n->type) {
        case OP_MUL:
                if (n->left->data.type == WF_LITERAL && n->left->data.val.lit == 1) {
                        free_tree(n->left);
                        abandon_child(n, n->right);
                } else if (n->right->data.type == WF_LITERAL && n->right->data.val.lit == 1) {
                        free_tree(n->right);
                        abandon_child(n, n->left);
                }
                break;

        default:
                break;
        }

        return nullptr;
}
*/


