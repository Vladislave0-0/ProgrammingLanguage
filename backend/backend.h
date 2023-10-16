#ifndef BACKEND_H
#define BACKEND_H

//================================================================================================

#include "../frontend/InputProcessing.h"
#include "../frontend/Tree.h"

//================================================================================================

struct VarInfo
{
    char text[MAX_WORD_LENGTH] = {};
    float val = POISON;
};

struct FuncInfo
{
    char name[MAX_WORD_LENGTH] = {};

    size_t args_num            = 0;  
    size_t decl_vars_num       = 0;  
    size_t all_vars_num        = 0;

    VarInfo* args_arr          = nullptr;
    VarInfo* decl_vars_arr     = nullptr;
    VarInfo* all_vars_arr      = nullptr;
};

struct BackendInfo
{
    FILE* asm_file     = nullptr;
    TokenInfo* tok_arr = nullptr;
    FuncInfo* fnc_arr  = nullptr;
    Node* root         = nullptr;
    size_t fnc_num     = 0;
    size_t params_num  = 0;


    size_t if_num        = 0;
    size_t elif_else_num = 1;

    int error = 0;                             
};

//================================================================================================

int backend_ctor(struct BackendInfo* BackInfo, struct InputInfo* InputInfo, Node* root, const char* filename);

//================================================================================================

void fill_back_struct(struct BackendInfo* BackInfo, struct InputInfo* InputInfo, Node* root);

//================================================================================================

void open_back_file(struct BackendInfo* BackInfo, const char* filename);

//================================================================================================

int translate_tree_to_asm(struct BackendInfo* BackInfo, Node* root, FILE* asm_file);
int translate_programm(struct BackendInfo* BackInfo, Node* root, FILE* asm_file);
int translate_fnc_decl(struct BackendInfo* BackInfo, Node* root, FILE* asm_file);
int translate_fnc_body(struct BackendInfo* BackInfo, Node* root, const size_t fnc_num, FILE* asm_file);
int translate_var_decl(struct BackendInfo* BackInfo, Node* root, const size_t fnc_num, FILE* asm_file);
int translate_var_name(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file);
int translate_return(struct BackendInfo* BackInfo, Node* root, const size_t fnc_num, FILE* asm_file);
int translate_right_value(struct BackendInfo* BackInfo, Node* root, const size_t fnc_num, FILE* asm_file);
int translate_arth_op(Node* root, FILE* asm_file);
int translate_fnc_call(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file);
int translate_fnc_call_params(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file);
int translate_programm_fnc(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file);
int translate_printf_scanf(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file);
int translate_while(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file);
int translate_cond_while_body(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file);
int translate_condition(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, size_t cond_num, FILE* asm_file);
int translate_if(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, size_t cond_num, FILE* asm_file);
int translate_elif(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, size_t cond_num, FILE* asm_file);
int translate_else(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, size_t cond_num, FILE* asm_file);
int translate_if_while_cond_op(Node* root, FILE* asm_file);
size_t get_cur_var(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id);
size_t get_cur_fnc(struct BackendInfo* BackInfo, Node* root);
int hex_to_int(const char* hex_in_str);

//================================================================================================

#endif //BACKEND_H
