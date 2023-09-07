#define TOK     tree->cur_tok
#define TOK_ARR tree->tok_arr
#define TEXT    TOK_ARR[TOK].text
#define TYPE    TOK_ARR[TOK].type


#define CREATE_NODE                                           create_node(nullptr, nullptr, _NUL, nullptr, 0)               //
#define CREATE_EMPTY(left_child, right_child)                 create_node(left_child, right_child, _EMPTY, nullptr, 0)      //                     
#define CREATE_VAR_DECL(right_child)                          create_node(nullptr, right_child, _VAR_DECL, nullptr, 0)      //
#define CREATE_FNC_DECL                                       create_node(nullptr, nullptr, _FNC_DECL, nullptr, 0)          //
#define CREATE_ASSIGN(left_child, right_child)                create_node(left_child, right_child, _ASSIGN, nullptr, 0)     //
#define CREATE_ARTH_OP(left_child, right_child, name)         create_node(left_child, right_child, _ARTH_OP, name, 0)       //
#define CREATE_LOG_OP(log_name)                               create_node(nullptr, nullptr, _LOG_OP, log_name, 0)           //
#define CREATE_LOOP                                           create_node(nullptr, nullptr, _LOOP, nullptr, 0)              //
#define CREATE_NUM(value)                                     create_node(nullptr, nullptr, _NUM, nullptr, value)           //
#define CREATE_VAR_NAME(var_name)                             create_node(nullptr, nullptr, _VAR_NAME, var_name, 0)         //
#define CREATE_FNC_NAME(right_child, fnc_name)                create_node(nullptr, right_child, _FNC_NAME, fnc_name, 0)     //
#define CREATE_FNC_PARAMS                                     create_node(nullptr, nullptr, _PARAM, nullptr, 0)             //
#define CREATE_COND_OP(left_child, cond_name)                 create_node(left_child, nullptr, _COND_OP, cond_name, 0)      // 
#define CREATE_LOG_OP(log_name)                               create_node(nullptr, nullptr, _LOG_OP, log_name, 0)           //
#define CREATE_RETURN                                         create_node(nullptr, nullptr, _RETURN, nullptr, 0)            //
#define CREATE_STRING(str)                                    create_node(nullptr, nullptr, _STRING, str, 0)                //


#define ADD_NODE(left_child, right_child) CREATE_ARTH_OP(left_child, right_child, "+")
#define SUB_NODE(left_child, right_child) CREATE_ARTH_OP(left_child, right_child, "-")
#define MUL_NODE(left_child, right_child) CREATE_ARTH_OP(left_child, right_child, "*")
#define DIV_NODE(left_child, right_child) CREATE_ARTH_OP(left_child, right_child, "/")
#define POW_NODE(left_child, right_child) CREATE_ARTH_OP(left_child, right_child, "^")


#define PRINTF_NODE(right_child)                         CREATE_FNC_NAME(right_child, "printf")
#define SCANF_NODE(right_child)                          CREATE_FNC_NAME(right_child, "scanf")
#define SQRT_NODE(right_child)                           CREATE_FNC_NAME(right_child, "sqrt")
#define SIN_NODE(right_child)                            CREATE_FNC_NAME(right_child, "sin")
#define COS_NODE(right_child)                            CREATE_FNC_NAME(right_child, "cos")
#define TG_NODE(right_child)                             CREATE_FNC_NAME(right_child, "tg")
#define CTG_NODE(right_child)                            CREATE_FNC_NAME(right_child, "ctg")
#define ARCSIN_NODE(right_child)                         CREATE_FNC_NAME(right_child, "arcsin")
#define ARCCOS_NODE(right_child)                         CREATE_FNC_NAME(right_child, "arccos")
#define ARCTG_NODE(right_child)                          CREATE_FNC_NAME(right_child, "arctg")
#define ARCCTG_NODE(right_child)                         CREATE_FNC_NAME(right_child, "arcctg")
#define SH_NODE(right_child)                             CREATE_FNC_NAME(right_child, "sh")
#define CH_NODE(right_child)                             CREATE_FNC_NAME(right_child, "ch")
#define LN_NODE(right_child)                             CREATE_FNC_NAME(right_child, "ln")
#define EXP_NODE(right_child)                            CREATE_FNC_NAME(right_child, "exp")
