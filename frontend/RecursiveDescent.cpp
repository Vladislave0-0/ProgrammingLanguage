#include "./RecursiveDescent.h"

//=========================================================================================

Node* make_tree(struct Tree* tree)
{
    Node* root = getGen(tree);

    if(root == nullptr)
    {
        printf("in make_tree root nullptr\n");
    }

    return root;
}

//=========================================================================================

Node* getGen(struct Tree* tree)
{    
    Node* root = getProg(tree);

    if(TYPE != SEMICLN && TYPE != CRL_BRC)
    {
        return nullptr;
    }

    return root;
}

//=========================================================================================

Node* getProg(struct Tree* tree)
{
    Node* node        = nullptr;
    Node* left_child  = nullptr;
    Node* right_child = nullptr;

    if(TYPE == VAR_DECL)
    {
        left_child  = getVar_decl(tree);
        if(TOK < tree->tok_num - 1)
        {
            TOK++;
        }
        right_child = getProg(tree);
        node = CREATE_EMPTY(left_child, right_child);
    }

    else if(TYPE == FNC_DECL)
    {
        left_child  = getFnc_decl(tree);  
        if(TOK < tree->tok_num - 1)
        {
            TOK++;
        }

        right_child = getProg(tree);
        
        node = CREATE_EMPTY(left_child, right_child);
    }
    
    else if(TOK != 0 && TYPE == LOOP)
    {
        left_child  = getLoop(tree);  
        if(TOK < tree->tok_num - 1)
        {
            TOK++;
        }
        right_child = getProg(tree);
        node = CREATE_EMPTY(left_child, right_child);
    }

    else if(TYPE == COND_OP)
    {
        left_child  = getIf(tree);
        if(TYPE != CRL_BRC && TYPE != VAR_DECL && TYPE != RETURN && TOK < tree->tok_num - 1)
        {
            TOK++;
        }
        right_child = getProg(tree);
        node = CREATE_EMPTY(left_child, right_child);
    }

    else if(TYPE == RETURN)
    {
        left_child = CREATE_RETURN;
        TOK++;
        left_child->right_child = getExp(tree);
        right_child = getProg(tree);
        node = CREATE_EMPTY(left_child, right_child);
        TOK++;
    }

    else if(TYPE == FNC_NAME)
    {
        left_child = getFnc_name(tree);
        TOK += 2;
        right_child = getProg(tree);
        node = CREATE_EMPTY(left_child, right_child);
    }

    else if(TYPE == VAR_NAME)
    {
        left_child = getAssign(tree);
        if(TOK < tree->tok_num - 1)
        {
            TOK++;
        }
        right_child = getProg(tree);
        node = CREATE_EMPTY(left_child, right_child);
    }

    return node;
}

//=========================================================================================

Node* getFnc_name(struct Tree* tree)
{
    Node* node = nullptr;
    if(!strcmp(TEXT, "printf"))
    {
        node = CREATE_FNC_NAME(nullptr, TEXT);
        TOK += 2;
        if(TYPE == QUOTE)
        {
            TOK++;
            if(TYPE == STRING)
            {
                node->right_child = CREATE_STRING(TEXT);
                TOK += 2;
            }
            return node;
        }

        node->right_child = getExp(tree);
    }

    else if(!strcmp(TEXT, "scanf"))
    {
        node = CREATE_FNC_NAME(nullptr, TEXT);
        TOK += 2;
        node->right_child = getWord(tree);
    }

    return node;
}

//=========================================================================================

Node* getIf(struct Tree* tree)
{
    Node* node = nullptr;
    if(!strcmp("if", TEXT))
    {
        node = CREATE_COND_OP(CREATE_EMPTY(nullptr, nullptr), TEXT);
        TOK += 2;
        Node* left_exp = getExp(tree);
        Node* log_op = CREATE_LOG_OP(TEXT);
        TOK++;
        Node* right_exp = getExp(tree);
        TOK += 2;
        node->left_child->left_child = log_op;
        node->left_child->left_child->left_child = left_exp;
        node->left_child->left_child->right_child = right_exp;
        node->left_child->right_child = getProg(tree);
        if(TOK < tree->tok_num - 1)
        {
            TOK++;
        }
    }

    if(!strcmp("elif", TEXT) || !strcmp("else", TEXT))
    {
        node->right_child = getElif_else(tree);
    }

    return node;
}

//=========================================================================================

Node* getElif_else(struct Tree* tree)
{
    Node* node = nullptr;

    if(!strcmp("elif", TEXT))
    {
        node = CREATE_EMPTY(nullptr, nullptr);
        TOK += 2;
        Node* left_exp = getExp(tree);
        Node* log_op = CREATE_LOG_OP(TEXT);
        TOK++;
        Node* right_exp = getExp(tree);
        TOK += 2;

        node->left_child = CREATE_COND_OP(log_op, "elif");
        node->left_child->left_child->left_child = left_exp;
        node->left_child->left_child->right_child = right_exp;
        node->left_child->right_child = getProg(tree);

        if(TOK < tree->tok_num - 1)
        {
            TOK++;
        }
        node->right_child = getElif_else(tree);
    }

    else if(!strcmp("else", TEXT))
    {
        node = CREATE_COND_OP(nullptr, "else");
        TOK += 2;
        node->right_child = getProg(tree);
        TOK++;
    }

    return node;
}

//=========================================================================================

Node* getFnc_decl(struct Tree* tree)
{
    Node* node = CREATE_FNC_DECL;
    TOK++;
    if(TYPE == MAIN)
    {
        node->left_child = CREATE_FNC_NAME(nullptr, TEXT);
        TOK += 4;

        node->left_child->right_child = getProg(tree);
    }

    else if(TYPE == FNC_NAME)
    {
        char* old_tok_text = TEXT;
        TOK += 2;
        node->left_child  = CREATE_FNC_NAME(nullptr, old_tok_text);
        node->left_child->right_child = getFnc_params(tree);

        TOK += 2;
        if(TYPE == VAR_DECL || TYPE == LOOP || TYPE == COND_OP || 
           TYPE == RETURN   || TYPE == VAR_NAME)
        {
            node->right_child = getProg(tree);
        }

        else
        {
            printf(RED "\n\nUNKNOWN TYPE IN getFnc_decl\n\n" RESET);
        }
    }

    return node;
}

//=========================================================================================

Node* getLoop(struct Tree* tree)
{
    Node* node = CREATE_LOOP;
    TOK += 2;
    Node* cond_left = getExp(tree);
    node->left_child = CREATE_LOG_OP(TEXT);
    TOK++;
    Node* cond_right = getExp(tree);
    node->left_child->left_child = cond_left;
    node->left_child->right_child = cond_right;
    TOK += 2;
    node->right_child = getProg(tree);

    return node;
}

//=========================================================================================

Node* getFnc_params(struct Tree* tree)
{
    Node* node = CREATE_FNC_PARAMS;
    node->left_child = getExp(tree);

    if(TYPE == COMMA)
    {
        TOK++;
        node->right_child = getFnc_params(tree);
    }

    return node;
}

//=========================================================================================

Node* getVar_decl(struct Tree* tree)
{
    TOK++;
    Node* right_child = getAssign(tree);

    return CREATE_VAR_DECL(right_child);
}

//=========================================================================================

Node* getAssign(struct Tree* tree)
{
    Node* node = CREATE_ASSIGN(nullptr, nullptr);
    node->left_child = getWord(tree);
    TOK++;
    node->right_child = getExp(tree);

    return node;
}

//=========================================================================================

Node* getExp(struct Tree* tree)
{
    Node* left_child  = getMul(tree);
    Node* right_child = nullptr;
    Node* parent      = nullptr;

    while(!strcmp(TEXT, "+") || !strcmp(TEXT, "-"))
    {
        if(!strcmp(TEXT, "+"))
        {
            TOK++;
            parent = ADD_NODE(left_child, nullptr);
        }

        else
        {
            TOK++;
            parent = SUB_NODE(left_child, nullptr);
        }

        right_child = getMul(tree);
        if(right_child == nullptr)
        {
            return nullptr;
        }

        parent->right_child = right_child;
        left_child = parent;
    }

    return left_child;
}

//=========================================================================================

Node* getMul(struct Tree* tree)
{
    Node* left_child  = getDeg(tree);
    Node* right_child = nullptr;
    Node* parent      = nullptr; 

    while(!strcmp(TEXT, "*") || !strcmp(TEXT, "/"))
    {
        if(!strcmp(TEXT, "*"))
        {
            TOK++;
            parent = MUL_NODE(left_child, nullptr);
        }

        else
        {
            TOK++;
            parent = DIV_NODE(left_child, nullptr);
        }

        right_child = getDeg(tree);
        if(right_child == nullptr)
        {
            return nullptr;
        }

        parent->right_child = right_child;
        left_child = parent;
    }

    return left_child;
}

//=========================================================================================

Node* getDeg(struct Tree* tree)
{
    Node* left_child  = getBrc(tree);
    Node* right_child = nullptr;
    Node* parent      = nullptr;

    while(!strcmp(TEXT, "^"))
    {
        TOK++;
        right_child = getBrc(tree);

        if(right_child == nullptr)
        {
            return nullptr;
        }

        parent = POW_NODE(left_child, right_child);
        left_child = parent;
    }

    return left_child;
}

//=========================================================================================

Node* getBrc(struct Tree* tree)
{
    Node* node = nullptr;

    if(!strcmp(TEXT, "("))
    {
        TOK++;
        node = getExp(tree);

        if(strcmp(TOK_ARR[TOK++].text, ")"))
        {
            return nullptr;
        }
    }

    else if(TYPE == NUMBER)
    {
        node = getNum(tree);
    }

    else
    {
        node = getWord(tree);
    }

    return node;
}

//=========================================================================================

Node* getWord(struct Tree* tree)
{
    Node* node = nullptr;

    if(TYPE == VAR_NAME)
    {
        node = CREATE_VAR_NAME(TEXT);
        TOK++;
        return node;
    }

    else if(TYPE == FNC_NAME)
    {
        const char* func_text = TEXT;

        if(!strcasecmp(func_text, "sqrt")   || !strcasecmp(func_text, "sin")   || !strcasecmp(func_text, "cos")    || 
           !strcasecmp(func_text, "tg")     || !strcasecmp(func_text, "ctg")   || !strcasecmp(func_text, "arcsin") ||
           !strcasecmp(func_text, "arccos") || !strcasecmp(func_text, "arctg") || !strcasecmp(func_text, "arcctg") ||
           !strcasecmp(func_text, "sh")     || !strcasecmp(func_text, "ch")    || !strcasecmp(func_text, "ln")     ||
           !strcasecmp(func_text, "exp"))
        {
            TOK += 2;
            Node* arg_node = getExp(tree);

            TOK++;

            if(     !strcasecmp(func_text, "sqrt"))      node = SQRT_NODE  (arg_node);
            else if(!strcasecmp(func_text, "sin"))       node = SIN_NODE   (arg_node);
            else if(!strcasecmp(func_text, "cos"))       node = COS_NODE   (arg_node);
            else if(!strcasecmp(func_text, "tg"))        node = TG_NODE    (arg_node);
            else if(!strcasecmp(func_text, "ctg"))       node = CTG_NODE   (arg_node);
            else if(!strcasecmp(func_text, "arcsin"))    node = ARCSIN_NODE(arg_node);
            else if(!strcasecmp(func_text, "arccos"))    node = ARCCOS_NODE(arg_node);
            else if(!strcasecmp(func_text, "arctg"))     node = ARCTG_NODE (arg_node);
            else if(!strcasecmp(func_text, "arcctg"))    node = ARCCTG_NODE(arg_node);
            else if(!strcasecmp(func_text, "sh"))        node = SH_NODE    (arg_node);
            else if(!strcasecmp(func_text, "ch"))        node = CH_NODE    (arg_node);
            else if(!strcasecmp(func_text, "ln"))        node = LN_NODE    (arg_node);
            else if(!strcasecmp(func_text, "exp"))       node = EXP_NODE   (arg_node);
            else return nullptr;
        }
        
        else
        {
            TOK += 2;
            Node* arg_node = getFnc_params(tree);
            node = CREATE_FNC_NAME(nullptr, func_text);
            node->right_child = arg_node;
            TOK++;
        }

    }

    return node;
}

//=========================================================================================

Node* getNum(struct Tree* tree)
{
    if(TYPE == NUMBER)
    {
        return CREATE_NUM(TOK_ARR[TOK++].number);
    }
    else
    {
        printf("ERROR IN GET_NUM");
        return nullptr;
    }
}
