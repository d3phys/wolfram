#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <logs.h>
#include <stack.h>

#include <wolfram/wolfram.h>
#include <wolfram/tree.h>
#include <wolfram/operators.h>

static double EPSILON = 1e-100;

static node *create_num(double value);
static node *create_var(char name);
static node *create_op(unsigned opcode, node *left, node *right);
static node *diff_op(node *n);
node *calculate_consts(node *n);
node *diff_node(FILE *f, node *n);

static int equal(double left, double right);

static int node_is_num(node *n);
static int node_is_op(node *n, unsigned opcode);

static node *diff_op(node *n) 
{
        assert(n);

        #define L op->left
        #define R op->right

        #define C(t) copy_tree(t)
        #define D(rc) create_op(OP_DRV, var, rc)

        #define OP(op, lc, rc) create_op(op, lc, rc)
        #define NUM(val)       create_num(val)

        #define ADD(lc, rc) create_op(OP_ADD, lc, rc)
        #define SUB(lc, rc) create_op(OP_SUB, lc, rc)
        #define MUL(lc, rc) create_op(OP_MUL, lc, rc)
        #define DIV(lc, rc) create_op(OP_DIV, lc, rc)

        node *op  = n->right;
        node *var = n->left;

        free(n);

        if (op->type == NODE_NUM) {
                return create_num(0);
        }

        if (op->type == NODE_VAR) {
                if (*node_var(var) == *node_var(op))
                        return create_num(1);
                else
                        return create_num(0);
        }

        switch (*node_op(op)) {
        case OP_ADD:
                return ADD(D(L), D(R));
        case OP_SUB:
                return SUB(D(L), D(R));
        case OP_MUL:
                return ADD(MUL(D(L), C(R)), MUL(C(L), D(R)));
        case OP_DIV:
                return DIV(SUB(MUL(D(L), C(R)), MUL(D(R), C(L))), OP(OP_POW, C(R), NUM(2)));
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

node *diff_node(FILE *f, node *n, node *root)
{
        assert(n);

        if (n->left) {
                if (node_is_op(n->left, OP_DRV)) {
                        $(dump_tree(n->left);)
                        tex_tree_start(f);
                        $(tex_tree(f, n->left);)
                        $(n->left = diff_op(n->left);)
                        fprintf(f, "=");
                        $(dump_tree(n->left);)
                        $(tex_tree(f, n->left);)
                        tex_tree_end(f);
                }

                diff_node(f, n->left, root);
        }

        if (n->right) {
                if (node_is_op(n->right, OP_DRV)) {
                        $(dump_tree(n->right);)
                        tex_tree_start(f);
                        $(tex_tree(f, n->right);)
                        $(n->right = diff_op(n->right);)
                        fprintf(f, "=");
                        $(dump_tree(n->left);)
                        $(dump_tree(n->right);)
                        $(tex_tree(f, n->right);)
                        tex_tree_end(f);
                }

                diff_node(f, n->right, root);
        }


        return n;
}

node *diff_tree(FILE *f, node *n)
{
        assert(n);

        node *init = create_op(OP_DRV, nullptr, n);
        node *var  = create_var('x');
        init->left = var;
        $(dump_tree(init));
        tex_msg(f, "Возьмем простенькую производную\n");

        tex_tree_start(f);
        $(tex_tree(f, init);)
        tex_tree_end(f);

        init = diff_op(init);

        tex_tree_start(f);
        $(tex_tree(f, init);)
        tex_tree_end(f);

        $(dump_tree(init));

        init = diff_node(f, init, init);

        tex_tree_start(f);
        $(tex_tree(f, init);)
        tex_tree_end(f);

        $(dump_tree(init));
        free(var);
        return init; 
}

node *optimize_tree(node *n)
{
        assert(n);

        size_t before = calc_tree_size(n);
        size_t after  = 0;

        while (before != after) {
                before = after;

                n = cut_nodes(n);
                n = calculate_consts(n);

                after = calc_tree_size(n);
        }

        return n;
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


static node *cut_zero_nodes(node *n)
{
        assert(n);

        if (n->type != NODE_OP)
                return n;

        node *newbie = n;
        if (node_is_zero(n->left)) {
                switch (*node_op(n)) {
                case OP_MUL:
                        newbie = n->left;
                        n->left = nullptr;
                        free_tree(n);
                        break;
                case OP_ADD:
                        newbie = n->right;
                        n->right = nullptr;
                        free_tree(n);
                        break;
                case OP_SUB:
                        newbie = n;
                        *node_num(n->left) = -1;
                        *node_op(n) = OP_MUL;
                        break;
                case OP_DIV:
                        newbie = n->left;
                        n->left = nullptr;
                        free_tree(n);
                        break;
                default:
                        break;
                }

                return newbie;
        }

        if (node_is_zero(n->right)) {
                switch (*node_op(n)) {
                case OP_MUL:
                        newbie = n->right;
                        n->right = nullptr;
                        free_tree(n);
                        break;
                case OP_ADD:
                        newbie = n->left;
                        n->left = nullptr;
                        free_tree(n);
                        break;
                default:
                        break;
                }

                return newbie;
        }

        return newbie;
}

static node *cut_same_nodes(node *n)
{
        assert(n);

        if (n->type != NODE_OP)
                return n;
        if (!n->left || !n->right)
                return n;
        if (compare_trees(n->left, n->right)) 
                return n;

        node *newbie = n;

        switch (*node_op(n)) {
        case OP_MUL:
                newbie = create_op(OP_POW, n->left, create_num(2));
                n->left = nullptr;
                free_tree(n);
                break;
        case OP_ADD:
                newbie = create_op(OP_MUL, n->left, create_num(2));
                n->left = nullptr;
                free_tree(n);
                break;
        case OP_SUB:
                newbie = create_num(0);
                free_tree(n);
                break;
        case OP_DIV:
                newbie = create_num(1);
                free_tree(n);
                break;
        default:
                break;
        }

        return newbie;
}

static node *cut_one_nodes(node *n)
{
        assert(n);

        if (n->type != NODE_OP)
                return n;
        if (!n->left || !n->right)
                return n;
        $(dump_tree(n);)

        node *newbie = n;
        if (node_is_one(n->left)) {
                switch (*node_op(n)) {
                case OP_MUL:
                        newbie = n->right;
                        n->right = nullptr;
                        free_tree(n);
                        break;
                default:
                        break;
                }

                return newbie;
        }

        if (node_is_one(n->right)) {
                switch (*node_op(n)) {
                case OP_MUL:
                case OP_DIV:
                        newbie = n->left;
                        n->left = nullptr;
                        free_tree(n);
                        break;
                default:
                        break;
                }

                return newbie;
        }
        return newbie;
}

node *calculate_consts(node *n)
{
        assert(n);
        

        if (n->type != NODE_OP)
                return n;

        if (n->left)
                n->left  = calculate_consts(n->left);
        if (n->right)
                n->right = calculate_consts(n->right);

        if (!n->right)
                return n;
        if (!node_is_num(n->right))
                return n;

        node *newbie = n;

#define L n->left
#define R n->right
#define N(n) *node_num(n)

        switch (*node_op(n)) {
        case OP_ADD:
                if (node_is_num(R) && node_is_num(L)) {
                        newbie = R;
                        N(R) = N(L) + N(R);
                        R = nullptr;
                        free_tree(n);
                }
                break;
        case OP_SUB:
                if (node_is_num(R) && node_is_num(L)) {
                        newbie = R;
                        N(R) = N(L) - N(R);
                        R = nullptr;
                        free_tree(n);
                }
                break;
        case OP_MUL:
                if (node_is_num(R) && node_is_num(L)) {
                        newbie = R;
                        N(R) = N(L) * N(R);
                        R = nullptr;
                        free_tree(n);
                }
                break;
        case OP_DIV:
                if (node_is_num(R) && node_is_num(L)) {
                        newbie = R;
                        N(R) = N(L) / N(R);
                        R = nullptr;
                        free_tree(n);
                }
                break;
        case OP_SIN:
                if (node_is_num(R)) {
                        if (node_is_zero(R)) {
                                newbie = R;
                                N(R) = 0;
                                R = nullptr;
                                free_tree(n);
                        }
                }
                break;
        case OP_LN:
                if (node_is_num(R)) {
                        if (node_is_one(R)) {
                                newbie = R;
                                N(R) = 0;
                                R = nullptr;
                                free_tree(n);
                        }
                }
                break;
        default:
                break;
        }

        return newbie;
#undef L
#undef R
#undef N

}

node *cut_nodes(node *n)
{
        assert(n);

        $(n = cut_one_nodes (n);)
        $(n = cut_zero_nodes(n);)
        $(n = cut_same_nodes(n);)

        if (n->left)
                n->left  = cut_nodes(n->left);
        if (n->right)
                n->right = cut_nodes(n->right);

        return n;
}

static int equal(double left, double right)
{
        return fabs(left - right) < EPSILON;
}


