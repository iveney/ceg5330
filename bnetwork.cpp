#include "bnetwork.h"

void BNetwork::assignDominatorRecur(BNode *node) {
	int i, j, findIndex;
	BNode *fanout;
	Array<BNode *> temp;
	
#ifdef DEBUG
	if (node_num_fanout(node) == 0){
		if (node_type(node) == BNODE_PO){
			EXIT("error: PO encountered in recursion\n");
		}

		//trace("warning: node %s has no fanout\n", node->name);
		// insert itself as dominator
		bnode_abs_dom(node).insert(node);
		return;
	}
#endif
	
	foreach_fanout(node, i, fanout){
		if (!(bnode_flag(fanout) & BNODE_ABSDOM_VISITED)){
			assignDominatorRecur(fanout);
		}
	}

	// fanouts' dominators have been assigned.
	// assign this node's dominators as intersection of fanout's dominators
	foreach_fanout(node, i, fanout){
		if (i == 0) {
			bnode_abs_dom(node).append(bnode_abs_dom(fanout));
		}
		else {
			temp.clear();
			for (j = 0; j < bnode_abs_dom(node).size; j++) {
				findIndex = bnode_abs_dom(fanout).find(bnode_abs_dom(node)[j]);

				if (findIndex != -1) {
					temp.insert(bnode_abs_dom(node)[j]);
				}
			}
			bnode_abs_dom(node).clear();
			bnode_abs_dom(node).append(temp);
		}
	}

	// assign itself as its dominator
	bnode_abs_dom(node).insert(node);
	bnode_abs_dom(node).sort();
	
	bnode_flag(node) |= BNODE_ABSDOM_VISITED;
}
