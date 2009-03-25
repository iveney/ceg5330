#ifndef __BNODE_H
#define __BNODE_H

#include <string>

#define DEBUG_SHOW_TRACE
//#undef DEBUG_SHOW_TRACE
#include "debug.h"

using namespace std;

enum BNodeType_t_ {BNODE_PI, BNODE_PO, BNODE_AND, BNODE_OR, BNODE_INV, BNODE_BUF, BNODE_COMPLEX};
typedef enum BNodeType_t_ BNodeType_t;

class BNodeProperty {
};

#endif
