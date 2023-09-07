#include "InputProcessing.h"
#include "RecursiveDescent.h"
#include "Tree.h"
#include "GraphvizFunctions.h"

//========================================================

int main()
{
    struct TextInfo TextInfo = {};
    text_info_ctor(&TextInfo, "input.txt");

    Tree* prog_tree = tree_ctor(&TextInfo);
    prog_tree->root = make_tree(prog_tree);

    make_graph(prog_tree->root, "image.txt");

    init_tree_dtor(prog_tree);

    free(TextInfo.tok_arr);
    free(TextInfo.chars_buff_ptr);
}
