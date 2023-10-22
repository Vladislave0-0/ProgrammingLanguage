#include "./frontend/InputProcessing.h"
#include "./frontend/RecursiveDescent.h"
#include "./frontend/Tree.h"
#include "./graphviz/GraphvizFunctions.h"
#include "./backend/backend.h"

//===================================================================================

int main(int argc, char* argv[])
{
    const char* filename = nullptr;
    if(terminal_processing(argc, argv, &filename))
    {
        return ERROR_CMD_LINE_ARGS;
    }


    struct InputInfo InputInfo = {};
    text_info_ctor(&InputInfo, filename);
    if(InputInfo.error != 0)        
    {                               
        prog_dtor(&InputInfo);      
        return InputInfo.error;     
    }                               


    Tree* prog_tree = tree_ctor(&InputInfo);
    prog_tree->root = make_tree(prog_tree);
    make_graph(prog_tree->root, "graphviz.txt");
    struct BackendInfo BackInfo = {};
    backend_ctor(&BackInfo, &InputInfo, prog_tree->root, filename);


    // dtor
    init_tree_dtor(prog_tree);
    prog_dtor(&InputInfo);
    for(size_t i = 0; i < BackInfo.fnc_num; i++)
    {
        free(BackInfo.fnc_arr[i].args_arr);
        free(BackInfo.fnc_arr[i].decl_vars_arr);
        free(BackInfo.fnc_arr[i].all_vars_arr);
    }
    free(BackInfo.fnc_arr);
}
