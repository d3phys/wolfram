#include <stdio.h>
#include <string.h>
#include <iommap.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <hash.h>
#include <logs.h>

#include <wolfram/parse.h>
#include <wolfram/tree.h>
#include <wolfram/operators.h>
#include <wolfram/wolfram.h>

static node *read_data(char **ptr, node *n);
static char *find_bracket(char *str);
static ptrdiff_t remove_spaces(char *buf);

static node *read_data(char **ptr, node *n)
{
        assert(ptr && *ptr);
        assert(n);

        char *end = *ptr;
        double num = strtod(*ptr, &end);
        if (end != *ptr) {
                n->type = NODE_NUM;
                *node_num(n) = num;

                *ptr = end;
                return n;
        }

        char *endptr = find_bracket(*ptr);
        unsigned hash = murmur_hash(*ptr, (size_t)(endptr - *ptr), SEED);
        if (optostr(hash) != nullptr) {
                n->type = NODE_OP;
                *node_op(n) = hash;

                *ptr = endptr;
                return n;
        }
        
        if (**ptr != '\0') {
                n->type = NODE_VAR;
                *node_var(n) = **ptr;

                (*ptr)++;
                return n;
        }

        return nullptr;
}

static char *find_bracket(char *str)
{
        assert(str);

        while (*str != '\0' && *str != ')' && *str != '(')
                str++;

        return str;
}

static ptrdiff_t remove_spaces(char *buf) 
{
        assert(buf);

        char *r = buf;
        char *w = buf;
        while (*r != '\0') {
                if (!isspace(*r))
                        *w++ = *r;
                r++;
        }

        *w = '\0';
        return w - buf + 1;
}


static node *read_formula(char **ptr)
{
        #define PARSE_DEBUG
        #ifdef PARSE_DEBUG
        #define shw printf("%d: %s\n", __LINE__, *ptr);
        #else
        #define shw ;
        #endif 

        assert(ptr && *ptr);

        if (**ptr != '(')
                return nullptr;
shw
        (*ptr)++;
shw
        node *newbie = create_node();
        if (!newbie)
                return nullptr;
shw

        if (**ptr == '(') {
                newbie->left = read_formula(ptr);

shw
                if (!newbie->left) {
                        free_tree(newbie);
                        return nullptr;
                }
        }
shw
        node *read = read_data(ptr, newbie);
shw
        if (!read) {
                free_tree(newbie);
                return nullptr;
        }

shw
        if (**ptr == '(') {
                if (newbie->type != NODE_OP) {
                        free_tree(newbie);
                        return nullptr;
                }
shw
                newbie->right = read_formula(ptr);
                if (!newbie->right) {
                        free_tree(newbie);
                        return nullptr;
                }
        }

shw
        if (**ptr != ')') {
                free_tree(newbie);
                return nullptr;
        }

shw
        (*ptr)++;
shw
        return newbie;

        #undef shw
}

node *parse_infix(const char *fname)
{
        assert(fname);

        mmap_data data = {0};
        int err = mmap_in(&data, fname); 
        if (err)
                return nullptr;

        remove_spaces(data.buf);

        char *r = data.buf;
        node *rt = read_formula(&r);
        if (*r != '\0' || !rt) {
                fprintf(stderr, "Invalid format\n");
                fprintf(stderr, "Here >>> %s\n", r);
$               (free_tree(rt);)
                rt = nullptr;
        }

        mmap_free(&data);
        return rt;
}


