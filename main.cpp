#include "./frontend/RecursiveDescent.h"
#include "./graphviz/GraphvizFunctions.h"
#include "./backend/backend.h"

//===================================================================================

int main(int argc, char* argv[])
{
    struct InputInfo InputInfo = {};
    const char* filename = nullptr;

    text_info_ctor(&InputInfo, &filename, argc, argv);
    if(InputInfo.error != 0)        
    {                               
        frontend_dtor(&InputInfo);      
        return InputInfo.error;     
    }                               


    Tree* prog_tree = tree_ctor(&InputInfo);
    prog_tree->root = make_tree(prog_tree);
    make_graph(prog_tree->root, "graphviz.txt");
    struct BackendInfo BackInfo = {};
    backend_ctor(&BackInfo, &InputInfo, prog_tree->root, filename);


    init_tree_dtor(prog_tree);
    frontend_dtor(&InputInfo);
    backend_dtor(&BackInfo);
}
