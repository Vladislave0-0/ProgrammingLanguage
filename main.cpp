#include "InputProcessing.h"
#include "RecursiveDescent.h"
#include "Tree.h"
#include "GraphvizFunctions.h"

//========================================================

int main()
{
    struct InputInfo InputInfo = {};
    text_info_ctor(&InputInfo, "input.txt");

    Tree* prog_tree = tree_ctor(&InputInfo);
    prog_tree->root = make_tree(prog_tree);

    make_graph(prog_tree->root, "image.txt");

    init_tree_dtor(prog_tree);
    prog_dtor(&InputInfo);
}
