#include "InputProcessing.h"
#include <string.h>
#include <ctype.h>
#include "colors.h"

//=========================================================================================

void text_info_ctor(struct TextInfo* TextInfo, const char* filename)
{
    open_file(TextInfo, filename);
    num_of_chars(TextInfo, filename);
    chars_buffer(TextInfo);

    count_tokens(TextInfo);
    tokenization(TextInfo);
    syntactic_analysis(TextInfo);
    TextInfo->lst_file = fopen("source.lst", "w");
    listing(TextInfo);
}

//=========================================================================================

void open_file(struct TextInfo* TextInfo, const char* filename)
{
    FILE* file_input = fopen(filename, "r");

    if(file_input == nullptr)
    {
        printf("IN ASM. There is no file named \"%s\".\n", filename);
        TextInfo->error = ERROR_MAIN_FILE_OPEN;
        return;
    }

    TextInfo->mainfile = file_input;
}

//=========================================================================================

void num_of_chars(struct TextInfo* TextInfo, const char* filename)
{
    struct stat buf = {};

    stat(filename, &buf);

    TextInfo->ch_num = buf.st_size + 1;

    // printf("TextInfo->ch_num:\n%lu\n\n", TextInfo->ch_num);
}

//=========================================================================================

void chars_buffer(struct TextInfo* TextInfo)
{
    TextInfo->chars_buff_ptr = (char*)calloc(TextInfo->ch_num, sizeof(char));
    if(TextInfo->chars_buff_ptr == nullptr)
    {
        TextInfo->error = ERROR_CHARS_BUFFER;
        return;
    }
    char* tmp_buff = (char*)calloc(TextInfo->ch_num, sizeof(char));
    if(tmp_buff == nullptr)
    {
        TextInfo->error = ERROR_CHARS_BUFFER;
        return;
    }
    fread(tmp_buff, sizeof(char), TextInfo->ch_num - 1, TextInfo->mainfile);
    fclose(TextInfo->mainfile);

    for(size_t i = 0; i < TextInfo->ch_num; i++)
    {
        if((tmp_buff[i] == '/') && (tmp_buff[i + 1] == '/'))
        {
            while(i < TextInfo->ch_num - 1  && tmp_buff[i] != '\n')
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
            while(i < TextInfo->ch_num &&  tmp_buff[i] != ')')
            {
                if(tmp_buff[i] == ' ')
                {
                    tmp_buff[i] = '\0';
                }
                i++;
            }
        }
    }

    for(size_t i = 0, j = 0; i < TextInfo->ch_num; i++)
    {
        if(tmp_buff[i] != '\0')
        {
            TextInfo->chars_buff_ptr[j++] = tmp_buff[i];
        }
    }

    free(tmp_buff);
    TextInfo->ch_num = strlen(TextInfo->chars_buff_ptr);
    // printf("%s\n", TextInfo->chars_buff_ptr);
}

//=========================================================================================

void count_tokens(struct TextInfo* TextInfo)
{
    char* cur_ptr  = nullptr;
    char* tmp_buff = (char*)calloc(TextInfo->ch_num + 1, sizeof(char));
    if(tmp_buff == nullptr)
    {
        TextInfo->error = ERROR_COUNT_TOKENS;
        return;
    }

    for(size_t i = 0, j = 0; i < TextInfo->ch_num; i++)
    {
        if(TextInfo->chars_buff_ptr[i] != ' ' && TextInfo->chars_buff_ptr[i] != '\n')
        {
            tmp_buff[j++] = TextInfo->chars_buff_ptr[i];
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

                TextInfo->tok_num++;
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
            TextInfo->tok_num++;
        }
    }

    free(tmp_buff);

    // printf("%lu\n", TextInfo->tok_num);
}

//=========================================================================================

void tokenization(struct TextInfo* TextInfo)
{
    char* tmp_buff = (char*)calloc(TextInfo->ch_num + 1, sizeof(char));
    
    size_t j = 0;
    for(size_t i = 0; i < TextInfo->ch_num; i++)
    {
        if(TextInfo->chars_buff_ptr[i] != ' ')
        {
            tmp_buff[j++] = TextInfo->chars_buff_ptr[i];
        }
    }
    // printf("%s\n", tmp_buff);
    TextInfo->tok_arr = (TokenInfo*)calloc(TextInfo->tok_num, sizeof(TokenInfo));

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

    for(size_t cur_ch = 0; cur_ch < strlen(tmp_buff) && tmp_buff[cur_ch] != '\0' && cur_tok < TextInfo->tok_num; cur_tok++)
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
                        TextInfo->tok_num--;
                        word_flag = MAX_KNOWN_TOKENS;
                        break;
                    }
                }
                // тут различаем унарный минус от отрицательного числа
                if(tmp_buff[cur_ch] == '-' && isdigit(tmp_buff[cur_ch + 1]) && 
                  ((cur_ch != 0 && !isdigit(tmp_buff[cur_ch - 1])) || cur_ch == 0))
                {
                    TextInfo->tok_arr[cur_tok].number = (float)atof(tmp_buff + cur_ch);
                    snprintf(TextInfo->tok_arr[cur_tok].text, sizeof(TextInfo->tok_arr[cur_tok].text), "%g", TextInfo->tok_arr[cur_tok].number);
                    cur_ch += strlen(TextInfo->tok_arr[cur_tok].text);
                    TextInfo->tok_arr[cur_tok].line = cur_line;
                    TextInfo->tok_arr[cur_tok].type = NUMBER;
                    TextInfo->tok_num--;
                    break;
                }

                // тут токенизируем '"', потом строковый литерал, и потом снова '"'
                if(tmp_buff[cur_ch] == '"')
                {
                    TextInfo->tok_arr[cur_tok].text[0] = '"';
                    TextInfo->tok_arr[cur_tok].line    = cur_line;
                    TextInfo->tok_arr[cur_tok].type    = QUOTE;
                    cur_ch++; // because of strlen of "\""

                    cur_tok++;
                    size_t strlen  = 0;

                    while(tmp_buff[cur_ch] != '"')
                    {
                        TextInfo->tok_arr[cur_tok].text[strlen] = tmp_buff[cur_ch];
                        strlen++;
                        cur_ch++;
                    }
                    TextInfo->tok_arr[cur_tok].line = cur_line;
                    TextInfo->tok_arr[cur_tok].type = STRING;

                    cur_tok++;
                    if(tmp_buff[cur_ch] == '"')
                    {
                        TextInfo->tok_arr[cur_tok].text[0] = '"';
                        TextInfo->tok_arr[cur_tok].line    = cur_line;
                        TextInfo->tok_arr[cur_tok].type    = QUOTE;
                        cur_ch++; // because of strlen of "\""
                    }

                    break;
                }

                cur_ch += strlen(all_token[cur_known_tok].text);
                strcpy(TextInfo->tok_arr[cur_tok].text, all_token[cur_known_tok].text);
                TextInfo->tok_arr[cur_tok].line = cur_line;
                TextInfo->tok_arr[cur_tok].type = all_token[cur_known_tok].type;
                break;
            }

            word_flag++;
        }

        // токенизируем число или переменную
        if(word_flag == MAX_KNOWN_TOKENS)
        {
            TextInfo->tok_arr[cur_tok].line = cur_line;
            if(isalpha(tmp_buff[cur_ch]))
            {
                if(cur_tok != 0 && TextInfo->tok_arr[cur_tok - 1].type == FNC_DECL)
                {
                    TextInfo->tok_arr[cur_tok].type = FNC_NAME;
                }
                else
                {
                    TextInfo->tok_arr[cur_tok].type = VAR_NAME;
                }
            }

            else if(isdigit(tmp_buff[cur_ch]) || tmp_buff[cur_ch] == '-')
            {
                TextInfo->tok_arr[cur_tok].type = NUMBER;
            }

            int i = 0;
            while(isalpha(tmp_buff[cur_ch]) || isdigit(tmp_buff[cur_ch]) || tmp_buff[cur_ch] == '_' || tmp_buff[cur_ch] == '.')
            {
                TextInfo->tok_arr[cur_tok].text[i] = tmp_buff[cur_ch];
                i++;
                cur_ch++;
            }

            if(TextInfo->tok_arr[cur_tok].type == NUMBER)
            {
                TextInfo->tok_arr[cur_tok].number = (float)atof(TextInfo->tok_arr[cur_tok].text);
            }
        }
    }

    for(size_t i = 0; i + 1 < TextInfo->tok_num; i++)
    {
        if(TextInfo->tok_arr[i].type == VAR_NAME && !strcmp(TextInfo->tok_arr[i + 1].text, "("))
        {
            TextInfo->tok_arr[i].type = FNC_NAME;
        }
    }

    free(tmp_buff);

    // printf("\n\nTextInfo->tok_arr:\n\n");
    // for(size_t i = 0; i < TextInfo->tok_num; i++)
    // {
    //     printf("token[%lu]: %s\n", i + 1, TextInfo->tok_arr[i].text);
    //     printf("type:       %d\n", TextInfo->tok_arr[i].type);
    //     printf("line:       %lu\n", TextInfo->tok_arr[i].line);
    //     printf("error:      %d\n\n", TextInfo->tok_arr[i].error);
    // }
    // printf("\n\n");
}

//=========================================================================================

#define TYPE  TextInfo->tok_arr[cur_tok].type 
#define TEXT  TextInfo->tok_arr[cur_tok].text 
#define ERROR TextInfo->tok_arr[cur_tok].error

#define R_VAL (TYPE == NUMBER   || TYPE == VAR_NAME ||      \
               TYPE == RND_BRC  || TYPE == ARTH_OP  ||      \
               TYPE == FNC_NAME || TYPE == COMMA)           \

#define CHECK_TOK(type, err)                                                            \
        cur_tok++;                                                                      \
        if(cur_tok < TextInfo->tok_num)                                                 \
        {                                                                               \
            if(TYPE != type)                                                            \
            {                                                                           \
                errors++;                                                               \
                ERROR = err;                                                            \
            }                                                                           \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            TextInfo->tok_arr[TextInfo->tok_num - 1].error = ERROR_END_OF_THE_FILE;     \
        }                                                                               \

void syntactic_analysis(struct TextInfo* TextInfo)
{
    int errors = 0;
    for(size_t cur_tok = 0; cur_tok < TextInfo->tok_num; cur_tok++)
    {
        switch(TextInfo->tok_arr[cur_tok].type)
        {
            case(VAR_DECL):
            {
                CHECK_TOK(VAR_NAME, ERROR_VAR_INIT);
                CHECK_TOK(ASSIGN, ERROR_VAR_INIT);
                cur_tok++;
                while(R_VAL)
                {
                    cur_tok++;
                }
                cur_tok--;
                CHECK_TOK(SEMICLN, ERROR_VAR_INIT);

                break;
            }
            
            case(FNC_DECL):
            {
                if(TextInfo->tok_arr[cur_tok + 1].type != MAIN)
                {
                    CHECK_TOK(FNC_NAME, ERROR_FNC_DECL);
                    CHECK_TOK(RND_BRC, ERROR_FNC_DECL);
                    cur_tok++;
                    while(TYPE == COMMA || TYPE == VAR_NAME)
                    {
                        cur_tok++;
                    }
                    cur_tok--;
                    CHECK_TOK(RND_BRC, ERROR_FNC_DECL);
                    CHECK_TOK(CRL_BRC, ERROR_FNC_DECL);
                }

                else
                {
                    cur_tok++;
                    CHECK_TOK(RND_BRC, ERROR_FNC_DECL);
                    CHECK_TOK(RND_BRC, ERROR_FNC_DECL);
                    CHECK_TOK(CRL_BRC, ERROR_VAR_NAME);
                }

                break;
            }

            case(COND_OP):
            {
                if(!strcmp(TEXT, "if"))
                {
                    CHECK_TOK(RND_BRC, ERROR_COND_OP);
                    cur_tok++;
                    while(R_VAL)
                    {
                        cur_tok++;
                    }
                    cur_tok--;
                    CHECK_TOK(LOG_OP, ERROR_COND_OP);
                    cur_tok++;
                    while(R_VAL)
                    {
                        cur_tok++;
                    }
                    cur_tok -= 2;
                    CHECK_TOK(RND_BRC, ERROR_COND_OP);
                    CHECK_TOK(CRL_BRC, ERROR_COND_OP);
                }

                else if(!strcmp(TEXT, "elif"))
                {
                    CHECK_TOK(RND_BRC, ERROR_COND_OP);
                    cur_tok++;
                    while(R_VAL)
                    {
                        cur_tok++;
                    }
                    cur_tok--;
                    CHECK_TOK(LOG_OP, ERROR_COND_OP);
                    cur_tok++;
                    while(R_VAL)
                    {
                        cur_tok++;
                    }
                    cur_tok -= 2;
                    CHECK_TOK(RND_BRC, ERROR_COND_OP);
                    CHECK_TOK(CRL_BRC, ERROR_COND_OP);
                }

                else if(!strcmp(TEXT, "else"))
                {
                    CHECK_TOK(CRL_BRC, ERROR_COND_OP);
                }

                break;
            }

            case(LOOP):
            {
                CHECK_TOK(RND_BRC, ERROR_LOOP);
                cur_tok++;
                while(R_VAL)
                {
                    cur_tok++;
                }
                cur_tok--;
                CHECK_TOK(LOG_OP, ERROR_LOOP);
                cur_tok++;
                while(R_VAL)
                {
                    cur_tok++;
                }
                cur_tok -= 2;
                CHECK_TOK(RND_BRC, ERROR_LOOP);
                CHECK_TOK(CRL_BRC, ERROR_LOOP);

                break;
            }

            case(VAR_NAME):
            {
                CHECK_TOK(ASSIGN, ERROR_VAR_NAME);
                cur_tok++;
                printf("%s\n", TEXT);
                while(R_VAL)
                {
                    cur_tok++;
                }
                cur_tok--;
                CHECK_TOK(SEMICLN, ERROR_VAR_NAME);

                break;
            }

            case(FNC_NAME):
            {
                if(!strcmp(TEXT, "printf"))
                {
                    CHECK_TOK(RND_BRC, ERROR_FNC_NAME);
                    cur_tok++;
                    if(TYPE == QUOTE)
                    {
                        CHECK_TOK(STRING, ERROR_FNC_NAME);
                        CHECK_TOK(QUOTE, ERROR_FNC_NAME);
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
                    CHECK_TOK(RND_BRC, ERROR_FNC_NAME);
                    CHECK_TOK(SEMICLN, ERROR_FNC_NAME);

                    break;
                }

                else if(!strcmp(TEXT, "scanf"))
                {
                    CHECK_TOK(RND_BRC, ERROR_FNC_NAME);
                    CHECK_TOK(VAR_NAME, ERROR_FNC_NAME);
                    CHECK_TOK(RND_BRC, ERROR_FNC_NAME);
                    CHECK_TOK(SEMICLN, ERROR_FNC_NAME);

                    break;
                }

                else if(!strcmp(TEXT, "sqrt")   || !strcmp(TEXT, "sin")   || !strcmp(TEXT, "cos")    ||
                        !strcmp(TEXT, "tg")     || !strcmp(TEXT, "ctg")   || !strcmp(TEXT, "arcsin") ||
                        !strcmp(TEXT, "arccos") || !strcmp(TEXT, "arctg") || !strcmp(TEXT, "arcctg") ||
                        !strcmp(TEXT, "sh")     || !strcmp(TEXT, "ch")    || !strcmp(TEXT, "ln")     ||
                        !strcmp(TEXT, "exp"))
                {
                    CHECK_TOK(RND_BRC, ERROR_FNC_NAME);
                    cur_tok++;
                    while(R_VAL)
                    {
                        cur_tok++;
                    }
                    cur_tok--;
                    CHECK_TOK(RND_BRC, ERROR_FNC_NAME);
                    CHECK_TOK(SEMICLN, ERROR_FNC_NAME);

                    break;
                } 

                else
                {
                    CHECK_TOK(RND_BRC, ERROR_FNC_NAME);
                    cur_tok++;
                    while(R_VAL || TYPE == COMMA)
                    {
                        cur_tok++;
                    }
                    cur_tok -= 2;
                    CHECK_TOK(RND_BRC, ERROR_FNC_NAME);
                    CHECK_TOK(SEMICLN, ERROR_FNC_NAME);

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
                    ERROR = ERROR_RETURN;

                    break;
                }
                while(R_VAL)
                {
                    cur_tok++;
                }
                cur_tok--;
                CHECK_TOK(SEMICLN, ERROR_RETURN);

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

    if(TextInfo->tok_arr[TextInfo->tok_num - 1].type != CRL_BRC && 
       TextInfo->tok_arr[TextInfo->tok_num - 1].type != SEMICLN)
    {
        TextInfo->tok_arr[TextInfo->tok_num - 1].error = ERROR_END_OF_PROG;
    }

    if(TextInfo->tok_arr[0].type != FNC_DECL && TextInfo->tok_arr[0].type != VAR_DECL)
    {
        TextInfo->tok_arr[0].error = ERROR_BAD_FIRST_TOK;
    }

    size_t first_main = 0;
    size_t main_cnt   = 0;
    for(size_t cur_tok = 0; cur_tok < TextInfo->tok_num; cur_tok++)
    {
        if(TYPE == MAIN)
        {
            main_cnt++;
            if(main_cnt > 1)
            {
                ERROR = ERROR_DOUBLE_MAIN;
                break;
            }
            first_main = cur_tok;
        }
    }

    // if(main_cnt != 1)
    // {
    //     TextInfo->tok_arr[0].error = ERROR_NO_MAIN;
    // }

    for(size_t cur_tok = 0; cur_tok < TextInfo->tok_num; cur_tok++)
    {
        if(cur_tok > first_main && TYPE == FNC_DECL && 
          (cur_tok + 1 < TextInfo->tok_num && TextInfo->tok_arr[cur_tok + 1].type != MAIN))
        {
            ERROR = ERROR_NO_PREV_FNC_DECL;
        }
    }



    // printf("\n\nTextInfo->tok_arr:\n\n");
    // for(size_t i = 0; i < TextInfo->tok_num; i++)
    // {
    //     printf("token[%lu]: %s\n", i + 1, TextInfo->tok_arr[i].text);
    //     printf("type:       %d\n", TextInfo->tok_arr[i].type);
    //     printf("line:       %lu\n", TextInfo->tok_arr[i].line);
    //     printf("error:      %d\n\n", TextInfo->tok_arr[i].error);
    // }
    // printf("\n\n");
}


#undef R_VAL
#undef CHECK_TOK

//=========================================================================================

#define LINE TextInfo->tok_arr[cur_tok].line

void listing(struct TextInfo* TextInfo)
{
    fprintf(TextInfo->lst_file, "signature: %s\n", TextInfo->signature);
    size_t total_errors = 0;
    for(size_t cur_tok = 0; cur_tok < TextInfo->tok_num; cur_tok++)
    {
        if(TextInfo->tok_arr[cur_tok].error != 0)
        {
            total_errors++;
        }
    }
    fprintf(TextInfo->lst_file, "total errors: %lu\n", total_errors);
    fprintf(TextInfo->lst_file, "quantity: %lu\n\n", TextInfo->tok_num);

    fprintf(TextInfo->lst_file, "|  ip  |\t\tCOMMAND\t\t\t|\t\tTYPE\t\t|\t\tLINE\t\t|\t\tERROR\n");
    for(size_t cur_tok = 0; cur_tok < TextInfo->tok_num; cur_tok++)
    {
        fprintf(TextInfo->lst_file, "|%06lu|\t\t", cur_tok);
        fprintf(TextInfo->lst_file, "%-16s|\t", TEXT);
        
        switch(TYPE)
        {
            case(VAR_DECL):
            {
                fprintf(TextInfo->lst_file, "  var_decl\t\t|\t\t");
                break;
            }
            case(FNC_DECL):
            {
                fprintf(TextInfo->lst_file, "  fnc_decl\t\t|\t\t");
                break;
            }
            case(ASSIGN):
            {
                fprintf(TextInfo->lst_file, "  assign\t\t|\t\t");
                break;
            }
            case(ARTH_OP):
            {
                fprintf(TextInfo->lst_file, "  arth_op\t\t|\t\t");
                break;
            }
            case(LOG_OP):
            {
                fprintf(TextInfo->lst_file, "  log_op\t\t|\t\t");
                break;
            }
            case(COND_OP):
            {
                fprintf(TextInfo->lst_file, "  conp_op\t\t|\t\t");
                break;
            }
            case(LOOP):
            {
                fprintf(TextInfo->lst_file, "  loop  \t\t|\t\t");
                break;
            }
            case(NUMBER):
            {
                fprintf(TextInfo->lst_file, "  number\t\t|\t\t");
                break;
            }
            case(VAR_NAME):
            {
                fprintf(TextInfo->lst_file, "  var_name\t\t|\t\t");
                break;
            }
            case(FNC_NAME):
            {
                fprintf(TextInfo->lst_file, "  fnc_name\t\t|\t\t");
                break;
            }
            case(MAIN):
            {
                fprintf(TextInfo->lst_file, "  main  \t\t|\t\t");
                break;
            }
            case(RETURN):
            {
                fprintf(TextInfo->lst_file, "  return\t\t|\t\t");
                break;
            }
            case(RND_BRC):
            {
                fprintf(TextInfo->lst_file, "  rnd_brc\t\t|\t\t");
                break;
            }
            case(CRL_BRC):
            {
                fprintf(TextInfo->lst_file, "  crl_brc\t\t|\t\t");
                break;
            }
            case(COMMA):
            {
                fprintf(TextInfo->lst_file, "  comma\t\t\t|\t\t");
                break;
            }
            case(SEMICLN):
            {
                fprintf(TextInfo->lst_file, "  semicln\t\t|\t\t");
                break;
            }
            case(QUOTE):
            {
                fprintf(TextInfo->lst_file, "  quote  \t\t|\t\t");
                break;
            }
            case(STRING):
            {
                fprintf(TextInfo->lst_file, "  string\t\t|\t\t");
                break;
            }

            default:
            {
                printf(RED "\n\n Unknown token or token sequence in listing: %s\n\n" RESET, TEXT);
                break;
            }
        }

        fprintf(TextInfo->lst_file, " %-4lu\t\t|\t\t", LINE);
        fprintf(TextInfo->lst_file, "  %d\n", ERROR);
    }

    fclose(TextInfo->lst_file);
}

#undef TEXT
#undef TYPE
#undef ERROR
#undef LINE
