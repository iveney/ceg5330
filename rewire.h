#ifndef __REWIRE_H
#define __REWIRE_H

#include "array.h"
#include "imply.h"

#define DEBUG_SHOW_TRACE
//#undef DEBUG_SHOW_TRACE
#include "debug.h"

using namespace std;

// constants =========================================
#define REDUNDANT 0
#define NONREDUNDANT 1

// data structs=======================================
typedef struct {
	BNode *src;
	BNode *dst;
	// 1 -- not complemented, 0 -- complemented
	int polarity;
	BNodeType dstFunction;
} RewireAW;

void rewireEndImply();
int rewireFindAW(BNode *twSrc, BNode *twDst, int rlevel, Array<RewireAW> &aw);

#endif
