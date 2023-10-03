#include "./GraphvizFunctions.h" 
#include "../include/colors.h"

//=====================================================================================================================

#define PRINT_NODE(color, type, type_text)                                                                      \
        fprintf(dotfile, "\tnode_%p [style = \"filled\", fillcolor = %s, shape = Mrecord, label = "             \
                         "\"{{<f1> %p} | {type: %s", root, color, root, type_text);                             \
        if(type == _NUM)                                                                                        \
        {                                                                                                       \
            fprintf(dotfile, " | value = %g}", root->val.number);                                               \
        }                                                                                                       \
        else if(type == _VAR_DECL || type == _ASSIGN || type == _LOOP)                                          \
        {                                                                                                       \
            if     (type == _VAR_DECL) fprintf(dotfile, " | value = var}");                                     \
            else if(type == _ASSIGN)   fprintf(dotfile, " | value = '='}");                                     \
            else if(type == _LOOP)     fprintf(dotfile, " | value = while}");                                   \
        }                                                                                                       \
        else if(type == _EMPTY || type == _PARAM || type == _FNC_DECL || type == _RETURN)                       \
        {                                                                                                       \
            fprintf(dotfile, "}");                                                                              \
        }                                                                                                       \
        else                                                                                                    \
        {                                                                                                       \
            if     (type == _LOG_OP) fprintf(dotfile, " | value = '\\%s'}", root->val.name);                    \
            else if(type == _STRING) fprintf(dotfile, " | value = \\\"%s\\\"}", root->val.name);                \
            else fprintf(dotfile, " | value = %s}", root->val.name);                                            \
        }                                                                                                       \
        if(root->left_child == nullptr && root->right_child == nullptr)                                         \
        {                                                                                                       \
            fprintf(dotfile,  "}\"];\n");                                                                      \
        }                                                                                                       \
        else                                                                                                    \
        {                                                                                                       \
            fprintf(dotfile,  " | {<f2>%p | <f3> %p}}\"];\n", root->left_child, root->right_child);             \
        }                                                                                                       \
        

#define PRINT_CHILD                                                                                 \
        if(root->left_child != nullptr)                                                             \
        {                                                                                           \
            fprintf(dotfile, "\tnode_%p:<f2>:s -> node_%p:<f1>:n;\n", root, root->left_child);      \
        }                                                                                           \
                                                                                                    \
        if(root->right_child != nullptr)                                                            \
        {                                                                                           \
            fprintf(dotfile, "\tnode_%p:<f3>:s -> node_%p:<f1>:n;\n", root, root->right_child);     \
        }                                                                                           \

//=====================================================================================================================

void make_graph(Node* root, const char* filename) 
{
    char path[50] = "./output/";
    strcat(path, filename);
    FILE* dotfile = fopen(path, "w");

    // if(dotfile == nullptr)
    // {
    //     printf(RED "\nIn function %s at %s(%u):\nError code: %d. Check file \"Tree.h\" to decipher "
    //                "the error code.\n\n" RESET, __PRETTY_FUNCTION__, __FILE__, __LINE__, ERROR_OPEN_GRAPHVIZ_FILE);       

    //     return;
    // }

    fprintf(dotfile, "digraph tree\n");
    fprintf(dotfile, "{\n");
    fprintf(dotfile, "\trankdir = TB;\n");
    fprintf(dotfile, "\tsplines = false;\n\n");

    print_dot_tree(root, dotfile);
    fprintf(dotfile, "}\n\n");
    fclose(dotfile);


    char dot_command[70] = "dot ";
    char new_file_ext[30] = {};
    strcat(dot_command, path);
    strcat(dot_command, " -Tpng -o ");
    strcat(dot_command, "./output/");
    for(size_t i = 0; i < strlen(filename) - 3; i++)
    {
        new_file_ext[i] = filename[i];
    }
    strcat(new_file_ext, "png");
    strcat(dot_command, new_file_ext);            

    system(dot_command);
}

//=====================================================================================================================

#define TYPE root->type

void print_dot_tree(Node* root, FILE* dotfile)
{
    switch(TYPE)
    {
        case(_EMPTY)   : PRINT_NODE("white", _EMPTY,             "EMPTY");      break;
        case(_NUM)     : PRINT_NODE("skyblue", _NUM,             "NUM");        break;
        case(_ARTH_OP) : PRINT_NODE("yellow", _ARTH_OP,          "ARTH_OP");    break;
        case(_VAR_DECL): PRINT_NODE("hotpink2", _VAR_DECL,       "VAR_DECL");   break;
        case(_VAR_NAME): PRINT_NODE("indianred1", _VAR_NAME,     "VAR_NAME");   break;
        case(_FNC_DECL): PRINT_NODE("mediumseagreen", _FNC_DECL, "FNC_DECL");   break;
        case(_FNC_NAME): PRINT_NODE("limegreen", _FNC_NAME,      "FNC_NAME");   break;
        case(_ASSIGN)  : PRINT_NODE("grey82", _ASSIGN,           "ASSIGN");     break;
        case(_PARAM)   : PRINT_NODE("lightgreen", _PARAM,        "PARAM");      break;
        case(_LOG_OP)  : PRINT_NODE("plum", _LOG_OP,             "LOG_OP");     break;
        case(_LOOP)    : PRINT_NODE("tan", _LOOP,                "LOOP");       break;
        case(_COND_OP) : PRINT_NODE("pink", _COND_OP,            "COND_OP");    break;
        case(_RETURN)  : PRINT_NODE("sienna3", _RETURN,          "RETURN");     break;
        case(_STRING)  : PRINT_NODE("mediumpurple", _STRING,     "STRING");     break;
        default: printf("Error print_tree");                                    break;
    }

    if(TYPE != _NUM)
    {
        PRINT_CHILD;
    }                                                                                                                                                  

    if(root->left_child != nullptr)
    {
        print_dot_tree(root->left_child, dotfile);
    }

    if(root->right_child != nullptr)
    {
        print_dot_tree(root->right_child, dotfile);
    }
}

#undef TYPE
