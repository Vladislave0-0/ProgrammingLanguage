#include "./InputProcessing.h"
#include <string.h>
#include <ctype.h>
#include "../include/colors.h"

//=========================================================================================

#define CHECK_ERROR                     \
        if(InputInfo->error != 0)       \
        {                               \
            return InputInfo->error;    \
        }                               \

//=========================================================================================

int text_info_ctor(struct InputInfo* InputInfo, const char* filename)
{
    open_file(InputInfo, filename);
    CHECK_ERROR;
    num_of_chars(InputInfo, filename);
    chars_buffer(InputInfo);

    count_tokens(InputInfo);
    tokenization(InputInfo);
    syntactic_analysis(InputInfo);
    prog_scope_check(InputInfo);
    open_lst_file(InputInfo, filename);
    listing(InputInfo);

    return SUCCESS;
}

#undef CHECK_ERROR

//=========================================================================================

void open_file(struct InputInfo* InputInfo, const char* filename)
{
    FILE* file_input = fopen(filename, "r");

    if(file_input == nullptr)
    {
        printf(RED "\nThere is no file named \"%s\" in the current directory!\n\n" RESET, filename);
        InputInfo->error = ERROR_MAINFILE_OPEN;
        return;
    }

    size_t filename_len = strlen(filename);

    if(filename[filename_len-1] != 't' || filename[filename_len-2] != 'x' ||
       filename[filename_len-3] != 't' || filename[filename_len-4] != '.')
       {
        printf(RED "\nInvalid input file format. Only the \"txt\" format is allowed!\n\n" RESET);
        InputInfo->error = ERROR_MAINFILE_OPEN;
        return;
       }

    InputInfo->mainfile = file_input;
}

//=========================================================================================

void open_lst_file(struct InputInfo* InputInfo, const char* filename)
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
    strcat(path, "lst");
    
    InputInfo->lst_file = fopen(path, "w");
}

//=========================================================================================

void num_of_chars(struct InputInfo* InputInfo, const char* filename)
{
    struct stat buf = {};

    stat(filename, &buf);

    InputInfo->ch_num = buf.st_size + 1;

    // printf("InputInfo->ch_num:\n%lu\n\n", InputInfo->ch_num);
}

//=========================================================================================

void chars_buffer(struct InputInfo* InputInfo)
{
    InputInfo->chars_buff_ptr = (char*)calloc(InputInfo->ch_num, sizeof(char));
    if(InputInfo->chars_buff_ptr == nullptr)
    {
        InputInfo->error = ERROR_CHARS_BUFFER;
        return;
    }
    char* tmp_buff = (char*)calloc(InputInfo->ch_num, sizeof(char));
    if(tmp_buff == nullptr)
    {
        InputInfo->error = ERROR_CHARS_BUFFER;
        return;
    }
    fread(tmp_buff, sizeof(char), InputInfo->ch_num - 1, InputInfo->mainfile);
    fclose(InputInfo->mainfile);

    for(size_t i = 0; i < InputInfo->ch_num; i++)
    {
        if((tmp_buff[i] == '/') && (tmp_buff[i + 1] == '/'))
        {
            while(i < InputInfo->ch_num - 1  && tmp_buff[i] != '\n')
            {
                tmp_buff[i++] = '\0';
            }
        }

        // if((tmp_buff[i] == '\n') && (tmp_buff[i + 1] == '\n'))
        // {
        //     tmp_buff[i++] = '\0';
        // }

        if(((tmp_buff[i] == '\n') && (tmp_buff[i + 1] == ' ')) || 
           ((tmp_buff[i] == ' ')  && (tmp_buff[i + 1] == ' ')))
        {
            i++;
            while(tmp_buff[i] == ' ')
            {
                tmp_buff[i++] = '\0';
            }
        }

        if(tmp_buff[i] == '(')
        {
            i++;
            while(i < InputInfo->ch_num &&  tmp_buff[i] != ')')
            {
                if(tmp_buff[i] == ' ')
                {
                    tmp_buff[i] = '\0';
                }
                i++;
            }
        }
    }

    for(size_t i = 0, j = 0; i < InputInfo->ch_num; i++)
    {
        if(tmp_buff[i] != '\0')
        {
            InputInfo->chars_buff_ptr[j++] = tmp_buff[i];
        }
    }

    free(tmp_buff);
    InputInfo->ch_num = strlen(InputInfo->chars_buff_ptr);
    // printf("%s\n", InputInfo->chars_buff_ptr);
}

//=========================================================================================

void count_tokens(struct InputInfo* InputInfo)
{
    char* cur_ptr  = nullptr;
    char* tmp_buff = (char*)calloc(InputInfo->ch_num + 1, sizeof(char));
    if(tmp_buff == nullptr)
    {
        InputInfo->error = ERROR_TOKS_ARR_CALLOC;
        return;
    }

    for(size_t i = 0, j = 0; i < InputInfo->ch_num; i++)
    {
        if(InputInfo->chars_buff_ptr[i] != ' ' && InputInfo->chars_buff_ptr[i] != '\n')
        {
            tmp_buff[j++] = InputInfo->chars_buff_ptr[i];
        }
    }

    size_t len_tmp_buff = strlen(tmp_buff);
    char* start_ptr     = tmp_buff;
 
    const char* known_tok_arr[MAX_KNOWN_TOKENS] = {"main", "printf", "scanf", "return", "if", 
                                                   "elif", "else", "while", "var", "func",
                                                   "sqrt", "sin", "cos", "tg", "ctg", "arcsin",
                                                   "arccos", "arctg", "arcctg", "sh", "ch", "ln", "exp", 
                                                   "==", "!=", "<=", "<", ">=", ">", "=", 
                                                   "+", "-", "*", "/", "^", 
                                                   "(", ")", "{", "}", 
                                                   ",", ";", "\""                                         };

    for(size_t cur_ch = 0; cur_ch < len_tmp_buff && tmp_buff[cur_ch] != '\0';)
    {
        size_t word_flag = 0;
        
        for(size_t cur_tok = 0; cur_tok < MAX_KNOWN_TOKENS; cur_tok++)
        {
            cur_ptr = strstr(start_ptr + cur_ch, known_tok_arr[cur_tok]);
            if(cur_ptr - (start_ptr + cur_ch) == 0)
            {
                if(tmp_buff[cur_ch + strlen(known_tok_arr[cur_tok])] == '_')
                {
                    word_flag = MAX_KNOWN_TOKENS;
                    break;
                }

                InputInfo->tok_num++;
                cur_ch += strlen(known_tok_arr[cur_tok]);
                break;
            }

            word_flag++;
        }

        if(word_flag == MAX_KNOWN_TOKENS)
        {
            if(cur_ch != 0 && tmp_buff[cur_ch - 1] == '"')
            {
                while(tmp_buff[cur_ch] != '"')
                {
                    cur_ch++;
                }
            }


            while(isalpha(tmp_buff[cur_ch]) || isdigit(tmp_buff[cur_ch]) || tmp_buff[cur_ch] == '_' || tmp_buff[cur_ch] == '.')
            {
                cur_ch++;
            }
            InputInfo->tok_num++;
        }
    }

    free(tmp_buff);

    // printf("%lu\n", InputInfo->tok_num);
}

//=========================================================================================

struct KnownTokenInfo
{
    char text[MAX_WORD_LENGTH] = {0};
    TypeOfToken type;
};

void tokenization(struct InputInfo* InputInfo)
{
    char* tmp_buff = (char*)calloc(InputInfo->ch_num + 1, sizeof(char));
    
    size_t j = 0;
    for(size_t i = 0; i < InputInfo->ch_num; i++)
    {
        if(InputInfo->chars_buff_ptr[i] != ' ')
        {
            tmp_buff[j++] = InputInfo->chars_buff_ptr[i];
        }
    }
    // printf("%s\n", tmp_buff);
    InputInfo->tok_arr = (TokenInfo*)calloc(InputInfo->tok_num, sizeof(TokenInfo));

    struct KnownTokenInfo all_token[MAX_KNOWN_TOKENS] = {{"main",  MAIN},     {"printf", FNC_NAME}, {"scanf",  FNC_NAME}, {"return", RETURN}, 
                                                         {"if",    COND_OP},  {"elif",   COND_OP},  {"else",   COND_OP},  {"while",  LOOP}, 
                                                         {"var",   VAR_DECL}, {"==",     LOG_OP},   {"!=",     LOG_OP},   {"<=",     LOG_OP}, 
                                                         {"<",     LOG_OP},   {">=",     LOG_OP},   {">",      LOG_OP},   {"=",      ASSIGN}, 
                                                         {"+",     ARTH_OP},  {"-",      ARTH_OP},  {"*",      ARTH_OP},  {"/",      ARTH_OP}, 
                                                         {"^",     ARTH_OP},  {"(",      RND_BRC},  {")",      RND_BRC},  {"{",      CRL_BRC}, 
                                                         {"}",     CRL_BRC},  {",",      COMMA},    {";",      SEMICLN},  {"\"",     QUOTE},
                                                         {"func",  FNC_DECL}, {"sqrt",   FNC_NAME}, {"sin",    FNC_NAME}, {"cos",    FNC_NAME},
                                                         {"tg",    FNC_NAME}, {"ctg",    FNC_NAME}, {"arcsin", FNC_NAME}, {"arccos", FNC_NAME},
                                                         {"arctg", FNC_NAME}, {"arcctg", FNC_NAME}, {"sh",     FNC_NAME}, {"ch",     FNC_NAME},
                                                         {"ln",    FNC_NAME}, {"exp",    FNC_NAME}                                              };

    char* cur_ptr   = nullptr;
    size_t cur_tok  = 0;
    size_t cur_line = 1;

    for(size_t cur_ch = 0; cur_ch < strlen(tmp_buff) && tmp_buff[cur_ch] != '\0' && cur_tok < InputInfo->tok_num; cur_tok++)
    {
        while(tmp_buff[cur_ch] == '\n')
        {
            cur_ch++;
            cur_line++;
        }

        size_t word_flag = 0;

        for(size_t cur_known_tok = 0; cur_known_tok < MAX_KNOWN_TOKENS; cur_known_tok++)
        {
            cur_ptr = strstr(tmp_buff + cur_ch, all_token[cur_known_tok].text);
            
            if(cur_ptr - (tmp_buff + cur_ch) == 0)
            {
                if(cur_ch + strlen(all_token[cur_known_tok].text) < strlen(tmp_buff) && isalpha(tmp_buff[cur_ch]) &&(tmp_buff[cur_ch + strlen(all_token[cur_known_tok].text)] == '_' || isalpha(tmp_buff[cur_ch + strlen(all_token[cur_known_tok].text)])) && strcmp(all_token[cur_known_tok].text, "return"))
                {
                    if((tmp_buff[cur_ch] != 'v' && tmp_buff[cur_ch + 1] != 'a' && tmp_buff[cur_ch + 2] != 'r') &&
                       (tmp_buff[cur_ch] != 'f' && tmp_buff[cur_ch + 1] != 'u' && tmp_buff[cur_ch + 2] != 'n' && tmp_buff[cur_ch + 3] != 'c'))
                    {
                        InputInfo->tok_num--;
                        word_flag = MAX_KNOWN_TOKENS;
                        break;
                    }
                }
                // тут различаем унарный минус от отрицательного числа
                if(tmp_buff[cur_ch] == '-' && !isdigit(tmp_buff[cur_ch - 1]) && !isalpha(tmp_buff[cur_ch - 1]))
                {
                    InputInfo->tok_arr[cur_tok].number = (float)atof(tmp_buff + cur_ch);
                    snprintf(InputInfo->tok_arr[cur_tok].text, sizeof(InputInfo->tok_arr[cur_tok].text), "%g", InputInfo->tok_arr[cur_tok].number);
                    cur_ch += strlen(InputInfo->tok_arr[cur_tok].text);
                    InputInfo->tok_arr[cur_tok].line = cur_line;
                    InputInfo->tok_arr[cur_tok].type = NUMBER;
                    InputInfo->tok_num--;
                    break;
                }

                // тут токенизируем '"', потом строковый литерал, и потом снова '"'
                if(tmp_buff[cur_ch] == '"')
                {
                    InputInfo->tok_arr[cur_tok].text[0] = '"';
                    InputInfo->tok_arr[cur_tok].line    = cur_line;
                    InputInfo->tok_arr[cur_tok].type    = QUOTE;
                    cur_ch++; // because of strlen of "\""

                    cur_tok++;
                    size_t strlen  = 0;

                    while(tmp_buff[cur_ch] != '"')
                    {
                        InputInfo->tok_arr[cur_tok].text[strlen] = tmp_buff[cur_ch];
                        strlen++;
                        cur_ch++;
                    }
                    InputInfo->tok_arr[cur_tok].line = cur_line;
                    InputInfo->tok_arr[cur_tok].type = STRING;

                    cur_tok++;
                    if(tmp_buff[cur_ch] == '"')
                    {
                        InputInfo->tok_arr[cur_tok].text[0] = '"';
                        InputInfo->tok_arr[cur_tok].line    = cur_line;
                        InputInfo->tok_arr[cur_tok].type    = QUOTE;
                        cur_ch++; // because of strlen of "\""
                    }

                    break;
                }

                cur_ch += strlen(all_token[cur_known_tok].text);
                strcpy(InputInfo->tok_arr[cur_tok].text, all_token[cur_known_tok].text);
                InputInfo->tok_arr[cur_tok].line = cur_line;
                InputInfo->tok_arr[cur_tok].type = all_token[cur_known_tok].type;
                break;
            }

            word_flag++;
        }

        // токенизируем число или переменную
        if(word_flag == MAX_KNOWN_TOKENS)
        {
            InputInfo->tok_arr[cur_tok].line = cur_line;
            if(isalpha(tmp_buff[cur_ch]))
            {
                if(cur_tok != 0 && InputInfo->tok_arr[cur_tok - 1].type == FNC_DECL)
                {
                    InputInfo->tok_arr[cur_tok].type = FNC_NAME;
                }
                else
                {
                    InputInfo->tok_arr[cur_tok].type = VAR_NAME;
                }
            }

            else if(isdigit(tmp_buff[cur_ch]) || tmp_buff[cur_ch] == '-')
            {
                InputInfo->tok_arr[cur_tok].type = NUMBER;
            }

            int i = 0;
            while(isalpha(tmp_buff[cur_ch]) || isdigit(tmp_buff[cur_ch]) || tmp_buff[cur_ch] == '_' || tmp_buff[cur_ch] == '.')
            {
                InputInfo->tok_arr[cur_tok].text[i] = tmp_buff[cur_ch];
                i++;
                cur_ch++;
            }

            if(InputInfo->tok_arr[cur_tok].type == NUMBER)
            {
                InputInfo->tok_arr[cur_tok].number = (float)atof(InputInfo->tok_arr[cur_tok].text);
            }
        }
    }

    for(size_t i = 0; i + 1 < InputInfo->tok_num; i++)
    {
        if(InputInfo->tok_arr[i].type == VAR_NAME && !strcmp(InputInfo->tok_arr[i + 1].text, "("))
        {
            InputInfo->tok_arr[i].type = FNC_NAME;
        }
    }

    free(tmp_buff);

    int main_cnt = 0;
    for(size_t i = 0; i < InputInfo->tok_num; i++)
    {
        if(InputInfo->tok_arr[i].type == MAIN)
        {
            main_cnt++;
            break;
        }
    }
    if(main_cnt == 0)
    {
        InputInfo->error = ERROR_UNDEF_REF_TO_MAIN;
    }


    // printf("\n\nInputInfo->tok_arr:\n\n");
    // for(size_t i = 0; i < InputInfo->tok_num; i++)
    // {
    //     printf("token[%lu]: %s\n", i + 1, InputInfo->tok_arr[i].text);
    //     printf("type:       %d\n", InputInfo->tok_arr[i].type);
    //     printf("line:       %lu\n", InputInfo->tok_arr[i].line);
    //     printf("error:      %d\n\n", InputInfo->tok_arr[i].error);
    // }
    // printf("\n\n");
}

//=========================================================================================

#define TYPE  InputInfo->tok_arr[cur_tok].type 
#define TEXT  InputInfo->tok_arr[cur_tok].text 
#define ERROR InputInfo->tok_arr[cur_tok].error

#define R_VAL (TYPE == NUMBER   || TYPE == VAR_NAME ||      \
               TYPE == RND_BRC  || TYPE == ARTH_OP  ||      \
               TYPE == FNC_NAME || TYPE == COMMA)           \

#define CHECK_TOK(type, err)                                                            \
        cur_tok++;                                                                      \
        if(cur_tok < InputInfo->tok_num)                                                \
        {                                                                               \
            if(TYPE != type)                                                            \
            {                                                                           \
                errors++;                                                               \
                ERROR = err;                                                            \
            }                                                                           \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            InputInfo->tok_arr[InputInfo->tok_num - 1].error = ERROR_FILE_STRUCTURE;    \
        }                                                                               \

void syntactic_analysis(struct InputInfo* InputInfo)
{
    int errors = 0;
    for(size_t cur_tok = 0; cur_tok < InputInfo->tok_num; cur_tok++)
    {
        switch(InputInfo->tok_arr[cur_tok].type)
        {
            case(VAR_DECL):
            {
                CHECK_TOK(VAR_NAME, ERROR_IN_VAR_INIT_CONSTR);
                CHECK_TOK(ASSIGN, ERROR_IN_VAR_INIT_CONSTR);
                cur_tok++;
                while(R_VAL)
                {
                    cur_tok++;
                }
                cur_tok--;
                CHECK_TOK(SEMICLN, ERROR_IN_VAR_INIT_CONSTR);

                break;
            }
            
            case(FNC_DECL):
            {
                if(InputInfo->tok_arr[cur_tok + 1].type != MAIN)
                {
                    CHECK_TOK(FNC_NAME, ERROR_IN_FNC_DECL_CONSTR);
                    CHECK_TOK(RND_BRC, ERROR_IN_FNC_DECL_CONSTR);
                    cur_tok++;
                    while(TYPE == COMMA || TYPE == VAR_NAME)
                    {
                        cur_tok++;
                    }
                    cur_tok--;
                    CHECK_TOK(RND_BRC, ERROR_IN_FNC_DECL_CONSTR);
                    CHECK_TOK(CRL_BRC, ERROR_IN_FNC_DECL_CONSTR);
                }

                else
                {
                    cur_tok++;
                    CHECK_TOK(RND_BRC, ERROR_IN_FNC_DECL_CONSTR);
                    CHECK_TOK(RND_BRC, ERROR_IN_FNC_DECL_CONSTR);
                    CHECK_TOK(CRL_BRC, ERROR_IN_FNC_DECL_CONSTR);
                }

                break;
            }

            case(COND_OP):
            {
                if(!strcmp(TEXT, "if"))
                {
                    CHECK_TOK(RND_BRC, ERROR_IN_COND_OP_CONSTR);
                    cur_tok++;
                    while(R_VAL)
                    {
                        cur_tok++;
                    }
                    cur_tok--;
                    CHECK_TOK(LOG_OP, ERROR_IN_COND_OP_CONSTR);
                    cur_tok++;
                    while(R_VAL)
                    {
                        cur_tok++;
                    }
                    cur_tok -= 2;
                    CHECK_TOK(RND_BRC, ERROR_IN_COND_OP_CONSTR);
                    CHECK_TOK(CRL_BRC, ERROR_IN_COND_OP_CONSTR);
                }

                else if(!strcmp(TEXT, "elif"))
                {
                    CHECK_TOK(RND_BRC, ERROR_IN_COND_OP_CONSTR);
                    cur_tok++;
                    while(R_VAL)
                    {
                        cur_tok++;
                    }
                    cur_tok--;
                    CHECK_TOK(LOG_OP, ERROR_IN_COND_OP_CONSTR);
                    cur_tok++;
                    while(R_VAL)
                    {
                        cur_tok++;
                    }
                    cur_tok -= 2;
                    CHECK_TOK(RND_BRC, ERROR_IN_COND_OP_CONSTR);
                    CHECK_TOK(CRL_BRC, ERROR_IN_COND_OP_CONSTR);
                }

                else if(!strcmp(TEXT, "else"))
                {
                    CHECK_TOK(CRL_BRC, ERROR_IN_COND_OP_CONSTR);
                }

                break;
            }

            case(LOOP):
            {
                CHECK_TOK(RND_BRC, ERROR_IN_LOOP_CONSTR);
                cur_tok++;
                while(R_VAL)
                {
                    cur_tok++;
                }
                cur_tok--;
                CHECK_TOK(LOG_OP, ERROR_IN_LOOP_CONSTR);
                cur_tok++;
                while(R_VAL)
                {
                    cur_tok++;
                }
                cur_tok -= 2;
                CHECK_TOK(RND_BRC, ERROR_IN_LOOP_CONSTR);
                CHECK_TOK(CRL_BRC, ERROR_IN_LOOP_CONSTR);

                break;
            }

            case(VAR_NAME):
            {
                CHECK_TOK(ASSIGN, ERROR_IN_VAR_NAME_CONSTR);
                cur_tok++;
                while(R_VAL)
                {
                    cur_tok++;
                }
                cur_tok--;
                CHECK_TOK(SEMICLN, ERROR_IN_VAR_NAME_CONSTR);

                break;
            }

            case(FNC_NAME):
            {
                if(!strcmp(TEXT, "printf"))
                {
                    CHECK_TOK(RND_BRC, ERROR_IN_FNC_NAME_CONSTR);
                    cur_tok++;
                    if(TYPE == QUOTE)
                    {
                        CHECK_TOK(STRING, ERROR_IN_FNC_NAME_CONSTR);
                        CHECK_TOK(QUOTE, ERROR_IN_FNC_NAME_CONSTR);
                    }

                    else
                    {
                        cur_tok++;
                        while(R_VAL)
                        {
                            cur_tok++;
                        }
                        cur_tok -= 2;
                    }
                    CHECK_TOK(RND_BRC, ERROR_IN_FNC_NAME_CONSTR);
                    CHECK_TOK(SEMICLN, ERROR_IN_FNC_NAME_CONSTR);

                    break;
                }

                else if(!strcmp(TEXT, "scanf"))
                {
                    CHECK_TOK(RND_BRC, ERROR_IN_FNC_NAME_CONSTR);
                    CHECK_TOK(VAR_NAME, ERROR_IN_FNC_NAME_CONSTR);
                    CHECK_TOK(RND_BRC, ERROR_IN_FNC_NAME_CONSTR);
                    CHECK_TOK(SEMICLN, ERROR_IN_FNC_NAME_CONSTR);

                    break;
                }

                else if(!strcmp(TEXT, "sqrt")   || !strcmp(TEXT, "sin")   || !strcmp(TEXT, "cos")    ||
                        !strcmp(TEXT, "tg")     || !strcmp(TEXT, "ctg")   || !strcmp(TEXT, "arcsin") ||
                        !strcmp(TEXT, "arccos") || !strcmp(TEXT, "arctg") || !strcmp(TEXT, "arcctg") ||
                        !strcmp(TEXT, "sh")     || !strcmp(TEXT, "ch")    || !strcmp(TEXT, "ln")     ||
                        !strcmp(TEXT, "exp"))
                {
                    CHECK_TOK(RND_BRC, ERROR_IN_FNC_NAME_CONSTR);
                    cur_tok++;
                    while(R_VAL)
                    {
                        cur_tok++;
                    }
                    cur_tok--;
                    CHECK_TOK(RND_BRC, ERROR_IN_FNC_NAME_CONSTR);
                    CHECK_TOK(SEMICLN, ERROR_IN_FNC_NAME_CONSTR);

                    break;
                } 

                else
                {
                    CHECK_TOK(RND_BRC, ERROR_IN_FNC_NAME_CONSTR);
                    cur_tok++;
                    while(R_VAL || TYPE == COMMA)
                    {
                        cur_tok++;
                    }
                    cur_tok -= 2;
                    CHECK_TOK(RND_BRC, ERROR_IN_FNC_NAME_CONSTR);
                    CHECK_TOK(SEMICLN, ERROR_IN_FNC_NAME_CONSTR);

                    break;
                }
            }

            case(RETURN):
            {
                cur_tok++;
                if(TYPE == FNC_NAME && (!strcmp(TEXT, "printf") || !strcmp(TEXT, "scanf") || 
                                        !strcmp(TEXT, "sqrt")  || !strcmp(TEXT, "sin") || 
                                        !strcmp(TEXT, "scanf") || !strcmp(TEXT, "scanf") || 
                                        !strcmp(TEXT, "scanf") || !strcmp(TEXT, "scanf") || 
                                        !strcmp(TEXT, "scanf") || !strcmp(TEXT, "scanf") || 
                                        !strcmp(TEXT, "scanf") || !strcmp(TEXT, "scanf") || 
                                        !strcmp(TEXT, "scanf") || !strcmp(TEXT, "scanf") || 
                                        !strcmp(TEXT, "scanf")))
                {
                    ERROR = ERROR_IN_RETURN_CONSTR;

                    break;
                }
                while(R_VAL)
                {
                    cur_tok++;
                }
                cur_tok--;
                CHECK_TOK(SEMICLN, ERROR_IN_RETURN_CONSTR);

                break;
            }

            default:
            {
                if(TYPE == ASSIGN || TYPE == ARTH_OP || TYPE == LOG_OP || TYPE == NUMBER || TYPE == RND_BRC || TYPE == CRL_BRC || TYPE == COMMA || TYPE == SEMICLN || TYPE == QUOTE || TYPE == STRING)
                {
                    break;
                }

                printf(RED "\n\n Unknown token or token sequence in syntactic_analysis: %s\n\n" RESET, TEXT);
                break;
            }
        }
    }

    int rnd_brc_num = 0, crl_brc_num = 0, quote_num = 0;
    size_t last_rnd = 0, last_crl = 0, last_quote = 0;

    for(size_t cur_tok = 0; cur_tok < InputInfo->tok_num; cur_tok++)
    {
        if(TYPE == RND_BRC || TYPE == CRL_BRC || TYPE == QUOTE)
        {
            switch(TYPE)
            {
                case(RND_BRC):
                {
                    if(!strcmp(TEXT, "("))
                    {
                        rnd_brc_num++;
                    }
                    else
                    {
                        rnd_brc_num--;
                    }
                    last_rnd = cur_tok;

                    if(rnd_brc_num < 0)
                    {
                        ERROR = ERROR_BRACKET_STRUCTURE;
                    }
                    break;
                }

                case(CRL_BRC):
                {
                    if(!strcmp(TEXT, "{"))
                    {
                        crl_brc_num++;
                    }
                    else
                    {
                        crl_brc_num--;
                    }
                    last_crl = cur_tok;

                    if(crl_brc_num < 0)
                    {
                        ERROR = ERROR_BRACKET_STRUCTURE;
                    }
                    break;
                }

                case(QUOTE):
                {
                    quote_num++;
                    last_quote = cur_tok;
                    break;
                }

                default:
                {
                    break;
                }
            }
        }
    }

    if(rnd_brc_num != 0)
    {
        InputInfo->tok_arr[last_rnd].error = ERROR_BRACKET_STRUCTURE;
    }
    if(crl_brc_num != 0)
    {
        InputInfo->tok_arr[last_crl].error = ERROR_BRACKET_STRUCTURE;
    }
    if(quote_num % 2 != 0)
    {
        InputInfo->tok_arr[last_quote].error = ERROR_QUOTE_STRUCTURE;
    }
}

#undef R_VAL
#undef CHECK_TOK

//=========================================================================================

#define LINE InputInfo->tok_arr[cur_tok].line

void listing(struct InputInfo* InputInfo)
{
    fprintf(InputInfo->lst_file, "signature: %s\n", InputInfo->signature);
    size_t total_errors = 0;
    for(size_t cur_tok = 0; cur_tok < InputInfo->tok_num; cur_tok++)
    {
        if(InputInfo->tok_arr[cur_tok].error != 0)
        {
            total_errors++;
        }
    }
    fprintf(InputInfo->lst_file, "total errors: %lu\n", total_errors);
    fprintf(InputInfo->lst_file, "quantity: %lu\n\n", InputInfo->tok_num);

    fprintf(InputInfo->lst_file, "|  ip  |\t\tCOMMAND\t\t\t|\t\tTYPE\t\t|\t\tLINE\t\t|\t\tERROR\n");
    for(size_t cur_tok = 0; cur_tok < InputInfo->tok_num; cur_tok++)
    {
        fprintf(InputInfo->lst_file, "|%06lu|\t\t", cur_tok);
        fprintf(InputInfo->lst_file, "%-16s|\t", TEXT);
        
        switch(TYPE)
        {
            case(VAR_DECL):
            {
                fprintf(InputInfo->lst_file, "  var_decl\t\t|\t\t");
                break;
            }
            case(FNC_DECL):
            {
                fprintf(InputInfo->lst_file, "  fnc_decl\t\t|\t\t");
                break;
            }
            case(ASSIGN):
            {
                fprintf(InputInfo->lst_file, "  assign\t\t|\t\t");
                break;
            }
            case(ARTH_OP):
            {
                fprintf(InputInfo->lst_file, "  arth_op\t\t|\t\t");
                break;
            }
            case(LOG_OP):
            {
                fprintf(InputInfo->lst_file, "  log_op\t\t|\t\t");
                break;
            }
            case(COND_OP):
            {
                fprintf(InputInfo->lst_file, "  conp_op\t\t|\t\t");
                break;
            }
            case(LOOP):
            {
                fprintf(InputInfo->lst_file, "  loop  \t\t|\t\t");
                break;
            }
            case(NUMBER):
            {
                fprintf(InputInfo->lst_file, "  number\t\t|\t\t");
                break;
            }
            case(VAR_NAME):
            {
                fprintf(InputInfo->lst_file, "  var_name\t\t|\t\t");
                break;
            }
            case(FNC_NAME):
            {
                fprintf(InputInfo->lst_file, "  fnc_name\t\t|\t\t");
                break;
            }
            case(MAIN):
            {
                fprintf(InputInfo->lst_file, "  main  \t\t|\t\t");
                break;
            }
            case(RETURN):
            {
                fprintf(InputInfo->lst_file, "  return\t\t|\t\t");
                break;
            }
            case(RND_BRC):
            {
                fprintf(InputInfo->lst_file, "  rnd_brc\t\t|\t\t");
                break;
            }
            case(CRL_BRC):
            {
                fprintf(InputInfo->lst_file, "  crl_brc\t\t|\t\t");
                break;
            }
            case(COMMA):
            {
                fprintf(InputInfo->lst_file, "  comma\t\t\t|\t\t");
                break;
            }
            case(SEMICLN):
            {
                fprintf(InputInfo->lst_file, "  semicln\t\t|\t\t");
                break;
            }
            case(QUOTE):
            {
                fprintf(InputInfo->lst_file, "  quote  \t\t|\t\t");
                break;
            }
            case(STRING):
            {
                fprintf(InputInfo->lst_file, "  string\t\t|\t\t");
                break;
            }

            default:
            {
                printf(RED "\n\n Unknown token or token sequence in listing: %s\n\n" RESET, TEXT);
                break;
            }
        }

        fprintf(InputInfo->lst_file, " %-4lu\t\t|\t\t", LINE);
        fprintf(InputInfo->lst_file, "  %d\n", ERROR);
    }

    fclose(InputInfo->lst_file);
}

#undef LINE

//=========================================================================================

#define FNC_ARR InputInfo->fnc_arr

void prog_scope_check(struct InputInfo* InputInfo)
{
    size_t fnc_decl_num = 0;
    for(size_t cur_tok = 0; cur_tok < InputInfo->tok_num; cur_tok++)
    {
        if(TYPE == FNC_DECL)
        {
            fnc_decl_num++;
        }
    }

    InputInfo->fnc_num = fnc_decl_num;
    InputInfo->fnc_arr = (FunctionInfo*)calloc(fnc_decl_num, sizeof(FunctionInfo));

    for(size_t cur_tok = 0, cur_fnc = 1; cur_tok < InputInfo->tok_num; cur_tok++)
    {
        if(TYPE == FNC_DECL)
        {
            cur_tok++;

            if(TYPE != MAIN)
            {
                if(InputInfo->fnc_num == 1)
                {
                    cur_fnc--;
                }
                
                strcpy(FNC_ARR[cur_fnc].name, TEXT);
                FNC_ARR[cur_fnc].token_num = cur_tok;
                cur_tok += 2;

                size_t cur_arg = 0;
                if(strcmp(TEXT, ")"))
                {
                    strcpy(FNC_ARR[cur_fnc].args_arr[cur_arg++], TEXT);
                    cur_tok++;
                    while(TYPE == COMMA)
                    {
                        cur_tok++;
                        strcpy(FNC_ARR[cur_fnc].args_arr[cur_arg++], TEXT);
                        cur_tok++;
                    }
                }
                cur_tok++;
                FNC_ARR[cur_fnc].args_num = cur_arg;

                fill_fnc_vars(InputInfo, cur_fnc, cur_tok);
            }

            else
            {
                strcpy(FNC_ARR[0].name, TEXT);
                FNC_ARR[0].token_num = cur_tok;
                cur_tok += 3;

                fill_fnc_vars(InputInfo, 0, cur_tok);
            }

            cur_fnc++;
        }
    }

    // редекларация переменных внутри функций
    for(size_t i = 0; i < fnc_decl_num; i++)
    {
        size_t args_num      = InputInfo->fnc_arr[i].args_num;
        size_t vars_num      = InputInfo->fnc_arr[i].vars_num;
        size_t glob_vars_num = InputInfo->fnc_arr[i].glob_vars_num;
        size_t all_vars_num  = args_num + vars_num + glob_vars_num;
        char** some_arr = (char**)calloc(all_vars_num, sizeof(char*));
        size_t j = 0;

        for(size_t z = 0; z < args_num; j++, z++)
        {
            some_arr[j] = InputInfo->fnc_arr[i].args_arr[z];
        }
        for(size_t z = 0; z < vars_num; j++, z++)
        {
            some_arr[j] = InputInfo->fnc_arr[i].decl_vars[z];
        }
        for(size_t z = 0; z < glob_vars_num; j++, z++)
        {
            some_arr[j] = InputInfo->fnc_arr[i].glob_vars[z];
        }

        int vars_conflict_flag = 0;
        for(size_t k = 0; k < j - glob_vars_num; k++)
        {
            if(vars_conflict_flag != 0)
            {
                InputInfo->tok_arr[InputInfo->fnc_arr[i].token_num].error = ERROR_CONFLICT_VAR_DECL;
                break;
            }

            for(size_t m = k + 1; m < j; m++)
            {
                if(!strcmp(some_arr[k], some_arr[m]))
                {
                    vars_conflict_flag = 1;
                    break;
                }
            }
        }

        free(some_arr);
    }

    // int scanf_arg_flag = 0;
    // проверки корректности scanf
    // for(size_t cur_tok = 0; cur_tok < InputInfo->tok_num; cur_tok++)
    // {
    //     if(!strcmp(TEXT, "scanf"))
    //     {
    //         size_t scanf_tok = cur_tok;
    //         cur_tok += 2;
    //         for(size_t j = 0; j < InputInfo->fnc_arr[0].glob_vars_num; j++)
    //         {
    //             if(!strcmp(TEXT, InputInfo->fnc_arr[0].glob_vars[j]))
    //             {
    //                 scanf_arg_flag = 1;
    //             }
    //         }

    //         if(scanf_arg_flag == 0)
    //         {
    //             InputInfo->tok_arr[scanf_tok].error = ERROR_SCANF_UNKNOWN_ARG;
    //         }

    //         scanf_arg_flag = 0;
    //     }
    // }

    // редекларация функций
    int fnc_conflict_flag = 0;
    for(size_t k = 0; k < fnc_decl_num; k++)
    {
        if(fnc_conflict_flag != 0)
        {
            InputInfo->tok_arr[InputInfo->fnc_arr[k].token_num].error = ERROR_FUNC_OVERLOAD;
            break;
        }

        for(size_t m = k + 1; m < fnc_decl_num; m++)
        {
            if(!strcmp(InputInfo->fnc_arr[k].name, InputInfo->fnc_arr[m].name))
            {
                fnc_conflict_flag = 1;
                break;
            }
        }
    }

    // редекларация глобальных переменных
    size_t glob_vars_num = 0;
    int crl_brc_num = 0;
    for(size_t cur_tok = 0; cur_tok < InputInfo->tok_num; cur_tok++)
    {
        if(!strcmp(TEXT, "{"))
        {
            crl_brc_num++;
            continue;
        }
        if(!strcmp(TEXT, "}"))
        {
            crl_brc_num--;
            continue;
        }

        if(TYPE == VAR_DECL)
        {
            cur_tok++;
            if(crl_brc_num == 0)
            {
                glob_vars_num++;
            }
            cur_tok++;
        }
    }
    char** glob_vars_arr = (char**)calloc(glob_vars_num, sizeof(char*));
    size_t* glob_vars_toks = (size_t*)calloc(glob_vars_num, sizeof(size_t));
    for(size_t cur_tok = 0, cur_glob_var = 0; cur_tok < InputInfo->tok_num; cur_tok++)
    {
        if(!strcmp(TEXT, "{"))
        {
            crl_brc_num++;
            continue;
        }
        if(!strcmp(TEXT, "}"))
        {
            crl_brc_num--;
            continue;
        }

        if(TYPE == VAR_DECL && crl_brc_num == 0)
        {
            cur_tok++;
            glob_vars_toks[cur_glob_var] = cur_tok;
            glob_vars_arr[cur_glob_var++] = TEXT;
            cur_tok++;
        }
    }
    int glob_var_conflict_flag = 0;
    for(size_t k = 0; k < glob_vars_num; k++)
    {
        size_t m = k + 1;
        for(; m < glob_vars_num; m++)
        {
            if(!strcmp(glob_vars_arr[k], glob_vars_arr[m]))
            {
                glob_var_conflict_flag = 1;
                break;
            }
        }

        if(glob_var_conflict_flag != 0)
        {
            InputInfo->tok_arr[glob_vars_toks[m]].error = ERROR_CONFLICT_VAR_DECL;
            break;
        }
    }

    

    free(glob_vars_toks);
    free(glob_vars_arr);
}

//=========================================================================================

void fill_fnc_vars(struct InputInfo* InputInfo, size_t fnc_num, size_t crl_brc_tok)
{
    int crl_brc_num = 0;
    size_t cur_tok = crl_brc_tok;

    if(!strcmp(TEXT, "{"))
    {
        crl_brc_num++;
        cur_tok++;
    }

    for(; crl_brc_num > 0; cur_tok++)
    {
        if(!strcmp(TEXT, "{"))
        {
            crl_brc_num++;
            continue;
        }
        if(!strcmp(TEXT, "}"))
        {
            crl_brc_num--;
            continue;
        }

        if(TYPE == VAR_DECL)
        {
            cur_tok++;
            if(crl_brc_num > 1)
            {
                ERROR = ERROR_INVALID_VAR_DECL;
            }
            strcpy(FNC_ARR[fnc_num].decl_vars[FNC_ARR[fnc_num].vars_num++], TEXT);
            cur_tok++;
        }
    }

    crl_brc_num = 0;
    for(cur_tok = 0; cur_tok < crl_brc_tok; cur_tok++)
    {
        if(!strcmp(TEXT, "{"))
        {
            crl_brc_num++;
            continue;
        }
        if(!strcmp(TEXT, "}"))
        {
            crl_brc_num--;
            continue;
        }

        if(crl_brc_num == 0 && TYPE == VAR_DECL)
        {
            cur_tok++;
            strcpy(FNC_ARR[fnc_num].glob_vars[FNC_ARR[fnc_num].glob_vars_num++], TEXT);
        }
    }

    // printf("IN %s:\n", FNC_ARR[fnc_num].name);
    // printf("ARGS\n");
    // for(size_t i = 0; i < FNC_ARR[fnc_num].args_num; i++)
    // {
    //     printf("%ld --- %s\n", i, FNC_ARR[fnc_num].args_arr[i]);
    // }
    // printf("\n");
    // printf("DECL_VARS\n");
    // for(size_t i = 0; i < FNC_ARR[fnc_num].vars_num; i++)
    // {
    //     printf("%ld --- %s\n", i, FNC_ARR[fnc_num].decl_vars[i]);
    // }
    // printf("\n");
    // printf("GLOB_VARS\n");
    // for(size_t i = 0; i < FNC_ARR[fnc_num].glob_vars_num; i++)
    // {
    //     printf("%ld --- %s\n", i, FNC_ARR[fnc_num].glob_vars[i]);
    // }
    // printf("\n\n");
}

#undef FNC_ARR
#undef TEXT
#undef TYPE
#undef ERROR

//=========================================================================================

void prog_dtor(struct InputInfo* InputInfo)
{
    // printf("HUI\n");
    int errors_num = 0;
    for(size_t i = 0; i < InputInfo->tok_num; i++)
    {
        if(InputInfo->tok_arr[i].error != 0)
        {
            errors_num++;
        }
    }

    if(errors_num == SUCCESS && InputInfo->error != SUCCESS)
    {
        errors_num++;
    }
    
    free(InputInfo->tok_arr);
    free(InputInfo->chars_buff_ptr);

    if(errors_num == 0)
    {
        printf(GRN "\nThe program was executed successfully!\n\n" RESET);
    }
    else
    {
        printf(RED "\nThe program was executed with %d errors!\n\n" RESET, errors_num);
    }
}
