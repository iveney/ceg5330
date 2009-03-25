#ifndef __IMPLY_H
#define __IMPLY_H

#include "array.h"
#include "ignetwork.h"

#define CONSISTENT 0
#define INCONSISTENT 1

int IGMarkInitNodes(Array<IGNode *> &initNodes, Array<IGNode *> &markedNodes);
int IGMarkInitNodesForced(Array<IGNode *> &initNodes, Array<IGNode *> &markedNodes);
void IGMarkFaultyNodes(Array<IGNode *> &initNodes);
void IGUnmarkFaultyNodes(Array<IGNode *> &initNodes);
int IGImply(int mask, Array<IGNode *> &markedNodes);
int IGRlearnImply(int r, Array<IGNode *> &markedNodes);
void IGResetImply(Array<IGNode *> &markedNodes);
void IGEndImply();
void IGStopForwardPropagation(IGNode *node);
void IGAllowForwardPropagation(IGNode *node);
void IGStopBackwardPropagation(IGNode *node);
void IGAllowBackwardPropagation(IGNode *node);
void IGClearImplyStack();

#endif
