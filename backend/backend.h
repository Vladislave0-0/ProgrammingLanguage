#ifndef BACKEND_H
#define BACKEND_H

//======================================================

#include "../frontend/InputProcessing.h"
#include "../frontend/Tree.h"

//======================================================

struct VarInfo
{
    char text[MAX_WORD_LENGTH] = {};
    float val = POISON;
};

struct FuncInfo
{
    char name[MAX_WORD_LENGTH] = {};

    size_t args_num      = 0;  
    size_t decl_vars_num = 0;  
    size_t all_vars_num  = 0;

    VarInfo* args_arr      = nullptr;
    VarInfo* decl_vars_arr = nullptr;
    VarInfo* all_vars_arr  = nullptr;
};

struct BackendInfo
{
    char signature[5]  = "Ver1";
    FILE* asm_file     = nullptr;
    TokenInfo* tok_arr = nullptr;
    FuncInfo* fnc_arr  = nullptr;
    Node* root         = nullptr;

    size_t fnc_num       = 0;
    size_t params_num    = 0;
    size_t if_num        = 0;
    size_t elif_else_num = 1;

    int error = 0;                             
};

enum BackendErrors
{
    ERROR_DIFF_BACK_FRONT_VERS    = 32,
    ERROR_VARS_ARR_NULLPTR        = 33,
    ERROR_TRANSLATE_PROGRAM       = 34,
    ERROR_TRANSLATE_FUNC_BODY     = 35,
    ERROR_TRANSLATE_RIGHT_VALUE   = 36,
    ERROR_TRANSLATE_COND_OR_WHILE = 37,
    ERROR_EMPTY_IF_TRANSLATION    = 38,
    ERROR_CONVERTION_HEX_TO_DEC   = 39,
    ERROR_UNKNOWN_VARIABLE        = 40,
    ERROR_UNKNOWN_FUNCTION        = 41,
    ERROR_TRANSLATE_CONDITION     = 42,
};

//======================================================================================================================

int backend_ctor(struct BackendInfo* BackInfo, struct InputInfo* InputInfo, Node* root, const char* filename);

//======================================================================================================================

void fill_back_struct(struct BackendInfo* BackInfo, struct InputInfo* InputInfo, Node* root);

//======================================================================================================================

void open_back_file(struct BackendInfo* BackInfo, const char* filename);

//======================================================================================================================

int translate_tree_to_asm(struct BackendInfo* BackInfo, Node* root, FILE* asm_file);

//======================================================================================================================

int translate_program(struct BackendInfo* BackInfo, Node* root, FILE* asm_file);

//======================================================================================================================

int translate_func_decl(struct BackendInfo* BackInfo, Node* root, FILE* asm_file);

//======================================================================================================================

int translate_func_body(struct BackendInfo* BackInfo, Node* root, const size_t fnc_num, FILE* asm_file);

//======================================================================================================================

int translate_var_decl(struct BackendInfo* BackInfo, Node* root, const size_t fnc_num, FILE* asm_file);

//======================================================================================================================

int translate_var_name(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file);

//======================================================================================================================

int translate_return(struct BackendInfo* BackInfo, Node* root, const size_t fnc_num, FILE* asm_file);

//======================================================================================================================

int translate_right_value(struct BackendInfo* BackInfo, Node* root, const size_t fnc_num, FILE* asm_file);

//======================================================================================================================

int translate_arth_op(Node* root, FILE* asm_file);

//======================================================================================================================

int translate_fnc_call(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, int r_val_flag, FILE* asm_file);

//======================================================================================================================

int translate_fnc_call_params(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file);

//======================================================================================================================

int translate_programm_fnc(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file);

//======================================================================================================================

int translate_printf_scanf(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file);

//======================================================================================================================

int translate_while(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file);

//======================================================================================================================

int translate_cond_or_while_body(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, FILE* asm_file);

//======================================================================================================================

int translate_condition(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, size_t cond_num, FILE* asm_file);

//======================================================================================================================

int translate_if(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, size_t cond_num, FILE* asm_file);

//======================================================================================================================

int translate_elif(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, size_t cond_num, FILE* asm_file);

//======================================================================================================================

int translate_else(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id, size_t cond_num, FILE* asm_file);

//======================================================================================================================

int translate_cond_or_while_op(Node* root, FILE* asm_file);

//======================================================================================================================

size_t get_cur_var(struct BackendInfo* BackInfo, Node* root, const size_t fnc_id);

//======================================================================================================================

size_t get_cur_fnc(struct BackendInfo* BackInfo, Node* root);

//======================================================================================================================

int hex_to_dec(const char* hex_in_str);

//======================================================================================================================

void backend_dtor(struct BackendInfo* BackInfo);

//======================================================================================================================

#endif //BACKEND_H
