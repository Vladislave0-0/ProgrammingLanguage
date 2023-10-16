#include "./Tree.h"

//==================================================================================================================

Tree* tree_ctor(struct InputInfo* InputInfo)
{
    Tree* tree = (Tree*)calloc(1, sizeof(Tree));

    tree->tok_arr = InputInfo->tok_arr;
    tree->tok_num = InputInfo->tok_num;

    return tree;
}

//==================================================================================================================

Node* create_node(Node* left_child, Node* right_child, NodeType type, const char* name, float value)
{
    Node* node = (Node*)calloc(1, sizeof(Node));
    node->type = type;

    if(type == _NUMBER)
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

//==================================================================================================================

void tree_simplification(Node* root)
{
    if(root != nullptr && root->left_child != nullptr)
    {
        tree_simplification(root->left_child);
    }

    if(root != nullptr && root->right_child != nullptr)
    {
        tree_simplification(root->right_child);
    }

    if(root != nullptr && root->type == _EMPTY)
    {
        if(root->left_child != nullptr && root->right_child == nullptr && root->parent != nullptr)
        {
            if(root->parent->left_child == root)
            {
                root->parent->left_child = root->left_child;
            }

            else if(root->parent->right_child == root)
            {
                root->parent->right_child = root->left_child;
            }

            free(root);
        }

        else if(root->right_child != nullptr && root->left_child == nullptr && root->parent != nullptr)
        {
            if(root->parent->left_child == root)
            {
                root->parent->left_child = root->right_child;
            }

            else if(root->parent->right_child == root)
            {
                root->parent->right_child = root->right_child;
            }
            
            free(root);
        }
    }

    else if(root != nullptr && root->type == _PARAM)
    {
        if(root->left_child == nullptr)
        {
            root->parent->right_child = nullptr;
            free(root);
        }
    }

    else if(root != nullptr && root->type == _VAR_DECL)
    {
        Node* left_child  = root->right_child->left_child;
        Node* right_child = root->right_child->right_child;
        free(root->right_child);
        root->left_child  = left_child;
        root->right_child = right_child; 
    }
}

//==================================================================================================================

void parents_copy(struct Node* root)
{
    if(root == nullptr)
    {
        return;
    }

    if(root->left_child != nullptr)
    {
        root->left_child->parent = root;
        parents_copy(root->left_child);
    }

    if(root->right_child != nullptr)
    {
        root->right_child->parent = root;
        parents_copy(root->right_child);
    }
}