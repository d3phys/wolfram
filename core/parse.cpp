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

static char *read_data(char *ptr, wf_data *const data);
static char *find_bracket(char *str);
static ptrdiff_t remove_spaces(char *buf);

static char *read_data(char *ptr, wf_data *const data)
{
        assert(ptr);
        assert(data);

        char *end = ptr;
        double lit = strtod(ptr, &end);
        if (end != ptr) {
                data->type = WF_LITERAL;
                data->val.lit = lit;
                return end;
        }

        char *endptr = find_bracket(ptr);
        unsigned hash = murmur_hash(ptr, (size_t)(endptr - ptr), SEED);
        if (optostr(hash) != nullptr) {
                data->type = WF_OPERATOR;
                data->val.op = hash;
                return endptr;
        }
        
        if (*ptr != '\0') {
                data->type = WF_VARIABLE;
                data->val.var = *ptr;
                return ptr + 1;
        }

        return ptr;
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

static char *read_formula(char *ptr, node *parent, node **parent_leaf)
{
        assert(ptr);

        if (*ptr != '(')
                return nullptr;

        ptr++;

        node *curr = create_node();
        if (parent_leaf)
                *parent_leaf = curr;
        if (parent)
                curr->parent = parent;

        if (*ptr == '(')
                ptr = read_formula(ptr, curr, &curr->left);

        if (!ptr)
                return nullptr;

        wf_data data = {0};
        ptr = read_data(ptr, &data);

        curr->data = data;

        if (*ptr == '(') {
                if (data.type != WF_OPERATOR)
                        return nullptr;

                ptr = read_formula(ptr, curr, &curr->right);
                if (!ptr)
                        return nullptr;
        }

        if (*ptr != ')')
                return nullptr;

        return ++ptr;
}

node *parse_infix(const char *fname)
{
        assert(fname);

        mmap_data data = {0};
        int err = mmap_in(&data, fname); 
        if (err)
                return nullptr;

        remove_spaces(data.buf);

        node *rt = nullptr;
        char *r  = read_formula(data.buf, rt, &rt);
        if (!r) {
                fprintf(stderr, "Invalid format\n");
$               (free_tree(rt);)
                rt = nullptr;
        }

        mmap_free(&data);
        return rt;
}


