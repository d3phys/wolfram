#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <logs.h>

#include <wolfram/tree.h>
#include <wolfram/stack.h>


node *create_node(node *parent, int type)
{
$       (node *newbie = (node *)calloc(1, sizeof(node));)
        if (!newbie) {
                fprintf(logs, "Can't create node\n");
                return newbie;
        }

        if (parent) {
$               (newbie->parent = parent;)
        }

        newbie->type = type;
        return newbie;
}

void delete_node(node *vet)
{
        assert(vet);

        if (vet->parent) {
                if (vet->parent->left == vet)
                        vet->parent->left  = nullptr;
                if (vet->parent->right == vet)
                        vet->parent->right = nullptr;
        }

        if (vet->left)
                vet->left->parent  = nullptr;

        if (vet->right)
                vet->right->parent = nullptr;

$       (free(vet);)
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

