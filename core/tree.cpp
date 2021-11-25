#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <logs.h>
#include <limits.h>

#include <wolfram/tree.h>
#include <wolfram/wolfram.h>

static void unlink_tree(node *n);

double *node_num(node *n)
{
        assert(n);
        assert(n->type == NODE_NUM);

        if (n->type == NODE_NUM)
                return &n->data.num;

        return 0;
}

char *node_var(node *n)
{
        assert(n);
        assert(n->type == NODE_VAR);

        if (n->type == NODE_VAR)
                return &n->data.var;

        return nullptr;
}

unsigned *node_op(node *n)
{
        assert(n);
        assert(n->type == NODE_OP);

        if (n->type == NODE_OP)
                return &n->data.op;

        return 0;
}

node *copy_tree(node *n)
{
        assert(n);

        node *newbie = create_node();
        if (!newbie)
                return nullptr;

        newbie->type = n->type;
        newbie->data = n->data;

        if (n->left) {
                newbie->left  = copy_tree(n->left);
                if (!newbie->left) {
                        free_tree(newbie);
                        return nullptr;
                }
        }

        if (n->right) {
                newbie->right = copy_tree(n->right);
                if (!newbie->right) {
                        free_tree(newbie);
                        return nullptr;
                }
        }

        return newbie;
}


void free_tree(node *root)
{
        assert(root);

        if (root->left)
                free_tree(root->left);
        if (root->right)
                free_tree(root->right);

$       (free(root);)
}

node *create_node(node *parent)
{
$       (node *newbie = (node *)calloc(1, sizeof(node));)
        if (!newbie) {
                fprintf(logs, "Can't create node\n");
                return newbie;
        }

        return newbie;
}

void visit_tree(node *root, void (*action)(node *nd))
{
        assert(root);
        assert(action);

        if (!root)
                return;

        action(root);

        if (root->left)  
                visit_tree(root->left, action);
        if (root->right) 
                visit_tree(root->right, action);
}

