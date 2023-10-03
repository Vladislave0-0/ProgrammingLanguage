#ifndef GRAPHVIZ_FUNCTIONS_H
#define GRAPHVIZ_FUNCTIONS_H

//=========================================================

#include "../frontend/Tree.h"

//=========================================================
/**
 * @brief Collects a command into a string that should be 
 *        executed by the system command in the terminal.
 *        Calls the tree printout function.
 * 
 * @param root the root of the tree
 * @param filename file name
 */
void make_graph(Node* root, const char* filename);

//=========================================================
/**
 * @brief Prints a dot tree markup language to a .txt file.
 * 
 * @param root the root of the tree
 * @param dotfile the file in which the text is printed 
 *                for further compilation.
 */
void print_dot_tree(Node* root, FILE* dotfile);

//=========================================================

//dot graphviz.txt -Tpng -o tree.png
//pdflatex TexExpression.tex > nul 2>&1

//=========================================================

#endif //GRAPHVIZ_FUNCTIONS_H
