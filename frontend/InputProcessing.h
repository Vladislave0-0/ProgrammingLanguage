#ifndef INPUT_PROCESSING_H
#define INPUT_PROCESSING_H

//================================================================================================

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>   
#include <string.h>

const int MAX_WORD_LENGTH    = 32;
const float POISON           = (float)0xDEADDED;
const int MAX_KNOWN_TOKENS   = 42;
const int MAX_FUNC_ARGS      = 5;
const int MAX_FUNC_VARS      = 20;
const int MAX_FUNC_GLOB_VARS = 10;

//================================================================================================

enum TypeOfToken
{
    UNKNOWN  = 0,

    VAR_DECL = 1,          // инициализация переменной           //| var
    FNC_DECL = 2,          // декларация функции                 //| func

    ASSIGN   = 3,          // присваивание                       //| =
    ARTH_OP  = 4,          // арифметический оператор            //| + - * / ^ 
    LOG_OP   = 5,          // логический оператор                //| == != < <= > >=
    COND_OP  = 6,          // условный оператор                  //| if elif else
    LOOP     = 7,          // цикл while                         //| while

    NUMBER   = 8,          // число типа float                   //| -1.488

    VAR_NAME = 9,          // имя переменной                     //| my_var
    FNC_NAME = 10,         // имя функции                        //| my_func
    MAIN     = 11,         // специальное слово main             //| main
    RETURN   = 12,         // специальное слово return           //| return

    RND_BRC  = 13,         // круглая  скобка                    //| ( )
    CRL_BRC  = 14,         // фигурная скобка                    //| { }
    
    COMMA    = 15,         // запятая                            //| ,
    SEMICLN  = 16,         // точка запятая                      //| ;
    QUOTE    = 17,         // кавычки                            //| "

    STRING   = 18,         // строковый литерал                  //| ya ebal v rot eto realizovivat'
};

struct TokenInfo
{
    char text[MAX_WORD_LENGTH] = {0};
    TypeOfToken type           = UNKNOWN;
    float number               = POISON;

    size_t line = 0;
    int error   = 0;
};

struct FunctionInfo
{
    char name[MAX_WORD_LENGTH]                          = {0};
    size_t token_num                                    = 0;  
    size_t args_num                                     = 0;  
    size_t vars_num                                     = 0;  
    size_t glob_vars_num                                = 0;
    char args_arr[MAX_FUNC_ARGS][MAX_WORD_LENGTH]       = {0};
    char decl_vars[MAX_FUNC_VARS][MAX_WORD_LENGTH]      = {0};
    char glob_vars[MAX_FUNC_GLOB_VARS][MAX_WORD_LENGTH] = {0}; 
};

struct InputInfo
{
    char* chars_buff_ptr  = nullptr;
    FILE* mainfile        = nullptr;
    FILE* lst_file        = nullptr;
    TokenInfo* tok_arr    = nullptr;
    FunctionInfo* fnc_arr = nullptr;
    char signature[5]     = "Ver1";

    size_t ch_num  = 0;
    size_t tok_num = 0;    
    size_t fnc_num = 0;

    int error = 0;                             
};

enum InputProcessingErrors
{
    SUCCESS                  = 0,
    ERROR_CMD_LINE_ARGS      = 1,
    ERROR_MAINFILE_OPEN      = 2,
    ERROR_CHARS_BUFFER       = 3,
    ERROR_TOKS_ARR_CALLOC    = 4,
    ERROR_IN_VAR_INIT_CONSTR = 5,
    ERROR_IN_FNC_DECL_CONSTR = 6,
    ERROR_IN_COND_OP_CONSTR  = 7, 
    ERROR_IN_LOOP_CONSTR     = 8,
    ERROR_IN_VAR_NAME_CONSTR = 9,
    ERROR_IN_FNC_NAME_CONSTR = 10,
    ERROR_IN_RETURN_CONSTR   = 11,
    ERROR_FILE_STRUCTURE     = 12,
    ERROR_BRACKET_STRUCTURE  = 13,
    ERROR_QUOTE_STRUCTURE    = 14,
    ERROR_INVALID_VAR_DECL   = 15,
    ERROR_CONFLICT_VAR_DECL  = 16,
    ERROR_FUNC_OVERLOAD      = 17,
    ERROR_UNDEF_REF_TO_MAIN  = 18,
    ERROR_SCANF_UNKNOWN_ARG  = 19,
};

//================================================================================================

int text_info_ctor(struct InputInfo* InputInfo, const char* filename);

//================================================================================================

void open_file(struct InputInfo* InputInfo, const char* filename);

//================================================================================================

void open_lst_file(struct InputInfo* InputInfo, const char* filename);

//================================================================================================

void num_of_chars(struct InputInfo* InputInfo, const char* filename);

//================================================================================================

void chars_buffer(struct InputInfo* InputInfo);

//================================================================================================

void count_tokens(struct InputInfo* InputInfo);

//================================================================================================

void tokenization(struct InputInfo* InputInfo);

//================================================================================================

void syntactic_analysis(struct InputInfo* InputInfo);

//================================================================================================

void prog_scope_check(struct InputInfo* InputInfo);

//================================================================================================

void listing(struct InputInfo* InputInfo);

//================================================================================================

void fill_fnc_vars(struct InputInfo* InputInfo, size_t fnc_num, size_t crl_brc_tok);

//================================================================================================

void prog_dtor(struct InputInfo* InputInfo);

#endif // INPUT_PROCESSING_H
