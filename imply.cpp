#include <string>
#include <map>
#include "array.h"
#include "ignetwork.h"
#include "imply.h"

#define DEBUG_SHOW_TRACE
//#undef DEBUG_SHOW_TRACE
#include "debug.h"

#define INIT_MASK 0xFFFFFFFF
#define DEFAULT_LIST_SIZE 20

#define allocNodelist() _nodelists.size == 0 ? new Array<IGNode *>(DEFAULT_LIST_SIZE) : _nodelists.remove()
#define freeNodelist(list) (list)->clear(),_nodelists.insert((list)), (list) = NULL
//#define freeNodelist(list) (list)->clear(),_nodelists.insert((list))

struct IGStackElement_ {
	IGNode *node;
	int forced;
};

typedef struct IGStackElement_ IGStackElement;
																													 
static Array<Array<IGNode *> *> _nodelists;

static Array<IGStackElement> implyStack(DEFAULT_LIST_SIZE);

static int power2[6] = {1, 2, 4, 8, 16, 32};

#define MAX_BIT 5
static int rStop = 0;

inline int imply_(int mask, Array<IGNode *> &markedNodes, Array<IGNode *> &unjustifiedAndNodes) {
	int i, j, flag;
	int forced;
	IGStackElement se;
	IGEdgeProperty ep;
	IGNode *node, *fanin, *fanout, *conflict;
	int returnVal = CONSISTENT;

	while (implyStack.size > 0) {
		se = implyStack.remove();
		node = se.node;
		forced = se.forced;

		//cout << "marking node: " << node_name(node) << endl;

		// clear the node's stack mark
		node_flag(node, IG_INSTACK) &= ~(mask);
		if (forced) {
			node_flag(node, IG_INSTACKFORCED) &= ~(mask);
		}

		// check the node type
		if (node_type(node) == IGNODE_SIGNAL) {
			
			//if (node_imply_type(node) == IGNODE_F0) {
			//	cout << node_name(node_bnode(node)) << " 0" << endl;
			//}
			//else if (node_imply_type(node) == IGNODE_F1){				
			//	cout << node_name(node_bnode(node)) << " 1" << endl;
			//}
					
			// check if marking this node cause conflict if this node is not marked
			if (!(node_mark(node) & mask)){
				flag = 0;
				
				for (i = 0; i < node_link(node).size; i++) {
					conflict = array_fetch(node_link(node), i);

					if (node_mark(conflict) & mask) {
						//cout << node_name(node) << " " << mask << " conflict at ";
						//cout << node_name(conflict) << " " << node_mark(conflict) << endl;
						returnVal = INCONSISTENT;
						flag = 1;

						while (implyStack.size > 0) {
							// clear all stack marks
							se = implyStack.remove();
							node = se.node;
							forced = se.forced;
							node_flag(node, IG_INSTACK) &= ~(mask);
							if (forced) {
								node_flag(node, IG_INSTACKFORCED) &= ~(mask);
							}
						}
						//implyStack.clear();
						break;
					}
				}

				if (flag) {
					break;
				}
				
				// insert the node if it is unmarked
				markedNodes.insert(node);
			}

			// mark this node
			node_mark(node) |= mask;

			if (forced) {
				// mark this node forced
				node_forced(node) |= mask;
			}

			foreach_fanout(node, i, fanout) {
				if (forced) {
					if ((!(node_forced(fanout) & mask)) && (!(node_flag(fanout, IG_INSTACKFORCED) & mask))) {
						ep = node_fanout_ep(node, i);
						
						se.node = fanout;
						se.forced = ep.type == IGEDGE_B ? 1 : 0;
						implyStack.insert(se);

						if (se.forced) {
							node_flag(fanout, IG_INSTACKFORCED) |= mask;
						}
						node_flag(fanout, IG_INSTACK) |= mask;
					}
				}
				else {
					if ((!(node_mark(fanout) & mask)) && (!(node_flag(fanout, IG_INSTACK) & mask))) {
						// se.forced = 0 here
						se.node = fanout;
						implyStack.insert(se);

						node_flag(fanout, IG_INSTACK) |= mask;
					}
				}
			}
		}
		else { // node_type == IGNODE_AND
			// check if all fanin is marked
			flag = 1;
			foreach_fanin(node, i, fanin) {
				if (!(node_mark(fanin) & mask)) {
					flag = 0;
					break;
				}
			}

			if (flag) {
				// mark this node
				node_mark(node) |= mask;
				markedNodes.insert(node);

				if (forced) {
					// mark this node forced
					node_forced(node) |= mask;
				}

				foreach_fanout(node, i, fanout) {
					if (forced) {
						if ((!(node_forced(fanout) & mask)) && (!(node_flag(fanout, IG_INSTACKFORCED) & mask))) {
							ep = node_fanout_ep(node, i);

							se.node = fanout;
							se.forced = ep.type == IGEDGE_B ? 1 : 0;
							implyStack.insert(se);

							if (se.forced) {
								node_flag(fanout, IG_INSTACKFORCED) |= mask;
							}
							node_flag(fanout, IG_INSTACK) |= mask;
						}
					}
					else {
						if ((!(node_mark(fanout) & mask)) && (!(node_flag(fanout, IG_INSTACK) & mask))) {
							// se.forced = 0 here
							se.node = fanout;
							implyStack.insert(se);

							node_flag(fanout, IG_INSTACK) |= mask;
						}
					}
				}
			}
			else {
				// insert the node into possible unjustified clause
				if (!(node_flag(node, IG_UNJUSTIFIED) & mask)) {
					node_flag(node, IG_UNJUSTIFIED) |= mask;
					unjustifiedAndNodes.insert(node);
				}
			}
		}
	}

	return returnVal;
}

inline void updateNodeMark(int mask, Array<IGNode *> &markedNodes) {
	int i;
	IGNode *node;
	
	for (i = 0; i < markedNodes.size; i++) {
		node = array_fetch(markedNodes, i);

		node_mark(node) |= mask;

		// TODO: potential problem, coz & mask may involve some nodes in lower
		// levels, not sure
		if (node_forced(node) & mask) {
			node_forced(node) |= mask;
		}
	}
}

inline void joinJustifications(int mask, Array<IGNode *> &just1MarkedNodes, Array<IGNode *> &just2MarkedNodes, Array<IGNode *> &markedNodes) {
	int i;
	IGNode *node;

	//cout << "just1 marked" << endl;
	for (i = 0; i < just1MarkedNodes.size; i++) {
		node = array_fetch(just1MarkedNodes, i);

		//cout << node_name(node) << endl;
		//printf("%x\n", node_mark(node));
		//printf("%x\n", mask);

		if ((node_mark(node) & mask) != mask) {
			node_mark(node) &= ~(mask);
			node_forced(node) &= ~(mask);
		}
		else {
			markedNodes.insert(node);
		}
		
		if ((node_forced(node) & mask) != mask) {
			node_forced(node) &= ~(mask);
		}
	
		//printf("%x\n", node_mark(node));
	}
	//cout << "just1 marked end" << endl;

	//cout << "just2 marked" << endl;
	for (i = 0; i < just2MarkedNodes.size; i++) {
		node = array_fetch(just2MarkedNodes, i);

		//cout << node_name(node) << endl;
		//printf("%x\n", node_mark(node));
		if ((node_mark(node) & mask) != mask) {
			node_mark(node) &= ~(mask);
			node_forced(node) &= ~(mask);
		}
		if ((node_forced(node) & mask) != mask) {
			node_forced(node) &= ~(mask);
		}
	}
	//cout << "just2 marked end" << endl;
}

inline void resetNodeMark(int mask, Array<IGNode *> &markedNodes) {
	int i;
	IGNode *node;
	
	for (i = 0; i < markedNodes.size; i++) {
		node = array_fetch(markedNodes, i);

		node_mark(node) &= ~(mask);
		node_forced(node) &= ~(mask);
	}
}

inline void resetNodeFlag(int mask, Array<IGNode *> &markedNodes, int flag) {
	int i;
	IGNode *node;
	
	for (i = 0; i < markedNodes.size; i++) {
		node = array_fetch(markedNodes, i);

		node_flag(node, flag) &= ~(mask);
	}
}

//static int rlearnImply_(IGNode *node, int forced, int r, unsigned int mask, Array<IGNode *> &markedNodes) {
static int rlearnImply_(IGNode *node, int r, unsigned int mask, Array<IGNode *> &markedNodes) {
	int result;
	int i, j, k;
	Array<IGNode *> *markedNodesImply = allocNodelist();
	Array<IGNode *> *unjustifiedAndNodes = allocNodelist();
	Array<IGNode *> *justifiedNodes;
	Array<IGNode *> *just1MarkedNodes, *just2MarkedNodes;
	IGNode *andNode, *andPred1, *andPred2, *markedPred, *just1, *just2, *peerAnd;
	IGNode *fanout;
	IGStackElement se;
	IGEdgeProperty ep;
	int just1Forced, just2Forced;
	unsigned int mask1, mask2;

	// TODO: check if there is forced problem
	// TODO: check if marking this node cause inconsistent
	// mark this node if node is not null
	if (node) {
		node_mark(node) |= mask;
		markedNodesImply->insert(node);

		//if (forced) {
			// mark this node forced
		//	node_forced(node) |= mask;
		//}

		foreach_fanout(node, i, fanout) {
			//if (forced) {
			//	if ((!(node_forced(fanout) & mask)) && (!(node_flag(fanout, IG_INSTACKFORCED) & mask))) {
			//		ep = node_fanout_ep(node, i);

			//		se.node = fanout;
			//		se.forced = ep.type == IGEDGE_B ? 1 : 0;
			//		implyStack.insert(se);

			//		if (se.forced) {
			//			node_flag(fanout, IG_INSTACKFORCED) |= mask;
			//		}
			//		node_flag(fanout, IG_INSTACK) |= mask;
			//	}
			//}
			//else {
				if ((!(node_mark(fanout) & mask)) && (!(node_flag(fanout, IG_INSTACK) & mask))) {
					se.forced = 0;
					se.node = fanout;
					implyStack.insert(se);

					node_flag(fanout, IG_INSTACK) |= mask;
				}
			//}
		}
	}

	result = imply_(mask, *markedNodesImply, *unjustifiedAndNodes);

	if (result == INCONSISTENT) {
		//cout << "inconsistent ------" << endl;
		//for (i = 0; i < (*markedNodesImply).size; i++) {
		//	cout << node_name((*markedNodesImply)[i]) << endl;
		//}
		//cout << "end inconsistent ------" << endl;
		resetNodeMark(mask, *markedNodesImply);
		freeNodelist(markedNodesImply);
		resetNodeFlag(mask, *unjustifiedAndNodes, IG_UNJUSTIFIED);
		freeNodelist(unjustifiedAndNodes);
		return INCONSISTENT;
	}

	if (r == rStop) {
		markedNodes.append(*markedNodesImply);
		freeNodelist(markedNodesImply);
		resetNodeFlag(mask, *unjustifiedAndNodes, IG_UNJUSTIFIED);
		freeNodelist(unjustifiedAndNodes);

		//cout << "top level reached" << endl;

		return CONSISTENT;
	}

	justifiedNodes = allocNodelist();
	// find unjustified clause
	for (i = 0; i < unjustifiedAndNodes->size; i++) {
		andNode = array_fetch(*unjustifiedAndNodes, i);

		// clear the mark
		node_flag(andNode, IG_UNJUSTIFIED) &= ~(mask);
		
		ASSERT(node_type(andNode) == IGNODE_AND, "error: %s is not a AND node", node_name(andNode).c_str());

		// both pred are marked --> andNode marked
		if (node_mark(andNode) & mask) {
			continue;
		}

		// check if the clause is justified
		if (node_flag(andNode, IG_JUSTIFIED) & mask) {
			continue;
		}

		andPred1 = node_fanin(andNode, 0);
		andPred2 = node_fanin(andNode, 1);

		//if (node_mark(andPred1) & mask) {
		//	markedPred = andPred1;
		//}
		//else {
		//	markedPred = andPred2;
		//}

		markedPred = (node_mark(andPred1) & mask) ? andPred1 : andPred2;
		
		just1 = node_fanout(andNode, 0);

		// only one node is marked, this is the start of a unjustified clause
		// get the other AND node
		just2 = NULL;
		for (j = 0; j < 2 && !just2; j++) {
			peerAnd = array_fetch(node_link(andNode), j);

			for (k = 0; k < 2; k++) {
				if (node_fanin(peerAnd, k) == markedPred) {
					just2 = node_fanout(peerAnd, 0);
					break;
				}
			}
		}

		ASSERT(just2, "error: unable to find second justification");
		ASSERT(just2 == node_fanout(peerAnd, 0), "error: justification bug");

		// just1 and just2 should not be marked
		if ((node_mark(just1) & mask) || (node_mark(just2) & mask)) {
			continue;
		}

		// check if just1 and just2 are marked as faulty
		if ((node_flag(just1, IG_INSTACK) & mask) || (node_flag(just2, IG_INSTACK) & mask)) {
			continue;
		}
		
		//cout << "just1: " << node_name(just1) << endl;
		//cout << "just2: " << node_name(just2) << endl;

		// calculate the forced status of the justifications
		//just1Forced = 0;
		//just2Forced = 0;
		//if (node_forced(markedPred) & mask) {
		//	ep = node_fanout_ep(andNode, 0);

		//	if (ep.type == IGEDGE_B) {
		//		just1Forced = 1;
		//	}

		//	ep = node_fanout_ep(peerAnd, 0);

			//if (ep.type == IGEDGE_B && just1Forced) {
		//	if (ep.type == IGEDGE_B) {
		//		just2Forced = 1;
		//	}
		//	else {
				//just1Forced = 0;
		//	}
		//}

		mask1 = (mask << power2[r - 1]) & mask;
		//printf("mask1: %x\n", mask1);

		just1MarkedNodes = allocNodelist();
		//result = rlearnImply_(just1, just1Forced, r - 1, mask1, *just1MarkedNodes);
		result = rlearnImply_(just1, r - 1, mask1, *just1MarkedNodes);

		if (result == INCONSISTENT) {
			//result = rlearnImply_(just2, just2Forced, r - 1, mask1, *just1MarkedNodes);
			result = rlearnImply_(just2, r - 1, mask1, *just1MarkedNodes);

			if (result == INCONSISTENT) {
				// reset node marked during this invocation
				resetNodeMark(mask, *markedNodesImply);
				freeNodelist(markedNodesImply);

				// clear the unjustified flags on nodes
				resetNodeFlag(mask, *unjustifiedAndNodes, IG_UNJUSTIFIED);
				freeNodelist(unjustifiedAndNodes);
				
				// clear the justified flags on nodes
				resetNodeFlag(mask, *justifiedNodes, IG_JUSTIFIED);
				freeNodelist(justifiedNodes);
				
				freeNodelist(just1MarkedNodes);
				return INCONSISTENT;
			}

			updateNodeMark(mask, *just1MarkedNodes);

			// justification 2 is the only consistent justification
			markedNodesImply->append(*just1MarkedNodes);
			freeNodelist(just1MarkedNodes);
			// mark both AND node in this clause as justified
			node_flag(andNode, IG_JUSTIFIED) |= mask;
			justifiedNodes->insert(andNode);
			node_flag(peerAnd, IG_JUSTIFIED) |= mask;
			justifiedNodes->insert(peerAnd);
			// continue to next justification
			continue;
		}

		just2MarkedNodes = allocNodelist();
		mask2 = (mask >> power2[r - 1]) & mask;

		//printf("mask2: %x\n", mask2);

		//result = rlearnImply_(just2, just2Forced, r - 1, mask2, *just2MarkedNodes);
		result = rlearnImply_(just2, r - 1, mask2, *just2MarkedNodes);

		if (result == INCONSISTENT) {
			updateNodeMark(mask, *just1MarkedNodes);

			// justification 1 is the only consistent justification
			markedNodesImply->append(*just1MarkedNodes);
			freeNodelist(just1MarkedNodes);
			freeNodelist(just2MarkedNodes);
			// mark both AND node in this clause as justified
			node_flag(andNode, IG_JUSTIFIED) |= mask;
			justifiedNodes->insert(andNode);
			node_flag(peerAnd, IG_JUSTIFIED) |= mask;
			justifiedNodes->insert(peerAnd);
			// continue to next justification
			continue;
		}
		
		// join both justification and reset nodes marked in lower levels
		joinJustifications(mask, *just1MarkedNodes, *just2MarkedNodes, *markedNodesImply);
			
		freeNodelist(just1MarkedNodes);
		freeNodelist(just2MarkedNodes);
		// mark both AND node in this clause as justified
		node_flag(andNode, IG_JUSTIFIED) |= mask;
		justifiedNodes->insert(andNode);
		node_flag(peerAnd, IG_JUSTIFIED) |= mask;
		justifiedNodes->insert(peerAnd);
		// continue to next justification
	}

	markedNodes.append(*markedNodesImply);

	// clear the justified flags on nodes
	resetNodeFlag(mask, *justifiedNodes, IG_JUSTIFIED);

	freeNodelist(markedNodesImply);
	freeNodelist(unjustifiedAndNodes);
	freeNodelist(justifiedNodes);
	return CONSISTENT;
}

// Exported functions

void IGEndImply() {
	while (_nodelists.size) {
		delete _nodelists.remove();
	}
}

int IGMarkInitNodes(Array<IGNode *> &initNodes, Array<IGNode *> &markedNodes) {
	int i, j, mask = INIT_MASK;
	IGNode *node, *fanout, *conflict;
	IGStackElement se;
	int ret = CONSISTENT;
	
	for (i = 0; i < initNodes.size; i++) {
		node = array_fetch(initNodes, i);

		if (!(node_mark(node) & mask)){
			// mark this node
			node_mark(node) |= mask;
			markedNodes.insert(node);

			// check for conflict
			for (j = 0; j < node_link(node).size; j++) {
				conflict = array_fetch(node_link(node), j);
				if (node_mark(conflict) & mask) {
					ret = INCONSISTENT;
				}
			}

			// insert all successor into stack
			foreach_fanout(node, j, fanout) {
				if ((!(node_forced(fanout) & mask)) && (!(node_flag(fanout, IG_INSTACK) & mask))) {
					se.forced = 0;
					se.node = fanout;
					implyStack.insert(se);

					node_flag(fanout, IG_INSTACK) |= mask;
				}
			}
		}
	}

	return ret;
}

int IGMarkInitNodesForced(Array<IGNode *> &initNodes, Array<IGNode *> &markedNodes) {
	int i, j, mask = INIT_MASK;
	IGNode *node, *fanout, *conflict;
	IGStackElement se;
	IGEdgeProperty ep;
	int ret = CONSISTENT;

	for (i = 0; i < initNodes.size; i++) {
		node = array_fetch(initNodes, i);

		if (!(node_mark(node) & mask)){
			// mark this node
			node_mark(node) |= mask;
			markedNodes.insert(node);

			// mark this node as forced
			node_forced(node) |= mask;

			// check for conflict
			for (j = 0; j < node_link(node).size; j++) {
				conflict = array_fetch(node_link(node), j);
				if (node_mark(conflict) & mask) {
					ret = INCONSISTENT;
				}
			}

			// insert all successor into stack
			foreach_fanout(node, j, fanout) {
				if ((!(node_forced(fanout) & mask)) && (!(node_flag(fanout, IG_INSTACKFORCED) & mask))) {
					ep = node_fanout_ep(node, j);

					se.node = fanout;
					se.forced = ep.type == IGEDGE_B ? 1 : 0;
					implyStack.insert(se);

					if (se.forced) {
						node_flag(fanout, IG_INSTACKFORCED) |= mask;
					}
					node_flag(fanout, IG_INSTACK) |= mask;
				}
			}
		}
	}

	return ret;
}

void IGMarkFaultyNodes(Array<IGNode *> &initNodes) {
	int mask = INIT_MASK;
	int i;
	IGNode *node;
	
	for (i = 0; i < initNodes.size; i++) {
		node = array_fetch(initNodes, i);
		node_flag(node, IG_INSTACK) |= mask;
		node_flag(node, IG_INSTACKFORCED) |= mask;
	}
}

void IGUnmarkFaultyNodes(Array<IGNode *> &initNodes) {
	int mask = INIT_MASK;
	int i;
	IGNode *node;
	
	for (i = 0; i < initNodes.size; i++) {
		node = array_fetch(initNodes, i);
		node_flag(node, IG_INSTACK) &= ~(mask);
		node_flag(node, IG_INSTACKFORCED) &= ~(mask);
	}
}

void IGStopForwardPropagation(IGNode *node) {
	int mask = INIT_MASK;
	int i;
	IGNode *fanout;
	IGEdgeProperty ep;
	
	foreach_fanout(node, i, fanout) {
		ep = node_fanout_ep(node, i);

		if (ep.type == IGEDGE_F || ep.type == IGEDGE_O) {
			node_flag(fanout, IG_INSTACK) |= mask;
			node_flag(fanout, IG_INSTACKFORCED) |= mask;
		}
	}
}

void IGAllowForwardPropagation(IGNode *node) {
	int mask = INIT_MASK;
	int i;
	IGNode *fanout;
	IGEdgeProperty ep;
	
	foreach_fanout(node, i, fanout) {
		ep = node_fanout_ep(node, i);

		if (ep.type == IGEDGE_F || ep.type == IGEDGE_O) {
			node_flag(fanout, IG_INSTACK) &= ~(mask);
			node_flag(fanout, IG_INSTACKFORCED) &= ~(mask);
		}
	}
}

void IGStopBackwardPropagation(IGNode *node) {
	int mask = INIT_MASK;
	int i;
	IGNode *fanout;
	IGEdgeProperty ep;
	
	foreach_fanout(node, i, fanout) {
		ep = node_fanout_ep(node, i);

		if (ep.type == IGEDGE_B) {
			node_flag(fanout, IG_INSTACK) |= mask;
			node_flag(fanout, IG_INSTACKFORCED) |= mask;
		}
	}
}

void IGAllowBackwardPropagation(IGNode *node) {
	int mask = INIT_MASK;
	int i;
	IGNode *fanout;
	IGEdgeProperty ep;
	
	foreach_fanout(node, i, fanout) {
		ep = node_fanout_ep(node, i);

		if (ep.type == IGEDGE_B) {
			node_flag(fanout, IG_INSTACK) &= ~(mask);
			node_flag(fanout, IG_INSTACKFORCED) &= ~(mask);
		}
	}
}

int IGImply(int mask, Array<IGNode *> &markedNodes) {
	Array<IGNode *> *dummy = allocNodelist();
	int result = imply_(mask, markedNodes, *dummy);

	freeNodelist(dummy);
	return result;
}

int IGRlearnImply(int r, Array<IGNode *> &markedNodes) {
	rStop = MAX_BIT - r;
	//return rlearnImply_(NULL, 0, MAX_BIT, INIT_MASK, markedNodes);
	return rlearnImply_(NULL, MAX_BIT, INIT_MASK, markedNodes);
}

void IGResetImply(Array<IGNode *> &markedNodes) {
	int i;
	IGNode *node;
	
	for (i = 0; i < markedNodes.size; i++) {
		node = array_fetch(markedNodes, i);

		node_mark(node) = 0;
		node_forced(node) = 0;
	}
}

void IGClearImplyStack() {
	int mask = INIT_MASK;
	IGStackElement se;

	while (implyStack.size > 0) {
		// clear all stack marks
		se = implyStack.remove();
		node_flag(se.node, IG_INSTACK) &= ~(mask);
		if (se.forced) {
			node_flag(se.node, IG_INSTACKFORCED) &= ~(mask);
		}
	}
}
