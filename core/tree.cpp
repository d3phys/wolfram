#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <logs.h>

#include <wolfram/tree.h>

static void unlink_tree(node *n);

node *copy_tree(node *n)
{
        assert(n);

        node *newbie = create_node();
        if (!newbie)
                return nullptr;

        newbie->data = n->data;

        if (n->left) {
                newbie->left  = copy_tree(n->left);
                if (!newbie->left) {
                        free_tree(newbie);
                        return nullptr;
                }

                newbie->left->parent  = newbie;
        }

        if (n->right) {
                newbie->right = copy_tree(n->right);
                if (!newbie->right) {
                        free_tree(newbie);
                        return nullptr;
                }

                newbie->right->parent = newbie;
        }


        return newbie;
}

static void unlink_tree(node *n)
{
        assert(n);

        if (n->parent) {
                if (n->parent->left  == n)
                        n->parent->left  = nullptr;
                if (n->parent->right == n)
                        n->parent->right = nullptr;
        }
}

void free_tree(node *root)
{
        assert(root);

        unlink_tree(root);

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

        if (parent)
                newbie->parent = parent;

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

