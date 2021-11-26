#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <logs.h>

#include <wolfram/tree.h>
#include <wolfram/operators.h>
#include <wolfram/wolfram.h>


static const char HEADER[] = R"(
        \documentclass{article}
        \usepackage[utf8]{inputenc}
        \usepackage[T2A]{fontenc}
        \begin{document}
)";

static const char FOOTER[] = R"(
        \end{document}
)";


static const char EQ_HEADER[] = R"(
        \begin{equation}
)";

static const char EQ_FOOTER[] = R"(
        \end{equation}
)";

static int isop(unsigned opcode, node *n);
static void tex_pow(FILE *f, node *left, node *right);
static void tex_data(FILE *const f, node *n);
static void tex_node(FILE *const f, node *n);
static void tex_func(FILE *f, node *func, node *arg);
static void tex_frac(FILE *f, node *left, node *right);
static void tex_cdot(FILE *f, node *left, node *right);

void tex_msg(FILE *f, const char *msg)
{
        assert(f);
        assert(msg);

        fprintf(f, "%s", msg);
}

FILE *open_tex(const char *fname)
{
        assert(fname);

        FILE *f = fopen(fname, "w");
        if (!f) {
                perror("Can't open file");
                return nullptr;
        }

        fprintf(f, HEADER);
        return f;
}

void close_tex(FILE *f)
{
        assert(f);

        fprintf(f, FOOTER);
        fclose(f);
}

void compile_tex(const char *fname)
{
        assert(fname);

        static const size_t CMD_SIZE = 100;
        char cmd[CMD_SIZE] = {0}; 

        sprintf(cmd, "pdflatex -output-directory tex %s ", fname);
        system(cmd);
}

int tex_tree(FILE *f, node *tree)
{
        assert(tree);
        assert(f);

        fprintf(f, EQ_HEADER);
        tex_node(f, tree);
        fprintf(f, EQ_FOOTER);

        return 0;
}

static int isop(unsigned opcode, node *n)
{
        assert(n);
        if (n->type != NODE_OP)
                return 0;

        return *node_op(n) == opcode;
}

static void tex_pow(FILE *f, node *left, node *right)
{
        assert(f);
        assert(left);
        assert(right);

        if (left->type == NODE_OP) {
                fprintf(f, "(");
                tex_node(f, left);
                fprintf(f, ")");
        } else {
                tex_node(f, left);
        }

        fprintf(f, "^{");
        tex_node(f, right);
        fprintf(f, "}");
}

static void tex_func(FILE *f, node *func, node *arg)
{
        assert(f);
        assert(arg);

        if (arg->type == NODE_OP) {
                fprintf(f, "\\%s{(", optostr(*node_op(func)));
                tex_node(f, arg);
                fprintf(f, ")}");
        } else {
                fprintf(f, "\\%s{", optostr(*node_op(func)));
                tex_node(f, arg);
                fprintf(f, "}");
        }
}

static void tex_frac(FILE *f, node *left, node *right)
{
        assert(f);
        assert(left);
        assert(right);

        fprintf(f, "\\frac{");
        tex_node(f, left);
        fprintf(f, "}{");
        tex_node(f, right);
        fprintf(f, "}");
}

static void tex_cdot(FILE *f, node *left, node *right)
{
        assert(f);
        assert(right);

        if (isop(OP_ADD, left) || isop(OP_SUB, left)) {
                fprintf(f, "(");
                tex_node(f, left);
                fprintf(f, ")"); 
        } else {
                tex_node(f, left);
        }

        fprintf(f, "\\cdot ");

        if (isop(OP_ADD, right) || isop(OP_SUB, right)) {
                fprintf(f, "(");
                tex_node(f, right);
                fprintf(f, ")"); 
        } else {
                tex_node(f, right);
        }
}

static void tex_node(FILE *const f, node *n)
{
        assert(f);
        assert(n);

        if (n->type != NODE_OP) {
                tex_data(f, n);
                return;
        }

        switch (*node_op(n)) {
        case OP_ADD:
        case OP_SUB:
                tex_node(f, n->left);
                fprintf(f, "%s", optostr(*node_op(n)));
                tex_node(f, n->right);
                break;
        case OP_MUL:
                tex_cdot(f, n->left, n->right);
                break;
        case OP_DIV:
                tex_frac(f, n->left, n->right);
                break;
        case OP_POW:
                tex_pow(f, n->left, n->right);
                break;
        default:
                tex_func(f, n, n->right);
                break;
        }
}

static void tex_data(FILE *const f, node *n)
{
        assert(f);

        switch (n->type) {
        case NODE_VAR:
                fprintf(f, "%c",  *node_var(n));
                break;
        case NODE_NUM:
                fprintf(f, "%lg", *node_num(n));
                break;
        default:
                assert(0);
                break;
        }
}


