#ifndef INPUT_PROCESSING_H
#define INPUT_PROCESSING_H

//================================================================================================

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>   
#include <string.h>

const int MAX_WORD_LENGTH  = 32;
const float POISON         = (float)0xDEADDED;
const int MAX_KNOWN_TOKENS = 42;

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

struct TextInfo
{
    char* chars_buff_ptr = nullptr;
    FILE* mainfile       = nullptr;
    FILE* lst_file       = nullptr;
    TokenInfo* tok_arr   = nullptr;
    
    char signature[5] = "Ver1";

    size_t ch_num  = 0;
    size_t tok_num = 0;    

    int error = 0;                             
};

struct KnownTokenInfo
{
    char text[MAX_WORD_LENGTH] = {0};
    TypeOfToken type;
};

enum InputProcessingErrors
{
    ERROR_MAIN_FILE_OPEN   = 1,
    ERROR_CHARS_BUFFER     = 2,
    ERROR_COUNT_TOKENS     = 3,
    ERROR_VAR_INIT         = 4,
    ERROR_FNC_DECL         = 5,
    ERROR_COND_OP          = 6, 
    ERROR_LOOP             = 7,
    ERROR_VAR_NAME         = 8,
    ERROR_MAIN             = 9,
    ERROR_FNC_NAME         = 10,
    ERROR_RETURN           = 11, 
    ERROR_END_OF_PROG      = 12,
    ERROR_NO_MAIN          = 13, 
    ERROR_DOUBLE_MAIN      = 14,
    ERROR_NO_PREV_FNC_DECL = 15,
    ERROR_END_OF_THE_FILE  = 16,
    ERROR_BAD_FIRST_TOK    = 17,
};

//================================================================================================

void text_info_ctor(struct TextInfo* TextInfo, const char* filename);

//================================================================================================

void open_file(struct TextInfo* TextInfo, const char* filename);

//================================================================================================

void num_of_chars(struct TextInfo* TextInfo, const char* filename);

//================================================================================================

void chars_buffer(struct TextInfo* TextInfo);

//================================================================================================

void count_tokens(struct TextInfo* TextInfo);

//================================================================================================

void tokenization(struct TextInfo* TextInfo);

//================================================================================================

void syntactic_analysis(struct TextInfo* TextInfo);

//================================================================================================

void listing(struct TextInfo* TextInfo);

//================================================================================================

#endif // INPUT_PROCESSING_H
