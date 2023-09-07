#ifndef RECURSIVE_DESCENT_H
#define RECURSIVE_DESCENT_H

//=========================================================================================

#include <stdio.h>
#include "Tree.h"
#include "DSL.h"

//=========================================================================================

Node* make_tree(struct Tree* tree);

Node* getGen(struct Tree* tree);

Node* getProg(struct Tree* tree);

Node* getFnc_decl(struct Tree* tree);

Node* getFnc_params(struct Tree* tree);

Node* getFnc_name(struct Tree* tree);

Node* getIf(struct Tree* tree);

Node* getElif_else(struct Tree* tree);

Node* getLoop(struct Tree* tree);

Node* getVar_decl(struct Tree* tree);

Node* getAssign(struct Tree* tree);

Node* getExp(struct Tree* tree);

Node* getMul(struct Tree* tree);

Node* getDeg(struct Tree* tree);

Node* getBrc(struct Tree* tree);

Node* getWord(struct Tree* tree);

Node* getNum(struct Tree* tree);

//=========================================================================================

#endif //RECURSIVE_DESCENT_H
