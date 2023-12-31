#ifndef TREE_H
#define TREE_H

//=======================================================================================================

#include "../frontend/InputProcessing.h"
#include "../include/colors.h"

//=======================================================================================================

enum NodeType
{
    _NUL      = 0,
    _VAR_DECL = 1,
    _FNC_DECL = 2,
    _ASSIGN   = 3,
    _ARTH_OP  = 4,
    _LOG_OP   = 5,
    _COND_OP  = 6,
    _LOOP     = 7,
    _NUMBER   = 8,
    _VAR_NAME = 9,
    _FNC_NAME = 10,
    _MAIN     = 11,
    _RETURN   = 12,
    _RND_BRC  = 13,
    _CRL_BRC  = 14,
    _COMMA    = 15,
    _SEMICLN  = 16,
    _QUOTE    = 17, 
    _STRING   = 18,
    _EMPTY    = 19,
    _PARAM    = 20,
};

union Value
{
    float number;
    const char* name;
};


struct Node
{
    NodeType type;
    Value val;

    Node* parent      = nullptr;
    Node* left_child  = nullptr;
    Node* right_child = nullptr;    
};


struct Var
{
    char text[MAX_WORD_LENGTH] = {};
    float value = POISON;
};


struct Tree
{
    TokenInfo* tok_arr = nullptr;
    Node* root         = nullptr;

    size_t tok_num = 0; 
    size_t cur_tok = 0;

    int error = 0;
};

//=======================================================================================================

Tree* tree_ctor(struct InputInfo* InputInfo);

//=======================================================================================================

Node* create_node(Node* left_child, Node* right_child, NodeType type, const char* name, float value);

//=======================================================================================================

void init_tree_dtor(struct Tree* tree);

//=======================================================================================================

void childs_dtor(struct Node* node);

//=======================================================================================================

void tree_simplification(Node* root);

//=======================================================================================================

void parents_copy(struct Node* root);

//=======================================================================================================

#endif
