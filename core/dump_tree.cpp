#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <logs.h>

#include <wolfram/tree.h>
#include <wolfram/operators.h>
#include <wolfram/wolfram.h>

/*
 * Default Graphviz settings for nodes, edges and graphs.
 *
 * Note! If you want to change the vast majority of nodes - 
 * you have to change it instead of modification print_node() function!
 *
 * And yeah... It is hardcoded right now...
 */
static const char HEADER[] = R"(
digraph {
        edge [color     = darkgrey]
        edge [arrowhead = onormal]
        edge [arrowsize = 1.6] 
        edge [penwidth  = 1.2]

        graph [penwidth = 2] 
        graph [ranksep  = 1.3] 
        graph [nodesep  = 0.5]
        graph [style    = "filled"]
        graph [color    = green, fillcolor = lightgreen]

        node [penwidth = 2] 
        node [shape    = box] 
        node [style    = "rounded, filled"] 
        node [color    = grey, fillcolor = white]
        node [fontname = "Courier"]

        compound  = true;
        newrank   = true;

        dpi      = 57
        fontname = "Courier"
)";

static const char DUMP_FILE_PATH[] = "log/dump";

struct gviz_options {
        const char *color     = nullptr;
        const char *style     = nullptr;
        const char *shape     = nullptr;
        const char *fontname  = nullptr;
        const char *fillcolor = nullptr;
        const char *fontsize  = nullptr;
};

const gviz_options NODE_LITERAL  = {
        .color = "darkslategrey",
        .shape = "ellipse",
        .fillcolor = "darkslategray3",
};

const gviz_options NODE_VARIABLE = {
        .color = "indigo",
        .shape = "diamond",
        .fillcolor = "thistle",
};

const gviz_options NODE_OPERATOR = {
        .color = "grey",
        .shape = "rectangle",
        .fillcolor = "azure2",
};

const gviz_options EDGE_INVIS = {
        .style = "dotted"
};

const gviz_options EDGE_DEFAULT = {
        .style = "dashed"
};

struct gviz_node {
        node *cur = nullptr;
        const gviz_options *opt = nullptr;
};

struct gviz_edge {
        node *from = nullptr;
        node *to   = nullptr;
        const gviz_options *opt = nullptr;
};

static void gvprint_options(const gviz_options *opt);

static inline void gvprint_option(const char *opt, const char *name);
static inline void gvprint_content(const gviz_node *node);

static void gvprint_operator(const unsigned type);

static void gvprint_node(gviz_node *node);
static void gvprint_edge(gviz_edge *edge);

static void print_node(node *cur);

static FILE *GVIZ_FILE = nullptr;
#define gvprint(fmt, ...) fprintf(GVIZ_FILE, fmt, ##__VA_ARGS__);

/*
 * Inserts tree dump .svg image to the log file. 
 * Each time you use it, it creates new .svg dump image 
 * in DUMP_FILE_PATH location.
 *
 * It uses Graphviz (dot). 
 * Note! Node memory address is used as node identifier. 
 *
 * This function is designed to be called multiple times.
 *
 * Note 2! It it really slow...
 */
void dump_tree(node *root)
{
        assert(root);
        static unsigned dump_num = 0;

        /*
         * Temporary buffer for string concatenation. 
         * We should use it to reduce memory allocation. 
         * The BUF_SIZE is unreasonable... I'll think about it later...
         */
        static const size_t BUF_SIZE = 512;
        static char buf[BUF_SIZE]    = {0};

        snprintf(buf, sizeof(buf), "%s%u.dot", DUMP_FILE_PATH, dump_num);

        GVIZ_FILE = fopen(buf, "w");
        if (!GVIZ_FILE) {
                perror("Can't create dump file");
                return;
        }

        gvprint(HEADER);

        /*
         * Recursively jumps from node to node and prints it.
         */
        visit_tree(root, print_node);

        gvprint("\n}");

        fclose(GVIZ_FILE);

        /*
         * That is Graphviz dot svg creation command. 
         * For more information check:
         *  - https://graphviz.org
         */
        snprintf(buf, sizeof(buf), "dot -Tsvg %s%u.dot -o %s%u.svg", 
                 DUMP_FILE_PATH, dump_num, DUMP_FILE_PATH, dump_num);
        system(buf);

        /*
         * We use html tag <img/>.
         * It's probably the best way to place graphical dump to the log file. 
         */
        fprintf(logs, "\n<img src=\"%s%u.svg\"/>\n", DUMP_FILE_PATH, dump_num);

        dump_num++;
}

static void print_node(node *cur)
{
        if (cur == nullptr)
                return;

        assert(cur);

        static gviz_node n       = {0};
        static gviz_edge e       = {0};

        static const gviz_options *opt = {0};

        n.cur = cur;

        switch (cur->type) {
        case NODE_VAR:
                opt = &NODE_VARIABLE;
                break;
        case NODE_OP:
                opt = &NODE_OPERATOR;
                break;
        case NODE_NUM:
                opt = &NODE_LITERAL;
                break;
        default:
                //assert(0);
                return;
                break;
        }

        n.opt = opt;

        gvprint_node(&n);

        e.opt = &EDGE_DEFAULT;
        e.from = cur;

        e.to   = cur->left;
        gvprint_edge(&e);

        e.to   = cur->right;
        gvprint_edge(&e);

        /*
#ifdef TREE_DEBUG
        e.opt = &EDGE_INVIS;

        if (cur->left) {
                e.from = cur->left;
                e.to   = cur->left->parent;
                gvprint_edge(&e);
        }

        if (cur->right) {
                e.from = cur->right;
                e.to   = cur->right->parent;
                gvprint_edge(&e);
        }

        e.to   = cur->right->parent;
        e.from = cur->right;
        gvprint_edge(&e);
#endif
        */
}

static inline void gvprint_option(const char *opt, const char *name)
{
        if (opt)
                gvprint("%s = \"%s\", ", name, opt);
}


static void gvprint_options(const gviz_options *opt)
{
        if (!opt)
                return;

        gvprint("[");

        gvprint_option(opt->color, "color");
        gvprint_option(opt->style, "style");
        gvprint_option(opt->shape, "shape");
        gvprint_option(opt->fontname, "fontname");
        gvprint_option(opt->fillcolor, "fillcolor");
        gvprint_option(opt->fontsize, "fontsize");

        gvprint("]");
}

static void gvprint_node(gviz_node *node)
{
        assert(node);

        gvprint("node%p", node->cur);
        gvprint_options(node->opt);
        gvprint_content(node);
        gvprint("\n");
}

static inline void gvprint_content(const gviz_node *node)
{
        assert(node);

        gvprint("[label=\"");

        switch (node->cur->type) {
        case NODE_VAR:
                gvprint("%c", *node_var(node->cur));
                break;
        case NODE_OP:
                gvprint_operator(*node_op(node->cur));
                break;
        case NODE_NUM:
                gvprint("%lg", *node_num(node->cur));
                break;
        default:
                assert(0);
                break;
        }

        gvprint("\"]");
}

static void gvprint_operator(const unsigned type)
{
        const char *op = optostr(type);
        if (op) {
                gvprint("%s\n", op);
        }

        gvprint("0x%x", type);
}

static void gvprint_edge(gviz_edge *edge)
{
        assert(edge);

        if (!edge->from || !edge->to)
                return;

        gvprint("node%p ", edge->from);
        gvprint("-> node%p", edge->to);
        gvprint_options(edge->opt);
        gvprint("\n");
}


#undef gvprint


