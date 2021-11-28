#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <logs.h>
#include <wolfram/wolfram.h>
#include <wolfram/tree.h>
#include <wolfram/operators.h>

static double EPSILON = 1e-100;

static node *create_num(double value);
static node *create_var(char name);
static node *create_op(unsigned opcode, node *left, node *right);
static node *diff_op(node *n);
node *calculate_consts(node *n);
static int node_is_op(node *n, unsigned opcode);
node *diff_node(FILE *f, node *n);

static int node_is_num(node *n);

static int equal(double left, double right)
{
        return fabs(left - right) <  EPSILON;
}

static node *create_num(double value) 
{
        node *newbie = create_node();
        if (!newbie)
                return nullptr;

        newbie->type      = NODE_NUM;
        *node_num(newbie) = value;

        return newbie;
}

static node *create_var(char name) 
{
        node *newbie = create_node();
        if (!newbie)
                return nullptr;

        newbie->type      = NODE_VAR;
        *node_var(newbie) = name;

        return newbie;
}

static node *create_op(unsigned opcode, node *left, node *right) 
{
        if (!right) 
                return nullptr;

        node *newbie = create_node();
        if (!newbie)
                return nullptr;

        newbie->type    = NODE_OP;
        *node_op(newbie) = opcode;

        if (left)
                newbie->left  = left;

        if (right)
                newbie->right = right;

        return newbie;
}

static node *diff_op(node *n) 
{
        assert(n);

        #define L op->left
        #define R op->right

        #define C(t) copy_tree(t)
        #define D(rc) create_op(OP_DRV, nullptr, rc)

        #define OP(op, lc, rc) create_op(op, lc, rc)
        #define NUM(val)       create_num(val)

        #define ADD(lc, rc) create_op(OP_ADD, lc, rc)
        #define SUB(lc, rc) create_op(OP_SUB, lc, rc)
        #define MUL(lc, rc) create_op(OP_MUL, lc, rc)
        #define DIV(lc, rc) create_op(OP_DIV, lc, rc)


        node *op = n->right;
        free(n);

        if (op->type == NODE_NUM)
                return create_num(0);

        if (op->type == NODE_VAR)
                return create_num(1);

        switch (*node_op(op)) {
        case OP_ADD:
                return ADD(D(L), D(R));
        case OP_SUB:
                return SUB(D(L), D(R));
        case OP_MUL:
                return ADD(MUL(D(L), C(R)), MUL(C(L), D(R)));
        case OP_DIV:
                return DIV(SUB(MUL(D(L), C(R)), MUL(C(L), D(R))), OP(OP_POW, C(R), NUM(2)));
        case OP_POW:
                return MUL(OP(OP_POW, C(L), C(R)), ADD(MUL(D(R), OP(OP_LN, nullptr, C(L))), DIV(MUL(C(R), D(L)), C(L))));
        case OP_SIN:
                return MUL(OP(OP_COS, nullptr, C(R)), D(R));
        case OP_COS:
                return MUL(MUL(NUM(-1), OP(OP_SIN, nullptr, C(R))), D(R));
        case OP_SH:
                return MUL(OP(OP_CH, nullptr, C(R)), D(R));
        case OP_CH:
                return MUL(OP(OP_SH, nullptr, C(R)), D(R));
        case OP_LN:
                return DIV(D(R), C(R));
        default:
                break;
        }

        return nullptr;

        #undef R
        #undef L
        #undef C
        #undef D
        #undef OP 
        #undef ADD
        #undef SUB
        #undef MUL 
        #undef DIV 
}

node *diff_node(FILE *f, node *n)
{
        assert(n);

        if (node_is_op(n, OP_DRV)) {
                $(dump_tree(n);)
                $(tex_tree(f, n);)
                n = diff_op(n);
                fprintf(logs, "=======\n");
                $(dump_tree(n);)
                $(tex_tree(f, n);)
        }

        if (n->left)
                n->left  = diff_node(f, n->left);
        if (n->right)
                n->right = diff_node(f, n->right);

        return n;
}

node *diff_tree(FILE *f, node *n)
{
        assert(n);

        node *init = create_op(OP_DRV, nullptr, n);
        $(dump_tree(init));
        init = diff_node(f, init);
        $(dump_tree(init));
        return init; 
}

node *optimize_tree(node *n)
{
        assert(n);
        n = cut_nodes(n);
        n = calculate_consts(n);
        return n;
}

static int node_is_zero(node *n)
{
        if (!n)
                return 0;

        if (n->type == NODE_NUM)
                if (equal(*node_num(n), 0))
                        return 1;
        return 0;
}

static int node_is_one(node *n)
{
        if (!n)
                return 0;

        if (n->type == NODE_NUM)
                if (equal(*node_num(n), 1))
                        return 1;
        return 0;
}

static int node_is_op(node *n, unsigned opcode)
{
        assert(n);

        if (n->type == NODE_OP)
                if (*node_op(n) == opcode)
                        return 1;
        return 0;
}

static int node_is_num(node *n)
{
        assert(n);

        if (n->type == NODE_NUM)
                return 1;

        return 0;
}

static node *cut_zero(node *n)
{
        assert(n);
        if (node_is_op(n, OP_MUL)) {
                if (node_is_zero(n->left)) {
                        node *ret = n->left;
                        n->left   = nullptr;
                        free_tree(n);
                        return ret;
                }

                if (node_is_zero(n->right)) {
                        node *ret = n->right;
                        n->right  = nullptr;
                        free_tree(n);
                        return ret;
                }
        }

        if (node_is_op(n, OP_ADD)) {
                if (node_is_zero(n->left)) {
                        node *ret = n->right;
                        n->right  = nullptr;
                        free_tree(n);
                        return ret;
                }

                if (node_is_zero(n->right)) {
                        node *ret = n->left;
                        n->left   = nullptr;
                        free_tree(n);
                        return ret;
                }
        }

        if (node_is_op(n, OP_SUB)) {
                if (node_is_zero(n->left)) {
                        *node_num(n->left) = -1;
                        *node_op(n) = OP_MUL;
                        return n;
                }

                if (node_is_zero(n->right)) {
                        node *ret = n->left;
                        n->left   = nullptr;
                        free_tree(n);
                        return ret;
                }
        }

        if (node_is_op(n, OP_DIV)) {
                if (node_is_zero(n->left)) {
                        node *ret = n->left;
                        n->left   = nullptr;
                        free_tree(n);
                        return ret;
                }
        }

        return n;
}

static node *cut_one(node *n)
{
        assert(n);
        if (node_is_op(n, OP_MUL)) {
                if (node_is_one(n->left)) {
                        node *ret = n->right;
                        n->right = nullptr;
                        free_tree(n);
                        return ret;
                }

                if (node_is_one(n->right)) {
                        node *ret = n->left;
                        n->left = nullptr;
                        free_tree(n);
                        return ret;
                }
        }

        return n;
}


node *calculate_consts(node *n)
{
        assert(n);

        if (n->type != NODE_OP)
                return n;

        if (n->left)
                n->left = calculate_consts(n->left);

        if (n->right)
                n->right = calculate_consts(n->right);

        switch (*node_op(n)) {
        case OP_ADD:
                if (node_is_num(n->right) && node_is_num(n->left)) {
                        node *ret = n->right;
                        *node_num(n->right) = *node_num(n->left) +
                                              *node_num(n->right);
                        n->right  = nullptr;
                        free_tree(n);
                        return ret;
                }
                break;
        case OP_SUB:
                if (node_is_num(n->right) && node_is_num(n->left)) {
                        node *ret = n->right;
                        *node_num(n->right) = *node_num(n->left) -
                                              *node_num(n->right);
                        n->right  = nullptr;
                        free_tree(n);
                        return ret;
                }
                break;
        case OP_MUL:
                if (node_is_num(n->right) && node_is_num(n->left)) {
                        node *ret = n->right;
                        *node_num(n->right) = *node_num(n->left) *
                                              *node_num(n->right);
                        n->right  = nullptr;
                        free_tree(n);
                        return ret;
                }
                break;
        case OP_DIV:
                if (node_is_num(n->right) && node_is_num(n->left)) {
                        node *ret = n->right;
                        *node_num(n->right) = *node_num(n->left) /
                                              *node_num(n->right);
                        n->right  = nullptr;
                        free_tree(n);
                        return ret;
                }
                break;
        case OP_SIN:
                if (node_is_num(n->right)) {
                        if (equal(*node_num(n->right), 0)) {
                                node *ret = n->right;
                                *node_num(n->right) = 0;
                                n->right  = nullptr;
                                free_tree(n);
                                return ret;
                        }
                        return n;
                }
                break;
        case OP_LN:
                if (node_is_num(n->right)) {
                        if (equal(*node_num(n->right), 1)) {
                                node *ret = n->right;
                                *node_num(n->right) = 0;
                                n->right  = nullptr;
                                free_tree(n);
                                return ret;
                        }
                        return n;
                }
                break;
        default:
                break;
        }

        
        return n;
}

node *cut_nodes(node *n)
{
        assert(n);

        if (n->type != NODE_OP)
                return n;

        n = cut_one(n);
        n = cut_zero(n);

        if (n->left)
                n->left = cut_nodes(n->left);
        if (n->right)
                n->right= cut_nodes(n->right);
        if (n->left)
                n->left = cut_zero(n->left);
        if (n->right)
                n->right = cut_zero(n->right);
        if (n->left)
                n->left = cut_one(n->left);
        if (n->right)
                n->right = cut_one(n->right);

        return n;
}


