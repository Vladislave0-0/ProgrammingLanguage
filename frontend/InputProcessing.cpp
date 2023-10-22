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
    CHECK_ERROR;

    count_tokens(InputInfo);
    CHECK_ERROR;
    tokenization(InputInfo);
    CHECK_ERROR;
    if(!syntactic_analysis(InputInfo))
    {
        return !SUCCESS;
    }
    if(check_prog_scope(InputInfo))
    {
        return !SUCCESS;
    }
    open_lst_file(InputInfo, filename);
    listing(InputInfo);

    return SUCCESS;
}

#undef CHECK_ERROR

//=========================================================================================

int terminal_processing(int argc, char** argv, const char** filename)
{
    if(argc > 2)
    {
        printf(RED "\nToo many command line arguments!\n\n" RESET);
        return ERROR_CMD_LINE_ARGS;
    }

    if(argc < 2)
    {
        *filename = "source.txt";
    }
    else if(argc == 2)
    {
        *filename = argv[1];
    }

    return SUCCESS;
}


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
}

//=========================================================================================

int chars_buffer(struct InputInfo* InputInfo)
{
    InputInfo->chars_buff_ptr = (char*)calloc(InputInfo->ch_num, sizeof(char));
    if(InputInfo->chars_buff_ptr == nullptr)
    {
        InputInfo->error = ERROR_CHARS_BUFFER;
        return ERROR_CHARS_BUFFER;
    }
    char* tmp_buff = (char*)calloc(InputInfo->ch_num, sizeof(char));
    if(tmp_buff == nullptr)
    {
        InputInfo->error = ERROR_CHARS_BUFFER;
        return ERROR_CHARS_BUFFER;
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
    
    return SUCCESS;
}

//=========================================================================================

int count_tokens(struct InputInfo* InputInfo)
{
    char* cur_ptr  = nullptr;
    char* tmp_buff = (char*)calloc(InputInfo->ch_num + 1, sizeof(char));
    if(tmp_buff == nullptr)
    {
        InputInfo->error = ERROR_TOKS_ARR_CALLOC;
        return ERROR_TOKS_ARR_CALLOC;
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

    return SUCCESS;
}

//=========================================================================================

struct KnownTokenInfo
{
    char text[MAX_WORD_LENGTH] = {0};
    TypeOfToken type;
};

int tokenization(struct InputInfo* InputInfo)
{
    char* tmp_buff = (char*)calloc(InputInfo->ch_num + 1, sizeof(char));
    if(tmp_buff == nullptr)
    {
        InputInfo->error = ERROR_TOKENIZATION_BUF;
        return ERROR_TOKENIZATION_BUF;
    }
    
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

    // printf("\n\nInputInfo->tok_arr:\n\n");
    // for(size_t i = 0; i < InputInfo->tok_num; i++)
    // {
    //     printf("token[%lu]: %s\n", i + 1, InputInfo->tok_arr[i].text);
    //     printf("type:       %d\n", InputInfo->tok_arr[i].type);
    //     printf("line:       %lu\n", InputInfo->tok_arr[i].line);
    //     printf("error:      %d\n\n", InputInfo->tok_arr[i].error);
    // }
    // printf("\n\n");

    return SUCCESS;
}

//=========================================================================================

#define TYPE  InputInfo->tok_arr[cur_tok].type 
#define TEXT  InputInfo->tok_arr[cur_tok].text 
#define ERROR InputInfo->tok_arr[cur_tok].error


#define RVAL                                        \
        TYPE == NUMBER  || TYPE == VAR_NAME ||      \
        TYPE == RND_BRC || TYPE == FNC_NAME ||      \
        TYPE == ARTH_OP                             \


#define CHECK_RVAL          \
        while(RVAL)         \
              cur_tok++;    \


#define CHECK_TYPE(type, err, add_tok_flag)                                             \
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
        if(add_tok_flag)                                                                \
            cur_tok++;                                                                  \


#define CHECK_NAME(token, err, add_tok_flag)    \
        if(strcmp(token, TEXT))                 \
            ERROR = err;                        \
        if(add_tok_flag)                        \
            cur_tok++;                          \


int syntactic_analysis(struct InputInfo* InputInfo)
{
    if(check_bracket_sequence(InputInfo))
    {
        return !SUCCESS;
    }

    int errors = 0;
    for(size_t cur_tok = 0; cur_tok < InputInfo->tok_num; cur_tok++)
    {
        switch(InputInfo->tok_arr[cur_tok].type)
        {
            case(VAR_DECL):
            {
                cur_tok++;
                CHECK_TYPE(VAR_NAME, ERROR_IN_VAR_DECL_CONSTR, 1);
                CHECK_TYPE(ASSIGN, ERROR_IN_VAR_DECL_CONSTR, 1);
                CHECK_RVAL;
                CHECK_TYPE(SEMICLN, ERROR_IN_VAR_DECL_CONSTR, 0);

                break;
            }
            
            case(FNC_DECL):
            {
                cur_tok++;

                if(InputInfo->tok_arr[cur_tok].type != MAIN)
                {
                    CHECK_TYPE(FNC_NAME, ERROR_IN_FNC_DECL_CONSTR, 1);
                    CHECK_NAME("(", ERROR_IN_FNC_DECL_CONSTR, 1);
                    if(TYPE == VAR_NAME)
                    {
                        cur_tok++;
                        while(TYPE == COMMA)
                        {
                            cur_tok++;
                            CHECK_TYPE(VAR_NAME, ERROR_IN_FNC_DECL_CONSTR, 1);
                        }
                    }
                    CHECK_NAME(")", ERROR_IN_FNC_DECL_CONSTR, 1);
                    CHECK_NAME("{", ERROR_IN_FNC_DECL_CONSTR, 0);
                }

                else
                {
                    cur_tok++;
                    CHECK_NAME("(", ERROR_IN_MAIN_DECL, 1);
                    CHECK_NAME(")", ERROR_IN_MAIN_DECL, 1);
                    CHECK_NAME("{", ERROR_IN_MAIN_DECL, 0);
                }

                break;
            }

            case(COND_OP):
            {
                if(!strcmp(TEXT, "if"))
                {
                    cur_tok++;
                    CHECK_NAME("(", ERROR_IN_IF_CONSTR, 1);
                    CHECK_RVAL;
                    CHECK_TYPE(LOG_OP, ERROR_IN_IF_CONSTR, 1);
                    CHECK_RVAL;
                    cur_tok--;
                    CHECK_NAME(")", ERROR_IN_IF_CONSTR, 1);
                    CHECK_NAME("{", ERROR_IN_IF_CONSTR, 0);
                }
                else if(!strcmp(TEXT, "elif"))
                {
                    cur_tok++;
                    CHECK_TYPE(RND_BRC, ERROR_IN_ELIF_CONSTR, 1);
                    CHECK_RVAL;
                    CHECK_TYPE(LOG_OP, ERROR_IN_ELIF_CONSTR, 1);
                    CHECK_RVAL;
                    cur_tok--;
                    CHECK_TYPE(RND_BRC, ERROR_IN_ELIF_CONSTR, 1);
                    CHECK_TYPE(CRL_BRC, ERROR_IN_ELIF_CONSTR, 0);
                }
                else if(!strcmp(TEXT, "else"))
                {
                    cur_tok++;
                    CHECK_TYPE(CRL_BRC, ERROR_IN_ELSE_CONSTR, 0);
                }
                else
                {
                    printf(RED "\nUnknown conditional (%s) in syntactic_analysis!\n\n" RESET, TEXT);
                }

                break;
            }

            case(LOOP):
            {
                cur_tok++;
                CHECK_TYPE(RND_BRC, ERROR_IN_LOOP_CONSTR, 1);
                CHECK_RVAL;
                CHECK_TYPE(LOG_OP, ERROR_IN_LOOP_CONSTR, 1);
                CHECK_RVAL;
                cur_tok--;
                CHECK_TYPE(RND_BRC, ERROR_IN_LOOP_CONSTR, 1);
                CHECK_TYPE(CRL_BRC, ERROR_IN_LOOP_CONSTR, 0);

                break;
            }

            case(VAR_NAME):
            {
                cur_tok++;
                CHECK_TYPE(ASSIGN, ERROR_IN_VAR_NAME_CONSTR, 1);
                CHECK_RVAL;
                CHECK_TYPE(SEMICLN, ERROR_IN_VAR_NAME_CONSTR, 0);

                break;
            }

            case(FNC_NAME):
            {
                if(!strcmp(TEXT, "printf"))
                {
                    cur_tok++;
                    CHECK_TYPE(RND_BRC, ERROR_IN_FNC_PRINTF, 1);
                    if(TYPE == QUOTE)
                    {
                        cur_tok++;
                        CHECK_TYPE(STRING, ERROR_IN_FNC_PRINTF, 1);
                        CHECK_TYPE(QUOTE, ERROR_IN_FNC_PRINTF, 1);
                    }
                    else
                    {
                        CHECK_RVAL;
                        cur_tok--;
                    }
                    CHECK_TYPE(RND_BRC, ERROR_IN_FNC_PRINTF, 1);
                    CHECK_TYPE(SEMICLN, ERROR_IN_FNC_PRINTF, 0);
                }

                else if(!strcmp(TEXT, "scanf"))
                {
                    cur_tok++;
                    CHECK_TYPE(RND_BRC, ERROR_IN_FNC_SCANF, 1);
                    CHECK_TYPE(VAR_NAME, ERROR_IN_FNC_SCANF, 1);
                    CHECK_TYPE(RND_BRC, ERROR_IN_FNC_SCANF, 1);
                    CHECK_TYPE(SEMICLN, ERROR_IN_FNC_SCANF, 0);
                }
                else
                {
                    cur_tok++;
                    CHECK_TYPE(RND_BRC, ERROR_IN_FNC_NAME_CONSTR, 1);
                    while(RVAL || TYPE == COMMA)
                    {
                        cur_tok++;
                    }
                    cur_tok--;
                    CHECK_TYPE(RND_BRC, ERROR_IN_FNC_NAME_CONSTR, 1);
                    CHECK_TYPE(SEMICLN, ERROR_IN_FNC_NAME_CONSTR, 0);
                }

                break;
            }

            case(RETURN):
            {
                cur_tok++;
                while(RVAL)
                {
                    if(TYPE == FNC_NAME)
                    {
                        if(!strcmp(TEXT, "scanf") || !strcmp(TEXT, "printf"))
                        {
                            ERROR = ERROR_IN_RETURN_CONSTR;
                        }
                    }
                    cur_tok++;
                }
                CHECK_TYPE(SEMICLN, ERROR_IN_RETURN_CONSTR, 0);

                break;
            }

            default:
            {
                if(TYPE == RND_BRC || TYPE == CRL_BRC)
                {
                    break;
                }

                printf(RED "\n\n Unknown token or token sequence in syntactic_analysis: %s\n\n" RESET, TEXT);
                break;
            }
        }
    }
    if(errors)
    {
        return !SUCCESS;
    }
    if(!check_main(InputInfo))
    {
    

        return !SUCCESS;
    }

    return SUCCESS;
}

#undef RVAL
#undef CHECK_TOK
#undef CHECK_RVAL
#undef CHECK_TYPE
#undef CHECK_NAME

//=========================================================================================

int check_bracket_sequence(struct InputInfo* InputInfo)
{
    int rnd_num = 0, crl_num = 0, quote_num = 0;
    size_t last_rnd = 0, last_crl = 0, last_quote = 0;

    for(size_t cur_tok = 0; cur_tok < InputInfo->tok_num; cur_tok++)
    {
        switch(TYPE)
        {
            case(RND_BRC):
            {
                if(!strcmp(TEXT, "("))
                    rnd_num++;
                else
                    rnd_num--;
                last_rnd = cur_tok;

                if(rnd_num < 0) 
                {
                    ERROR = ERROR_BRACKET_SEQUENSE;
                    return ERROR_BRACKET_SEQUENSE;
                }
                break;
            }

            case(CRL_BRC):
            {
                if(!strcmp(TEXT, "{"))
                    crl_num++;
                else
                    crl_num--;
                last_crl = cur_tok;
                
                if(crl_num < 0)
                {
                    ERROR = ERROR_BRACKET_SEQUENSE;
                    return ERROR_BRACKET_SEQUENSE;
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

    if(rnd_num != 0)
    {
        InputInfo->tok_arr[last_rnd].error = ERROR_BRACKET_SEQUENSE;
        return ERROR_BRACKET_SEQUENSE;
    }
    if(crl_num != 0)
    {
        InputInfo->tok_arr[last_crl].error = ERROR_BRACKET_SEQUENSE;
        return ERROR_BRACKET_SEQUENSE;
    }
    if(quote_num % 2 != 0)
    {
        InputInfo->tok_arr[last_quote].error = ERROR_QUOTE_STRUCTURE;
        return ERROR_QUOTE_STRUCTURE;
    }

    return SUCCESS;
}

//=========================================================================================

int check_main(struct InputInfo* InputInfo)
{
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
        return ERROR_UNDEF_REF_TO_MAIN;
    }
    else if(main_cnt > 1)
    {
        InputInfo->error = ERROR_MAIN_REDEFINITION;
        return ERROR_MAIN_REDEFINITION;
    }

    return SUCCESS;
}

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

int check_prog_scope(struct InputInfo* InputInfo)
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

    if(InputInfo->fnc_arr == nullptr)
    {
        return ERROR_FUNC_ARR_NULLPTR;
    }

    if(check_vars_scope(InputInfo))
    {
        return !SUCCESS;
    }

    if(check_functions(InputInfo))
    {
        return !SUCCESS;
    }

    if(check_vars_redeclaration(InputInfo))
    {
        return !SUCCESS;
    }

    return SUCCESS;
}

//=========================================================================================

int check_vars_redeclaration(struct InputInfo* InputInfo)
{
    for(size_t i = 0; i < InputInfo->fnc_num; i++)
    {
        size_t args_num     = InputInfo->fnc_arr[i].args_num;
        size_t vars_num     = InputInfo->fnc_arr[i].vars_num;
        size_t all_vars_num = args_num + vars_num;
        char** some_arr = (char**)calloc(all_vars_num, sizeof(char*));
        if(some_arr == nullptr)
        {
            InputInfo->error = ERROR_VARS_REDECLARATION_BUFFER;
            return ERROR_VARS_REDECLARATION_BUFFER;
        }
        size_t j = 0;

        for(size_t z = 0; z < args_num; j++, z++)
        {
            some_arr[j] = InputInfo->fnc_arr[i].args_arr[z];
            
        }
        for(size_t z = 0; z < vars_num; j++, z++)
        {
            some_arr[j] = InputInfo->fnc_arr[i].decl_vars[z];
        }

        int vars_conflict_flag = 0;
        for(size_t k = 0; k < j; k++)
        {
            if(vars_conflict_flag != 0)
            {
                InputInfo->tok_arr[InputInfo->fnc_arr[i].token_num].error = ERROR_CONFLICT_VAR_DECL;
                InputInfo->error = ERROR_CONFLICT_VAR_DECL;
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

    return SUCCESS;
}

//=========================================================================================

int check_vars_scope(struct InputInfo* InputInfo)
{
    int crl_brc_num = 0;
    for(size_t cur_tok = 0; cur_tok < InputInfo->tok_num; cur_tok++)
    {
        if(TYPE == CRL_BRC)
        {
            if(!strcmp(TEXT, "{"))
            {
                crl_brc_num++;
                continue;
            }
            crl_brc_num--;
            continue;
        }

        if(TYPE == VAR_DECL)
        {
            cur_tok++;
            if(crl_brc_num != 1)
            {
                ERROR = ERROR_VAR_SCOPE;
                InputInfo->error = ERROR_VAR_SCOPE;
            }
            cur_tok++;
        }
    }

    return InputInfo->error;
}

//=========================================================================================

int check_functions(struct InputInfo* InputInfo)
{
    for(size_t cur_tok = 0, cur_fnc = 1; cur_tok < InputInfo->tok_num; cur_tok++)
    {
        if(TYPE == FNC_DECL)
        {
            cur_tok++;

            if(TYPE != MAIN)
            {
                strcpy(FNC_ARR[cur_fnc].name, TEXT);
                FNC_ARR[cur_fnc].token_num = cur_tok;
                cur_tok += 2;

                cur_tok = fill_func_args(InputInfo, cur_fnc, cur_tok);
                fill_func_vars(InputInfo, cur_fnc, cur_tok);
            }
            else
            {
                strcpy(FNC_ARR[0].name, TEXT);
                FNC_ARR[0].token_num = cur_tok;
                cur_tok += 3;

                fill_func_vars(InputInfo, 0, cur_tok);
            }

            cur_fnc++;
        }
    }

    for(size_t i = 0; i < InputInfo->fnc_num; i++)
    {
        for(size_t j = i + 1; j < InputInfo->fnc_num; j++)
        {
            if(!strcmp(InputInfo->fnc_arr[i].name, InputInfo->fnc_arr[j].name))
            {
                InputInfo->tok_arr[InputInfo->fnc_arr[i].token_num].error = ERROR_FUNC_REDEFINITION;
                InputInfo->tok_arr[InputInfo->fnc_arr[j].token_num].error = ERROR_FUNC_REDEFINITION;
                InputInfo->error = ERROR_FUNC_REDEFINITION;
            }
        }
    }

    return InputInfo->error;
}

//=========================================================================================

size_t fill_func_args(struct InputInfo* InputInfo, size_t func, size_t cur_tok)
{
    size_t cur_arg = 0;
    if(strcmp(TEXT, ")"))
    {
        strcpy(FNC_ARR[func].args_arr[cur_arg++], TEXT);

        cur_tok++;
        while(TYPE == COMMA)
        {
            cur_tok++;
            strcpy(FNC_ARR[func].args_arr[cur_arg++], TEXT);
            cur_tok++;
        }
    }

    FNC_ARR[func].args_num = cur_arg;
    cur_tok++;

    return cur_tok;
}

//=========================================================================================

void fill_func_vars(struct InputInfo* InputInfo, size_t fnc_num, size_t crl_brc_tok)
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
        if(TYPE == CRL_BRC)
        {
            if(!strcmp(TEXT, "{"))
            {
                crl_brc_num++;
                continue;
            }
            crl_brc_num--;
            continue;
        }
        if(TYPE == VAR_DECL)
        {
            cur_tok++;
            strcpy(FNC_ARR[fnc_num].decl_vars[FNC_ARR[fnc_num].vars_num++], TEXT);
            cur_tok++;
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
    // printf("\n\n");
}

#undef FNC_ARR
#undef TEXT
#undef TYPE
#undef ERROR

//=========================================================================================

void prog_dtor(struct InputInfo* InputInfo)
{
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
