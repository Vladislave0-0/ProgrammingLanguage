#include "backend.h"
#include "math.h"

//=====================================================================================================================

#define CHECK_ERROR                     \
        if(BackInfo->error != 0)        \
        {                               \
            return BackInfo->error;     \
        }                               \

//=====================================================================================================================

int backend_ctor(struct BackendInfo* BackInfo, struct InputInfo* InputInfo, Node* root, const char* filename)
{
    if(strcmp(BackInfo->signature, InputInfo->signature))
    {
        return ERROR_UNKNOWN_FUNCTION;
    }

    open_back_file(BackInfo, filename);
    CHECK_ERROR;
    fill_back_struct(BackInfo, InputInfo, root);
    CHECK_ERROR;

    translate_tree_to_asm(BackInfo, root, BackInfo->asm_file);
    CHECK_ERROR;

    return SUCCESS;
}

//=====================================================================================================================

void fill_back_struct(struct BackendInfo* BackInfo, struct InputInfo* InputInfo, Node* root)
{
    BackInfo->tok_arr = InputInfo->tok_arr;
    BackInfo->root    = root;
    BackInfo->fnc_num = InputInfo->fnc_num;

    BackInfo->fnc_arr = (FuncInfo*)calloc(BackInfo->fnc_num, sizeof(FuncInfo));
    if(BackInfo->fnc_arr == nullptr)
    {
        BackInfo->error = ERROR_FUNC_ARR_NULLPTR;
    }

    for(size_t i = 0; i < BackInfo->fnc_num; i++)
    {
        strcpy(BackInfo->fnc_arr[i].name, InputInfo->fnc_arr[i].name);

        BackInfo->fnc_arr[i].args_num      = InputInfo->fnc_arr[i].args_num;
        BackInfo->fnc_arr[i].decl_vars_num = InputInfo->fnc_arr[i].vars_num;
        BackInfo->fnc_arr[i].all_vars_num  = InputInfo->fnc_arr[i].args_num + InputInfo->fnc_arr[i].vars_num;

        BackInfo->fnc_arr[i].args_arr      = (VarInfo*)calloc(BackInfo->fnc_arr[i].args_num,      sizeof(VarInfo));
        BackInfo->fnc_arr[i].decl_vars_arr = (VarInfo*)calloc(BackInfo->fnc_arr[i].decl_vars_num, sizeof(VarInfo));
        BackInfo->fnc_arr[i].all_vars_arr  = (VarInfo*)calloc(BackInfo->fnc_arr[i].all_vars_num,  sizeof(VarInfo));
        if(BackInfo->fnc_arr[i].args_arr && BackInfo->fnc_arr[i].decl_vars_arr && BackInfo->fnc_arr[i].all_vars_arr == nullptr)
        {
            BackInfo->error = ERROR_VARS_ARR_NULLPTR;
        }

        size_t cur_all_var = 0;
        for(size_t j = 0; j < BackInfo->fnc_arr[i].args_num; j++)
        {
            strcpy(BackInfo->fnc_arr[i].args_arr[j].text, InputInfo->fnc_arr[i].args_arr[j]);
            strcpy(BackInfo->fnc_arr[i].all_vars_arr[cur_all_var++].text, InputInfo->fnc_arr[i].args_arr[j]);
        }
        for(size_t j = 0; j < BackInfo->fnc_arr[i].decl_vars_num; j++)
        {
            strcpy(BackInfo->fnc_arr[i].decl_vars_arr[j].text, InputInfo->fnc_arr[i].decl_vars[j]);
            strcpy(BackInfo->fnc_arr[i].all_vars_arr[cur_all_var++].text, InputInfo->fnc_arr[i].decl_vars[j]);
        }
    }

    free(InputInfo->fnc_arr);
}

//=====================================================================================================================

void open_back_file(struct BackendInfo* BackInfo, const char* filename)
{
    char path[100] = {};
    strcat(path, "./output/");

    char new_filename[50] = {};
    size_t filename_len = strlen(filename);
    for(size_t i = 0, j = 0; i < filename_len - 3; i++)
    {
        new_filename[j++] = filename[i];
    }
    strcat(path, new_filename);
    strcat(path, "asm");
    
    BackInfo->asm_file = fopen(path, "w");
}

//=====================================================================================================================

#define STRING_A "\npush fx\npush ex\nadd\npop ex\n\n"
#define STRING_B "push [ex]\npush ex\npush [ex]\nsub\npop ex\npop fx\n"
#define STRING_C "pop ax\nret\n\n"
#define STRING_D "\npush fx\npop [ex]\npop fx\n\n"

int translate_tree_to_asm(struct BackendInfo* BackInfo, Node* root, FILE* asm_file)
{
    CHECK_ERROR;

    fprintf(asm_file, STRING_A);
    fprintf(asm_file, "push %lu\n", BackInfo->fnc_arr[0].all_vars_num + 1);
    fprintf(asm_file, STRING_D);
    fprintf(asm_file, "call :main\n");
    fprintf(asm_file, "hlt\n\n");

    translate_program(BackInfo, root, asm_file);
    CHECK_ERROR;

    fclose(asm_file);
    return SUCCESS;
}

//=====================================================================================================================

int translate_program(struct BackendInfo* BackInfo, Node* root, FILE* asm_file)
{
    CHECK_ERROR;

    switch(root->type)
    {
        case(_EMPTY):
        {
            if(root->left_child != nullptr)
            {
                translate_program(BackInfo, root->left_child, asm_file);
                CHECK_ERROR;
            }

            if(root->right_child != nullptr)
            {
                translate_program(BackInfo, root->right_child, asm_file);
                CHECK_ERROR;
            }

            break;
        }

        case(_FNC_DECL):
        {
            translate_func_decl(BackInfo, root, asm_file);
            CHECK_ERROR;
            break;
        }

        default:
        {
            printf(RED "Unknown syntax in translate_program!\n" RESET);
            BackInfo->error = ERROR_TRANSLATE_PROGRAM;
            return ERROR_TRANSLATE_PROGRAM;
        }
    }

    return SUCCESS;
}

//=====================================================================================================================

int translate_func_decl(struct BackendInfo* BackInfo, Node* root, FILE* asm_file)
{
    CHECK_ERROR;

    fprintf(asm_file, "\n%s:\n", root->left_child->val.name);
    size_t cur_fnc = get_cur_fnc(BackInfo, root->left_child);
    CHECK_ERROR;
    translate_func_body(BackInfo, root->right_child, cur_fnc, asm_file);
    CHECK_ERROR;
    return SUCCESS;
}

//=====================================================================================================================

int translate_func_body(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file)
{
    CHECK_ERROR;

    switch(root->type)
    {
        case(_EMPTY):
        {
            if(root->left_child != nullptr)
            {
                translate_func_body(BackInfo, root->left_child, fnc_id, asm_file);
                CHECK_ERROR;
            }

            if(root->right_child != nullptr)
            {
                translate_func_body(BackInfo, root->right_child, fnc_id, asm_file);
                CHECK_ERROR;
            }

            break;
        }

        case(_VAR_DECL):
        {
            translate_var_decl(BackInfo, root, fnc_id, asm_file);
            break;
        }

        case(_ASSIGN):
        {
            translate_var_name(BackInfo, root, fnc_id, asm_file);
            break;
        }

        case(_COND_OP):
        {
            BackInfo->if_num++;
            size_t cur_if_num = BackInfo->if_num;
            translate_condition(BackInfo, root, fnc_id, cur_if_num, asm_file);
            fprintf(asm_file, "\nexit_cond_%lu:\n", cur_if_num);
            break;
        }

        case(_LOOP):
        {
            translate_while(BackInfo, root, fnc_id, asm_file);
            break;
        }
        
        case(_FNC_NAME):
        {
            translate_fnc_call(BackInfo, root, fnc_id, 0, asm_file);
            break;
        }
        
        case(_RETURN):
        {
            translate_return(BackInfo, root, fnc_id, asm_file);
            break;
        }
        
        default:
        {
            printf(RED "Unknown type(%d) in translate_fnc_body!\n" RESET, root->type);
            BackInfo->error = ERROR_TRANSLATE_FUNC_BODY;
            return ERROR_TRANSLATE_FUNC_BODY;
        }
    }

    return SUCCESS;
}

//=====================================================================================================================

int translate_var_decl(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file)
{
    CHECK_ERROR;

    size_t cur_var = get_cur_var(BackInfo, root->left_child, fnc_id);
    CHECK_ERROR;
    translate_right_value(BackInfo, root->right_child, fnc_id, asm_file);
    CHECK_ERROR;
    fprintf(asm_file, "pop [ex+%lu]\n", cur_var + 1);

    return SUCCESS;
}

//=====================================================================================================================

int translate_var_name(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file)
{
    CHECK_ERROR;

    translate_right_value(BackInfo, root->right_child, fnc_id, asm_file);
    CHECK_ERROR;
    fprintf(asm_file, "pop [ex+%lu]\n", get_cur_var(BackInfo, root->left_child, fnc_id) + 1);
    CHECK_ERROR;
    return SUCCESS;
}

//=====================================================================================================================

int translate_return(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file)
{
    CHECK_ERROR;

    fprintf(asm_file, "\n");
    translate_right_value(BackInfo, root->right_child, fnc_id, asm_file);
    CHECK_ERROR;
    fprintf(asm_file, STRING_B);
    fprintf(asm_file, STRING_C);

    return SUCCESS;
}

#undef STRING_B
#undef STRING_C

//=====================================================================================================================

int translate_right_value(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file)
{
    CHECK_ERROR;

    if(root->type == _ARTH_OP && root->left_child != nullptr)
    {
        translate_right_value(BackInfo, root->left_child, fnc_id, asm_file);
        CHECK_ERROR;
    }

    if(root->type == _ARTH_OP && root->right_child != nullptr)
    {
        translate_right_value(BackInfo, root->right_child, fnc_id, asm_file);
        CHECK_ERROR;
    }

    switch(root->type)
    {
        case(_ARTH_OP):
        {
            translate_arth_op(root, asm_file);
            CHECK_ERROR;
            break;
        }

        case(_NUMBER):
        {
            fprintf(asm_file, "push %lg\n", root->val.number);
            break;
        }

        case(_VAR_NAME):
        {
            size_t cur_var = get_cur_var(BackInfo, root, fnc_id);
            CHECK_ERROR;
            fprintf(asm_file, "push [ex+%lu]\n", cur_var + 1);
            break;
        }

        case(_FNC_NAME):
        {
            translate_fnc_call(BackInfo, root, fnc_id, 1, asm_file);
            CHECK_ERROR;
            break;
        }

        default:
        {
            printf(RED "Unknown syntax in translate_right_value!\n" RESET);
            BackInfo->error = ERROR_TRANSLATE_RIGHT_VALUE;
            return ERROR_TRANSLATE_RIGHT_VALUE;
        }
    }

    return SUCCESS;
}

//=====================================================================================================================

int translate_arth_op(Node* root, FILE* asm_file)
{
    if     (!strcmp(root->val.name, "+"))    fprintf(asm_file, "add\n");
    else if(!strcmp(root->val.name, "-"))    fprintf(asm_file, "sub\n");
    else if(!strcmp(root->val.name, "*"))    fprintf(asm_file, "mul\n");
    else if(!strcmp(root->val.name, "/"))    fprintf(asm_file, "div\n");
    else if(!strcmp(root->val.name, "^"))    fprintf(asm_file, "pow\n");

    return SUCCESS;
}

//=====================================================================================================================

#define CMP_STD_FNC(fnc_name)                                                   \
    else if(!strcmp(root->val.name, fnc_name))                                  \
    {                                                                           \
        translate_right_value(BackInfo, root->right_child, fnc_id, asm_file);   \
        CHECK_ERROR;                                                            \
        fprintf(asm_file, "%s\n", fnc_name);                                    \
    }                                                                           \

int translate_fnc_call(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, int r_val_flag, FILE* asm_file)
{
    CHECK_ERROR;

    if(0) {}
    CMP_STD_FNC("sqrt")
    CMP_STD_FNC("sin")
    CMP_STD_FNC("cos")
    CMP_STD_FNC("tg")
    CMP_STD_FNC("ctg")
    CMP_STD_FNC("arcsin")
    CMP_STD_FNC("arccos")
    CMP_STD_FNC("arctg")
    CMP_STD_FNC("arcctg")
    CMP_STD_FNC("sh")
    CMP_STD_FNC("ch")
    CMP_STD_FNC("ln")
    CMP_STD_FNC("exp")
    else if(!strcmp(root->val.name, "printf") || !strcmp(root->val.name, "scanf"))
    {
        translate_printf_scanf(BackInfo, root, fnc_id, asm_file);
        CHECK_ERROR;
    }
    else
    {
        translate_programm_fnc(BackInfo, root, fnc_id, asm_file);
        CHECK_ERROR;
        if(r_val_flag)
        {
            fprintf(asm_file, "push ax\n");
        }
    }

    return SUCCESS;
}

#undef CMP_STD_FNC

//=====================================================================================================================

int translate_fnc_call_params(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file)
{
    CHECK_ERROR;

    if(root->type == _PARAM && root->right_child != nullptr)
    {
        translate_fnc_call_params(BackInfo, root->right_child, fnc_id, asm_file);
        CHECK_ERROR;
        BackInfo->params_num++;
    }
    translate_right_value(BackInfo, root->left_child, fnc_id, asm_file);
    CHECK_ERROR;

    return SUCCESS;
}

//=====================================================================================================================

int translate_programm_fnc(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file)
{
    CHECK_ERROR;

    size_t cur_fnc = get_cur_fnc(BackInfo, root);
    CHECK_ERROR;

    fprintf(asm_file, STRING_A);
    fprintf(asm_file, "push %lu\n", BackInfo->fnc_arr[cur_fnc].all_vars_num + 1);
    fprintf(asm_file, STRING_D);

    if(root->right_child != nullptr)
    {
        BackInfo->params_num = 1;
        translate_fnc_call_params(BackInfo, root->right_child, fnc_id, asm_file);
        CHECK_ERROR;

        for(size_t i = 1; i <= BackInfo->params_num; i++)
        {
            fprintf(asm_file, "pop [ex+%lu]\n", i);
        }
    }

    fprintf(asm_file, "call :%s\n", root->val.name);

    return SUCCESS;
}

#undef STRING_A
#undef STRING_D


//=====================================================================================================================

int translate_printf_scanf(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file)
{
    CHECK_ERROR;

    if(!strcmp(root->val.name, "printf"))
    {
        if(root->right_child == nullptr)
        {
            return SUCCESS;
        }

        if(root->right_child->type != _STRING)
        {
            translate_right_value(BackInfo, root->right_child, fnc_id, asm_file);
            CHECK_ERROR;
            fprintf(asm_file, "out\n");
            return SUCCESS;
        }

        char buf0[19], buf[17] = {0};
        sprintf(buf0, "%p", root->right_child->val.name);
        size_t len = strlen(buf0);

        size_t i = 0;
        for(; i < 18 - len; i++)
        {
            buf[i] = '0';
        }
        for(size_t j = 2; j < len; j++, i++)
        {
            buf[i] = buf0[j];
        }

        char str1[9], str2[9] = {0};

        for(size_t z = 0, w = 0; z < 8; z++, w++)
        {
            str1[w] = buf[z];
        }
        str1[8] = '\0';

        for(size_t z = 8, w = 0; z < 16; z++, w++)
        {
            str2[w] = buf[z];
        }
        str2[8] = '\0';

        fprintf(asm_file, "push %d\n", hex_to_dec(str2));
        fprintf(asm_file, "push %d\n", hex_to_dec(str1));
        fprintf(asm_file, "strout\n");

        return SUCCESS;
    }

    fprintf(asm_file, "in\n");

    size_t cur_var = get_cur_var(BackInfo, root->right_child, fnc_id);
    CHECK_ERROR;

    fprintf(asm_file, "pop [ex+%lu]\n", cur_var + 1);

    return SUCCESS;
}

//=====================================================================================================================

int translate_while(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file)
{
    CHECK_ERROR;

    static int cur_while = 0;
    cur_while++;

    char while_buf[MAX_WORD_LENGTH] = {0};
    sprintf(while_buf, "while_%d", cur_while);
    fprintf(asm_file, "\n%s:\n", while_buf);

    translate_right_value(BackInfo, root->left_child->left_child, fnc_id, asm_file);
    CHECK_ERROR;
    translate_right_value(BackInfo, root->left_child->right_child, fnc_id, asm_file);
    CHECK_ERROR;
    translate_cond_or_while_op(root->left_child, asm_file);
    CHECK_ERROR;

    char while_exec_buf[MAX_WORD_LENGTH] = {0};
    sprintf(while_exec_buf, "while_exec_%d", cur_while);
    fprintf(asm_file, ":%s\n", while_exec_buf);

    char while_exit[MAX_WORD_LENGTH] = {0};
    sprintf(while_exit, "while_exit_%d", cur_while);
    fprintf(asm_file, "jmp :%s\n", while_exit);

    fprintf(asm_file, "%s:\n", while_exec_buf);

    translate_cond_or_while_body(BackInfo, root->right_child, fnc_id, asm_file);
    CHECK_ERROR;

    fprintf(asm_file, "jmp :%s\n", while_buf);
    fprintf(asm_file, "%s:\n", while_exec_buf);

    return SUCCESS;
}

//=====================================================================================================================

int translate_cond_or_while_body(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file)
{
    CHECK_ERROR;

    switch(root->type)
    {
        case(_EMPTY):
        {
            if(root->left_child != nullptr)
            {
                translate_cond_or_while_body(BackInfo, root->left_child, fnc_id, asm_file);
                CHECK_ERROR;
            }

            if(root->right_child != nullptr)
            {
                translate_cond_or_while_body(BackInfo, root->right_child, fnc_id, asm_file);
                CHECK_ERROR;
            }

            break;
        }

        case(_ASSIGN):
        {
            translate_var_name(BackInfo, root, fnc_id, asm_file);
            CHECK_ERROR;
            break;
        }

        case(_COND_OP):
        {
            BackInfo->if_num++;
            size_t cur_if_num = BackInfo->if_num;
            translate_condition(BackInfo, root, fnc_id, cur_if_num, asm_file);
            CHECK_ERROR;
            fprintf(asm_file, "\nexit_cond_%lu:\n", cur_if_num);
            break;
        }

        case(_LOOP):
        {
            translate_while(BackInfo, root, fnc_id, asm_file);
            CHECK_ERROR;
            break;
        }
        
        case(_FNC_NAME):
        {
            translate_fnc_call(BackInfo, root, fnc_id, 0, asm_file);
            CHECK_ERROR;
            break;
        }
        
        case(_RETURN):
        {
            translate_return(BackInfo, root, fnc_id, asm_file);
            CHECK_ERROR;
            break;
        }
        
        default:
        {
            printf(RED "Unknown type(%d) in translate_while_body!\n" RESET, root->type);
            BackInfo->error = ERROR_TRANSLATE_COND_OR_WHILE;
            return ERROR_TRANSLATE_COND_OR_WHILE;
        }
    }

    return SUCCESS;
}

//=====================================================================================================================

int translate_condition(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, size_t cond_num, FILE* asm_file)
{
    CHECK_ERROR;

    if(root == nullptr)
    {
        return ERROR_TRANSLATE_CONDITION;
    }

    if(root->type == _EMPTY)
    {
        if(root->left_child != nullptr)
        {
            translate_condition(BackInfo, root->left_child, fnc_id, cond_num,  asm_file);
            CHECK_ERROR;
        }
        if(root->right_child != nullptr)
        {
            translate_condition(BackInfo, root->right_child, fnc_id, cond_num, asm_file);
            CHECK_ERROR;
        }
    }

    if(root->type == _COND_OP)
    {
        if(!strcmp(root->val.name, "if"))
        {
            translate_if(BackInfo, root, fnc_id, cond_num, asm_file);
            CHECK_ERROR;
        }

        else if(!strcmp(root->val.name, "elif"))
        {
            translate_elif(BackInfo, root, fnc_id, cond_num, asm_file);
            CHECK_ERROR;
            BackInfo->elif_else_num++;
        }

        else if(!strcmp(root->val.name, "else"))
        {
            translate_else(BackInfo, root, fnc_id, cond_num, asm_file);
            CHECK_ERROR;
            BackInfo->elif_else_num = 1;
        }
    }

    
    return SUCCESS;
}

//=====================================================================================================================

int translate_if(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, size_t cond_num, FILE* asm_file)
{
    CHECK_ERROR;

    if(root->left_child->type != _EMPTY)
    {
        printf(RED "Empty \"if\" translation in translate_if!\n" RESET);
        BackInfo->error = ERROR_EMPTY_IF_TRANSLATION;
        return ERROR_EMPTY_IF_TRANSLATION; 
    }

    fprintf(asm_file, "\n");
    translate_right_value(BackInfo, root->left_child->left_child->left_child,  fnc_id, asm_file);
    CHECK_ERROR;
    translate_right_value(BackInfo, root->left_child->left_child->right_child, fnc_id, asm_file);
    CHECK_ERROR;
    translate_cond_or_while_op(root->left_child->left_child, asm_file);
    CHECK_ERROR;
    fprintf(asm_file, ":if_cond_%lu\n", cond_num);
    if(root->right_child != nullptr)
    {
        fprintf(asm_file, "jmp :next_cond_%lu_%lu\n", cond_num, BackInfo->elif_else_num);
    }
    else
    {
        fprintf(asm_file, "jmp :exit_cond_%lu\n", cond_num);
    }
    fprintf(asm_file, "if_cond_%lu:\n", cond_num);
    translate_cond_or_while_body(BackInfo, root->left_child->right_child, fnc_id, asm_file);
    CHECK_ERROR;
    fprintf(asm_file, "jmp :exit_cond_%lu\n", cond_num);

    translate_condition(BackInfo, root->right_child, fnc_id, cond_num, asm_file);
    CHECK_ERROR;

    return SUCCESS; 
}

//=====================================================================================================================

int translate_elif(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, size_t cond_num, FILE* asm_file)
{
    CHECK_ERROR;

    fprintf(asm_file, "\n");
    fprintf(asm_file, "next_cond_%lu_%lu:\n", cond_num, BackInfo->elif_else_num);

    translate_right_value(BackInfo, root->left_child->left_child,  fnc_id, asm_file);
    CHECK_ERROR;
    translate_right_value(BackInfo, root->left_child->right_child, fnc_id, asm_file);
    CHECK_ERROR;
    translate_cond_or_while_op(root->left_child, asm_file);
    CHECK_ERROR;
    fprintf(asm_file, ":elif_cond_%lu_%lu\n", cond_num, BackInfo->elif_else_num);

    if(root->parent->type == _EMPTY && root == root->parent->left_child)
    {
        fprintf(asm_file, "jmp :next_cond_%lu_%lu\n", cond_num, BackInfo->elif_else_num + 1);
    }
    else
    {
        fprintf(asm_file, "jmp :exit_cond_%lu\n", cond_num);
    }

    fprintf(asm_file, "elif_cond_%lu_%lu:\n", cond_num, BackInfo->elif_else_num);
    translate_cond_or_while_body(BackInfo, root->right_child, fnc_id, asm_file);
    CHECK_ERROR;
    fprintf(asm_file, "jmp :exit_cond_%lu\n", cond_num);

    return SUCCESS;
}

//=====================================================================================================================

int translate_else(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, size_t cond_num, FILE* asm_file)
{
    CHECK_ERROR;

    fprintf(asm_file, "\n");
    fprintf(asm_file, "next_cond_%lu_%lu:\n", cond_num, BackInfo->elif_else_num);
    translate_cond_or_while_body(BackInfo, root->right_child, fnc_id, asm_file);
    CHECK_ERROR;
    return SUCCESS;
}

//=====================================================================================================================

#define COMP_COND_OP(string, asm)                   \
        else if(!strcmp(root->val.name, string))    \
        {                                           \
            fprintf(asm_file, "%s ", asm);          \
        }                                           \

int translate_cond_or_while_op(Node* root, FILE* asm_file)
{
    if(0) {}
    COMP_COND_OP("==", "je")
    COMP_COND_OP("!=", "jne")
    COMP_COND_OP("<=", "jle")
    COMP_COND_OP("<",  "jl")
    COMP_COND_OP(">=", "jge")
    COMP_COND_OP(">",  "jg")


    return SUCCESS;
}

//=====================================================================================================================

int hex_to_dec(const char* hex_in_str)
{
    if(hex_in_str == nullptr)
    {
        return ERROR_CONVERTION_HEX_TO_DEC;
    }

    int i = 0, val = 0;
    int decimal = 0;
    size_t len = strlen(hex_in_str) - 1;

    for(i = 0; hex_in_str[i] != '\0'; i++)
    {
        if(hex_in_str[i] >= '0' && hex_in_str[i] <= '9')
        {
            val = hex_in_str[i] - 48;
        }
        else if(hex_in_str[i]>='a' && hex_in_str[i]<='f')
        {
            val = hex_in_str[i] - 97 + 10;
        }
        else if(hex_in_str[i]>='A' && hex_in_str[i]<='F')
        {
            val = hex_in_str[i] - 65 + 10;
        }

        decimal += val * (int)pow(16, len);
        len--;
    }

    return decimal;
}

//=====================================================================================================================

size_t get_cur_var(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id)
{
    CHECK_ERROR;

    size_t cur_var = 0;
    int flag = 0;
    for(; cur_var < BackInfo->fnc_arr[fnc_id].all_vars_num; cur_var++)
    {
        if(!strcmp(BackInfo->fnc_arr[fnc_id].all_vars_arr[cur_var].text, root->val.name))
        {
            flag = 1;
            break;
        }
    }

    if(flag == 0)
    {
        printf(RED "\nThe variable %s wasn't found in the function %s!\n\n" RESET, root->left_child->val.name, BackInfo->fnc_arr[fnc_id].name);
        BackInfo->error = ERROR_UNKNOWN_VARIABLE;
        return ERROR_UNKNOWN_VARIABLE;
    }

    return cur_var;
}

//=====================================================================================================================

size_t get_cur_fnc(struct BackendInfo* BackInfo, Node* root)
{
    CHECK_ERROR;

    size_t cur_fnc = 0;
    int flag = 0;
    for(; cur_fnc < BackInfo->fnc_num; cur_fnc++)
    {
        if(!strcmp(BackInfo->fnc_arr[cur_fnc].name, root->val.name))
        {
            flag = 1;
            break;
        }
    }

    if(flag == 0)
    {
        printf(RED "\nThe \"%s\" function was not found!\n\n" RESET, root->val.name);
        BackInfo->error = ERROR_UNKNOWN_FUNCTION;
        return ERROR_UNKNOWN_FUNCTION;
    }

    return cur_fnc;
}

//=====================================================================================================================

void backend_dtor(struct BackendInfo* BackInfo)
{
    for(size_t i = 0; i < BackInfo->fnc_num; i++)
    {
        free(BackInfo->fnc_arr[i].args_arr);
        free(BackInfo->fnc_arr[i].decl_vars_arr);
        free(BackInfo->fnc_arr[i].all_vars_arr);
    }
    free(BackInfo->fnc_arr);
}
