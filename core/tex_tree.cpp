#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <logs.h>

#include <wolfram/tree.h>
#include <wolfram/operators.h>
#include <wolfram/wolfram.h>
#include <stack.h>


// \left \right
// indentfirst
// одиночные аргументы
// умножение на -1
// cos(x^2)
// (C)
// область определения
// проверить на нолики
// Замена всего выражения (БАГ)
// Свертка констант 0/0 ln(-7)
// 0^x 
static const char *const PHRASES[] = {
        "Методом тотальной аналогии получим",
        "Из элементарных свойств конических сечений получим",
        "А ведь не трудно вспомнить",
        "Легко увидеть принцип Дирихле",
        "Следующий результат был получен еще в 2032 году",
        "Внимательный читатель помнит",
        "После сокращения единиц, получаем",
        "Легко заметить, что",
        "Обобщим предыдущее выражение",
        "Вспомним числа Каталана",
        "Из диаграммы Эйлера",
        "Мы еще будем пользоваться полученным ниже результатом",
        "Из геометрических соображений получим",
        "Когда-то существовал анекдот, напоминающий следующую формулу",
        "Интересно, что приведенная ниже формула очень напоминает заметки в книге \"Анналы Драконьих жрецов\".",
        "Используя метод электростатических изображений",
};

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
static void tex_pow (FILE *f, node *left, node *right);
static void tex_data(FILE *const f, node *n);
static void tex_node(FILE *const f, node *n);
static void tex_func(FILE *f, node *func, node *arg);
static void tex_frac(FILE *f, node *left, node *right);
static void tex_cdot(FILE *f, node *left, node *right);
static void tex_drv (FILE *f, node *var, node *expr);

static stack *VARS_STACK = nullptr;

__attribute__((constructor))
static void init_vars_stack()
{ 
        VARS_STACK = (stack *)calloc(1, sizeof(stack));
        if (!VARS_STACK) {
                fprintf(stderr, "Can't create VARS_STACK\n");
                exit(EXIT_FAILURE);
        }

        construct_stack(VARS_STACK);
        dump_stack(VARS_STACK);
}

__attribute__((destructor))
static void kill_vars_stack()
{
        destruct_stack(VARS_STACK);
        free(VARS_STACK);
}

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


static size_t change_of_vars(node *n)
{
        assert(n);

        static char name = 'A';

        size_t size = 1;
        if (n->left)
                size += change_of_vars(n->left);
        if (n->right)
                size += change_of_vars(n->right);
        $(dump_tree(n);)

        if (size >= 33) {
                node *newbie = create_node();
                newbie->type = NODE_VAR;
                *node_var(newbie) = name;
                newbie->right = n;
                push_stack(VARS_STACK, newbie);
                name++;
                size = 0;
        }
        
        return size;
}

int tex_big_tree(FILE *f, node *tree) 
{
        assert(tree);
        assert(f);

        change_of_vars(tree);
        
        dump_stack(VARS_STACK);
        node *temp = nullptr;

        tex_tree_start(f);
        $(tex_tree(f, tree);)
        tex_tree_end(f);

        tex_msg(f, "Где\n");
        while (VARS_STACK->size) {
                temp = (node *)pop_stack(VARS_STACK);

                fprintf(f, EQ_HEADER);
                $(tex_node(f, temp);)
                fprintf(f, " = ");
                $(tex_node(f, temp->right);)
                fprintf(f, EQ_FOOTER);

                $(free(temp);)
        }

        return 0;
}

void tex_tree_start(FILE *f)
{
        fprintf(f, "%s\n", PHRASES[rand() % (sizeof(PHRASES) / sizeof(*PHRASES))]);
        fprintf(f, EQ_HEADER);
}

void tex_tree_end(FILE *f)
{
        fprintf(f, EQ_FOOTER);
}

int tex_tree(FILE *f, node *tree)
{
        assert(tree);
        assert(f);

        tex_node(f, tree);

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

static void tex_drv(FILE *f, node *var, node *expr)
{
        assert(f);
        assert(var);
        assert(expr);

        fprintf(f, "\\frac{\\partial}{\\partial %c}(", *node_var(var));
        tex_node(f, expr);
        fprintf(f, ")");
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

static node **find_var(stack *const stk, node *n)
{
        assert(n);

        if (stk->size == 0)
                return nullptr;

        node **found = nullptr;
        for (size_t i = stk->size; i > 0; i--) {
                if (((node *)stk->items[i - 1])->right == n) {
                        found = (node **)&stk->items[i - 1];
                        break;
                }
        }

        return found;
}

static void tex_node(FILE *const f, node *n)
{
        assert(f);
        assert(n);

#define L n->left
#define R n->right
#define N(n) *node_num(n)

        if (VARS_STACK->size) {
                node **var = find_var(VARS_STACK, n);
                if (var) {
                        tex_data(f, *var);
                        return;
                }
        }

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
                tex_cdot(f, L, R);
                break;
        case OP_DIV:
                tex_frac(f, L, R);
                break;
        case OP_POW:
                tex_pow (f, L, R);
                break;
        case OP_DRV:
                tex_drv (f, L, R);
                break;
        default:
                tex_func(f, n, R);
                break;
        }

#undef L
#undef R
#undef N
}

static void tex_data(FILE *const f, node *n)
{
        assert(f);

        switch (n->type) {
        case NODE_VAR:
                fprintf(f, "%c",  *node_var(n));
                break;
        case NODE_NUM:
                if (*node_num(n) < 0)
                        fprintf(f, "(%lg)", *node_num(n));
                else 
                        fprintf(f, "%lg", *node_num(n));
                break;
        default:
                assert(0);
                break;
        }
}


