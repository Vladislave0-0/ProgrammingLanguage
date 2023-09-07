#include "Tree.h"

//==================================================================================================================

Tree* tree_ctor(struct TextInfo* TextInfo)
{
    Tree* tree = (Tree*)calloc(1, sizeof(Tree));

    tree->tok_arr = TextInfo->tok_arr;
    tree->tok_num = TextInfo->tok_num;

    return tree;
}

//==================================================================================================================

Node* create_node(Node* left_child, Node* right_child, NodeType type, const char* name, float value)
{
    Node* node = (Node*)calloc(1, sizeof(Node));
    node->type = type;

    if(type == _NUM)
    {
        node->val.number = value;
    }
    
    else
    {
        node->val.name = name;
    }

    node->left_child = left_child;
    node->right_child = right_child;

    return node;
}

//==================================================================================================================

void init_tree_dtor(struct Tree* tree)
{
    childs_dtor(tree->root);

    free(tree);
    tree = nullptr;

    printf(GRN "\nThe program was executed successfully!\n\n" RESET);
}

//==================================================================================================================

void childs_dtor(struct Node* node)
{
    if(node == nullptr)
    {
        return;
    }

    if(node->left_child != nullptr)
    {
        childs_dtor(node->left_child);
    }

    if(node->right_child != nullptr)
    {
        childs_dtor(node->right_child);
    }

    if(node != nullptr)
    {
        node->val.number = POISON;
        node->type = _NUL;
        free(node);
        node = nullptr;
    }
}
