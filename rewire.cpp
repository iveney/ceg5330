#include <string>
#include <set>
#include <map>
#include "array.h"
#include "imply.h"
#include "rewire.h"

#define DEBUG_SHOW_TRACE
//#undef DEBUG_SHOW_TRACE
#include "debug.h"

#define DEFAULT_LIST_SIZE 20

#define allocBNodelist() _bnodelists.size == 0 ? new Array<BNode *>(DEFAULT_LIST_SIZE) : _bnodelists.remove()
#define freeBNodelist(list) (list)->clear(),_bnodelists.insert((list)), (list) = NULL
//#define freeBNodelist(list) (list)->clear(),_bnodelists.insert((list))

#define allocIGNodelist() _ignodelists.size == 0 ? new Array<IGNode *>(DEFAULT_LIST_SIZE) : _ignodelists.remove()
#define freeIGNodelist(list) (list)->clear(),_ignodelists.insert((list)), (list) = NULL
//#define freeIGNodelist(list) (list)->clear(),_ignodelists.insert((list))

static Array<Array<BNode *> *> _bnodelists;
static Array<Array<IGNode *> *> _ignodelists;

//char debug_buf[1048576];
//int debug_buf_s = 0;

inline static int isAllTwDstFaninFanoutCone(BNode* twDst, set<BNode*>& twFanoutCone);

static void rewireMarkFanoutCone(BNode *node, Array<BNode *> &fanoutMarkedNodes) {
	BNode *fanout;
	int i;

	if (bnode_flag(node) & BNODE_FANOUT_CONE_MARK){
		return;
	}

	bnode_flag(node) |= BNODE_FANOUT_CONE_MARK;
	fanoutMarkedNodes.insert(node);

	foreach_fanout(node, i, fanout){
		rewireMarkFanoutCone(fanout, fanoutMarkedNodes);
	}
}

static inline void resetFanoutConeMark(Array<BNode *> &fanoutMarkedNodes) {
	int i;
	BNode *node;
	
	for (i = 0; i < fanoutMarkedNodes.size; i++) {
		node = array_fetch(fanoutMarkedNodes, i);
		bnode_flag(node) &= ~(BNODE_FANOUT_CONE_MARK);
	}
}

static inline void rewireAssignControlValue(BNode *src, BNode *dst, int stucktype, Array<IGNode *> &markedNodes){
	int dstValue;
	int faninIndex = node_get_fanin_index(dst, src);
	Array<IGNode *> *initNodes = allocIGNodelist();

	// find the value should be assigned to the immediate fanout node
	if (bnode_fanin_polarity(dst, faninIndex)){
		dstValue = stucktype;
	}
	else {
		dstValue = !stucktype;
	}

	initNodes->clear();

	if (stucktype) {
		initNodes->insert(bnode_ignodes(src).g0);
		initNodes->insert(bnode_ignodes(src).f0);
	}
	else {
		initNodes->insert(bnode_ignodes(src).g1);
		initNodes->insert(bnode_ignodes(src).f1);
	}
	
	IGMarkInitNodesForced(*initNodes, markedNodes);

	//cout << "contorl value 1" << endl;
	//for (int i = 0; i < (*initNodes).size; i++) {
	//	cout << node_name((*initNodes)[i]) << endl;
	//}

	initNodes->clear();

	if (dstValue) {
		initNodes->insert(bnode_ignodes(dst).g0);
		initNodes->insert(bnode_ignodes(dst).f1);
	}
	else {
		initNodes->insert(bnode_ignodes(dst).g1);
		initNodes->insert(bnode_ignodes(dst).f0);
	}

	IGMarkInitNodes(*initNodes, markedNodes);

	//cout << "contorl value 2" << endl;
	//for (int i = 0; i < (*initNodes).size; i++) {
	//	cout << node_name((*initNodes)[i]) << endl;
	//}

	freeIGNodelist(initNodes);
}

static inline int rewireAssignSensitizeValue(BNode *src, BNode *dst, Array<IGNode *> &markedNodes) {
	int ret = CONSISTENT;
	Array<BNode *> &absDom = bnode_abs_dom(dst);
	
	// assume rewireMarkFanoutCone is called

	// assign sensitizing value
	if (absDom.size != 0) {
		Array<IGNode *> *initNodes = allocIGNodelist();
		BNode *node;
		BNode *fanin;

		int i, j;

		for (i = 0; i < absDom.size; i++) {
			node = array_fetch(absDom, i);
			
			//trace("preassigning dominator %s\n", node->name->c_str());

			// special case for dst, since the wire src->dst will have dst as the
			// absolute dominator but src should not be considered as the side
			// input and shouldn't be assigned a sensitizing value
			if (node == dst) {
				bnode_flag(src) |= BNODE_FANOUT_CONE_MARK;
			}

			switch (node_type(node)){
				case BNODE_AND:
					foreach_fanin(node, j, fanin) {
						// the fanin is on the sensitizing path (not side input)
						if (bnode_flag(fanin) & BNODE_FANOUT_CONE_MARK) {
							continue;
						}

						int temp;

						if (bnode_fanin_polarity(node, j)) {
							// normal edge 
							initNodes->insert(bnode_ignodes(fanin).g1);
							initNodes->insert(bnode_ignodes(fanin).f1);
							temp = 1;
						}
						else {
							// complemented edge
							initNodes->insert(bnode_ignodes(fanin).g0);
							initNodes->insert(bnode_ignodes(fanin).f0);
							temp = 0;
						}
						//trace("preassigning side input %s %d\n", fanin->name->c_str(), temp);
					}
					break;

				case BNODE_OR:
					foreach_fanin(node, j, fanin) {
						// the fanin is on the sensitizing path (not side input)
						if (bnode_flag(fanin) & BNODE_FANOUT_CONE_MARK) {
							continue;
						}

						int temp;
						if (bnode_fanin_polarity(node, j)) {
							// normal edge 
							initNodes->insert(bnode_ignodes(fanin).g0);
							initNodes->insert(bnode_ignodes(fanin).f0);
							temp = 0;
						}
						else {
							// complemented edge
							initNodes->insert(bnode_ignodes(fanin).g1);
							initNodes->insert(bnode_ignodes(fanin).f1);
							temp = 1;
						}
						//trace("preassigning side input %s %d\n", fanin->name->c_str(), temp);
					}
					break;

				default:
					// buffer or complex gate or PI or PO
					break;
			}

			if (node == dst) {
				bnode_flag(src) &= ~(BNODE_FANOUT_CONE_MARK);
			}
		}

		//trace("side inputs mark:\n");
		//for (int k = 0; k < (*initNodes).size; k++) {
		//	cout << node_name((*initNodes)[k]) << endl;
		//}
		//cout << "-------------" << endl;

		ret = IGMarkInitNodesForced(*initNodes, markedNodes);
		
		freeIGNodelist(initNodes);
	}

	if (ret == INCONSISTENT) {
		IGClearImplyStack();
	}

	return ret;
}

static inline int rewireAssignCWSensitizeValue(BNode *dst, Array<IGNode *> &markedNodes) {
	int ret = CONSISTENT;
	Array<BNode *> &absDom = bnode_abs_dom(dst);
	
	// assume rewireMarkFanoutCone is called

	// assign sensitizing value
	if (absDom.size != 0) {
		Array<IGNode *> *initNodes = allocIGNodelist();
		BNode *node;
		BNode *fanin;

		int i, j;

		for (i = 0; i < absDom.size; i++) {
			node = array_fetch(absDom, i);
			
			//debug_buf_s += sprintf(debug_buf + debug_buf_s, "preassigning dominator %s\n", node->name->c_str());
			
			// special case for dst, since the wire src->dst will have dst as the
			// absolute dominator but src should not be considered as the side
			// input and shouldn't be assigned a sensitizing value
			if (node == dst) {
				continue;
			}

			switch (node_type(node)){
				case BNODE_AND:
					foreach_fanin(node, j, fanin) {
						// the fanin is on the sensitizing path (not side input)
						if (bnode_flag(fanin) & BNODE_FANOUT_CONE_MARK) {
							continue;
						}

						int temp = -1;

						if (bnode_fanin_polarity(node, j)) {
							// normal edge 
							initNodes->insert(bnode_ignodes(fanin).g1);
							initNodes->insert(bnode_ignodes(fanin).f1);
							temp = 1;
						}
						else {
							// complemented edge
							initNodes->insert(bnode_ignodes(fanin).g0);
							initNodes->insert(bnode_ignodes(fanin).f0);
							temp = 0;
						}
						//debug_buf_s += sprintf(debug_buf + debug_buf_s, "preassigning side input %s %d\n", fanin->name->c_str(), temp);
					}
					break;

				case BNODE_OR:
					foreach_fanin(node, j, fanin) {
						// the fanin is on the sensitizing path (not side input)
						if (bnode_flag(fanin) & BNODE_FANOUT_CONE_MARK) {
							continue;
						}

						int temp = -1;
						if (bnode_fanin_polarity(node, j)) {
							// normal edge 
							initNodes->insert(bnode_ignodes(fanin).g0);
							initNodes->insert(bnode_ignodes(fanin).f0);
							temp = 0;
						}
						else {
							// complemented edge
							initNodes->insert(bnode_ignodes(fanin).g1);
							initNodes->insert(bnode_ignodes(fanin).f1);
							temp = 1;
						}
						//debug_buf_s += sprintf(debug_buf + debug_buf_s, "preassigning side input %s %d\n", fanin->name->c_str(), temp);
					}
					break;

				default:
					//debug_buf_s += sprintf(debug_buf + debug_buf_s, "node %s is complex\n", node->name->c_str());
					// buffer or complex gate or PI or PO
					break;
			}
		}

		//debug_buf_s += sprintf(debug_buf + debug_buf_s, "side inputs mark:\n");
		//for (int k = 0; k < (*initNodes).size; k++) {
		//	debug_buf_s += sprintf(debug_buf + debug_buf_s, "%s\n", node_name((*initNodes)[k]).c_str());
		//}
		//debug_buf_s += sprintf(debug_buf + debug_buf_s, "-------------\n");

		ret = IGMarkInitNodes(*initNodes, markedNodes);
		
		freeIGNodelist(initNodes);
	}

	if (ret == INCONSISTENT) {
		IGClearImplyStack();
	}

	return ret;
}

static inline void markFaultyNodes(BNode *src) {
	Array<IGNode *> *initNodes = allocIGNodelist();
	
	// mark src and dst ig nodes as faulty so they will not cause conflict
	initNodes->insert(bnode_ignodes(src).g0);
	initNodes->insert(bnode_ignodes(src).g1);
	initNodes->insert(bnode_ignodes(src).f0);
	initNodes->insert(bnode_ignodes(src).f1);

	IGMarkFaultyNodes(*initNodes);

	freeIGNodelist(initNodes);
}

static inline void unmarkFaultyNodes(BNode *src) {
	Array<IGNode *> *initNodes = allocIGNodelist();
	
	initNodes->insert(bnode_ignodes(src).g0);
	initNodes->insert(bnode_ignodes(src).g1);
	initNodes->insert(bnode_ignodes(src).f0);
	initNodes->insert(bnode_ignodes(src).f1);

	IGUnmarkFaultyNodes(*initNodes);

	freeIGNodelist(initNodes);
}

static inline int rewireTestStuckFault(BNode *src, BNode *dst, int stucktype, Array<IGNode *> &markedNodes, Array<BNode *> &omaMarkedNodes, Array<BNode *> &nomaMarkedNodes, int r) {
	// call rewireMarkFanoutCone(dst) before
	int ret, retVal = NONREDUNDANT;
	int i, newMark;
	IGNode *ignode;

	//trace("testing %s->%s stuck at %d fault\n", node_name(src).c_str(), node_name(dst).c_str(), stucktype);

#ifdef DEBUG
	if (node_get_fanin_index(dst, src) == -1){
		ASSERT(0, "error: wire %s->%s does not exist\n", node_name(src).c_str(), node_name(dst).c_str());
	}
#endif

	// assign sensitizing values
	// prevent the implication going through the faulty node
	// TODO: check if less aw
	// TODO: check if only stop forward propagate can result in more wires
	// TODO: or just mark single value enough?
	markFaultyNodes(dst);

	ret = rewireAssignSensitizeValue(src, dst, markedNodes);

	if (ret == INCONSISTENT) {
		retVal = REDUNDANT;
	}
	
	if (retVal != REDUNDANT) {
		
		ret = IGRlearnImply(r, markedNodes);


		if (ret == INCONSISTENT) {
			retVal = REDUNDANT;
		}
	}
	
	unmarkFaultyNodes(dst);

	if (retVal != REDUNDANT) {
		for (i = 0; i < markedNodes.size; i++) {
			ignode = array_fetch(markedNodes, i);

			if (node_type(ignode) == IGNODE_SIGNAL && (!(bnode_flag(node_bnode(ignode)) & BNODE_IS_OMA))) {
				bnode_flag(node_bnode(ignode)) |= BNODE_IS_OMA;
				omaMarkedNodes.insert(node_bnode(ignode));
			}
		}

		newMark = i;

		// TODO: check if only stop forward propagate can result in more wires
		markFaultyNodes(src);
		markFaultyNodes(dst);

		// assign contorlling values
		rewireAssignControlValue(src, dst, stucktype, markedNodes);

		ret = IGRlearnImply(r, markedNodes);
		
		unmarkFaultyNodes(src);
		unmarkFaultyNodes(dst);

		if (ret == INCONSISTENT) {
			retVal = REDUNDANT;
		}
	}

	if (retVal != REDUNDANT) {
		for (i = newMark; i < markedNodes.size; i++) {
			ignode = array_fetch(markedNodes, i);

			if (node_type(ignode) == IGNODE_SIGNAL && (!(bnode_flag(node_bnode(ignode)) & BNODE_IS_NOMA))) {
				bnode_flag(node_bnode(ignode)) |= BNODE_IS_NOMA;
				nomaMarkedNodes.insert(node_bnode(ignode));
			}
		}
	}

	//trace("finished preassignment\n");

	return retVal;
}

static inline void rewireClearFlag(Array<BNode *> &markedNodes, int flag) {
	int i;
	BNode *node;

	for (i = 0; i < markedNodes.size; i++){
		node = array_fetch(markedNodes, i);

		bnode_flag(node) &= ~flag;
	}
}

void rewireEndImply() {
	while (_bnodelists.size) {
		delete _bnodelists.remove();
	}

	while (_ignodelists.size) {
		delete _ignodelists.remove();
	}

	IGEndImply();
}

static inline int rewireTestCWDst(BNode *dst, BNodeType function, Array<IGNode *> &markedNodes, int r) {
	int ret = NONREDUNDANT;
	int result;
	Array<IGNode *> *initNodes = allocIGNodelist();

	if (function == BNODE_AND) {
		// stop g0 backward
		// stop g1 forward
		IGStopBackwardPropagation(bnode_ignodes(dst).g0);
		IGStopForwardPropagation(bnode_ignodes(dst).g1);
		initNodes->insert(bnode_ignodes(dst).f1);
	}
	else if (function == BNODE_OR) {
		// stop g1 backward
		// stop g0 forward
		IGStopBackwardPropagation(bnode_ignodes(dst).g1);
		IGStopForwardPropagation(bnode_ignodes(dst).g0);
		initNodes->insert(bnode_ignodes(dst).f0);
	}
	else {
		trace("error: unknown dst function type\n");
		freeIGNodelist(initNodes);
		return NONREDUNDANT;
	}

	initNodes->insert(bnode_ignodes(dst).g0);
	initNodes->insert(bnode_ignodes(dst).g1);

	IGMarkInitNodes(*initNodes, markedNodes);

	// assign sensitizing values
	result = rewireAssignCWSensitizeValue(dst, markedNodes);

	if (result == INCONSISTENT) {
		ret = REDUNDANT;
		//cout << "sensitization inconsistent" << endl;
		//printf("%s", debug_buf);
		//debug_buf_s = 0;
	}
	else {
		result = IGRlearnImply(r, markedNodes);
		//cout << "imply result: " << result << endl;

		if (result == INCONSISTENT) {
			ret = REDUNDANT;
			//cout << "imply inconsistent" << endl;
			//printf("%s", debug_buf);
			//debug_buf_s = 0;
		}
	}

	//debug_buf_s = 0;

	freeIGNodelist(initNodes);
	return ret;
}

static inline void rewireTestCWDstEnd(BNode *dst, BNodeType function) {
	// remove faulty marks
	if (function == BNODE_AND) {
		// allow g0 backward
		// allow g1 forward
		IGAllowBackwardPropagation(bnode_ignodes(dst).g0);
		IGAllowForwardPropagation(bnode_ignodes(dst).g1);
	}
	else if (function == BNODE_OR) {
		// allow g1 backward
		// allow g0 forward
		IGAllowBackwardPropagation(bnode_ignodes(dst).g1);
		IGAllowForwardPropagation(bnode_ignodes(dst).g0);
	}
}

static inline int rewireTestCWSrc(BNode *src, int stucktype, Array<IGNode *> &markedNodes, int r) {
	int result;
	int ret = NONREDUNDANT;
	Array<IGNode *> *initNodes = allocIGNodelist();
	
	// src control values
	if (stucktype) {
		// stk 1 ==> src = 0
		initNodes->insert(bnode_ignodes(src).g0);
		initNodes->insert(bnode_ignodes(src).f0);
	}
	else {
		// stk 0 ==> src = 1
		initNodes->insert(bnode_ignodes(src).g1);
		initNodes->insert(bnode_ignodes(src).f1);
	}

	IGMarkInitNodes(*initNodes, markedNodes);

	result = IGRlearnImply(r, markedNodes);
	if (result == INCONSISTENT) {
		ret = REDUNDANT;
	}

	freeIGNodelist(initNodes);
	return ret;
}

int rewireFindAW(BNode *twSrc, BNode *twDst, int rlevel, Array<RewireAW> &aw) {
	int faninIdx = node_get_fanin_index(twDst, twSrc);
	int stucktype;
	int result;
	int ret = REDUNDANT;
	
	Array<IGNode *> *markedNodes = allocIGNodelist();
	Array<IGNode *> *srcMarkedNodes = allocIGNodelist();
	Array<BNode *> *omaMarkedNodes = allocBNodelist();
	Array<BNode *> *nomaMarkedNodes = allocBNodelist();
	Array<BNode *> *fanoutMarkedNodes = allocBNodelist();

	Array<BNode *> *dstAnd = allocBNodelist();
	Array<BNode *> *dstOr = allocBNodelist();
	Array<BNode *> *src0 = allocBNodelist();
	Array<BNode *> *src1 = allocBNodelist();

	BNode *bnode;
	int i, j;
    set<BNode*> twFanoutConeSet;

	RewireAW awFound;

	// find the stuck at fault type of the tw
	switch (node_type(twDst)) {
		// if the node is AND
		case BNODE_AND:
		stucktype = bnode_fanin_polarity(twDst, faninIdx);
		break;
		
		// if the node is OR
      case BNODE_OR:
		stucktype = !bnode_fanin_polarity(twDst, faninIdx);
		break;

		default:
		trace("Node type of %s not supported\n", twDst->name->c_str());
		return REDUNDANT;
		break;
	}

	rewireMarkFanoutCone(twDst, *fanoutMarkedNodes);

	result = rewireTestStuckFault(twSrc, twDst, stucktype, *markedNodes, *omaMarkedNodes, *nomaMarkedNodes, rlevel);

	rewireClearFlag(*fanoutMarkedNodes, BNODE_FANOUT_CONE_MARK);//althought the marks are cleared, the list still exists    
    for (i = 0; i < (*fanoutMarkedNodes).size; i++)
    {
        BNode* fanoutNode = array_fetch(*fanoutMarkedNodes, i);
        twFanoutConeSet.insert(twFanoutConeSet.end(), fanoutNode);
    }

	fanoutMarkedNodes->clear();

	if (result != REDUNDANT) {
		ret = NONREDUNDANT;

		// mark abs dom
		for (i = 0; i < bnode_abs_dom(twDst).size; i++) {
			bnode = array_fetch(bnode_abs_dom(twDst), i);

			bnode_flag(bnode) |= BNODE_IS_ABSDOM;
		}

		// find the candidate set
		for (i = 0; i < (*omaMarkedNodes).size; i++) {
			bnode = array_fetch((*omaMarkedNodes), i);
			
			if (node_type(bnode) != BNODE_PI &&
                node_type(bnode) != BNODE_PO &&
                isAllTwDstFaninFanoutCone(bnode, twFanoutConeSet) == 0) {

				if (bnode_flag(bnode) & BNODE_IS_ABSDOM) {

					if (node_mark(bnode_ignodes(bnode).g0)) {
						// abs dom 0 ==> dst AND
						dstAnd->insert(bnode);
					}
					else if (node_mark(bnode_ignodes(bnode).g1)) {
						// abs dom 1 ==> dst OR
						dstOr->insert(bnode);
					}
					else {
						// TODO: check if faulty value can be useful
					}
				}
				else if (node_forced(bnode_ignodes(bnode).g0) && node_mark(bnode_ignodes(bnode).g0)) {
					// forced 0 ==> dst OR
					dstOr->insert(bnode);
				}
				else if (node_forced(bnode_ignodes(bnode).g1) && node_mark(bnode_ignodes(bnode).g1)) {
					// forced 1 ==> dst AND
					dstAnd->insert(bnode);
				}
				else {
					// TODO: check if faulty value can be useful
				}
			}
		}

		for (i = 0; i < (*nomaMarkedNodes).size; i++) {
			bnode = array_fetch((*nomaMarkedNodes), i);

			if (node_type(bnode) != BNODE_PI &&
                node_type(bnode) != BNODE_PO &&
                isAllTwDstFaninFanoutCone(bnode, twFanoutConeSet) == 0 ) {
				if (bnode_flag(bnode) & BNODE_IS_ABSDOM) {

					if (node_mark(bnode_ignodes(bnode).g0)) {
						// abs dom 0 ==> dst AND
						dstAnd->insert(bnode);
					}
					else if (node_mark(bnode_ignodes(bnode).g1)) {
						// abs dom 1 ==> dst OR
						dstOr->insert(bnode);
					}
					else {
						// TODO: check if faulty value can be useful
					}
				}
				else if (node_forced(bnode_ignodes(bnode).g0) && node_mark(bnode_ignodes(bnode).g0)) {
					// forced 0 ==> dst OR
					dstOr->insert(bnode);
				}
				else if (node_forced(bnode_ignodes(bnode).g1) && node_mark(bnode_ignodes(bnode).g1)) {
					// forced 1 ==> dst AND
					dstAnd->insert(bnode);
				}
				else {
					// TODO: check if faulty value can be useful
				}
			}

			if (node_type(bnode) != BNODE_PO) {
				if (!(bnode_flag(bnode) & BNODE_IS_ABSDOM)) {
					if (node_mark(bnode_ignodes(bnode).g0)) {
						// not OMA and 0 ==> src 0
						src0->insert(bnode);
					}
					else if (node_mark(bnode_ignodes(bnode).g1)) {
						// not OMA and 1 ==> src 1
						src1->insert(bnode);
					}
				}
			}
		}

		rewireClearFlag(bnode_abs_dom(twDst), BNODE_IS_ABSDOM);

		rewireClearFlag(*omaMarkedNodes, BNODE_IS_OMA);
		rewireClearFlag(*nomaMarkedNodes, BNODE_IS_NOMA);

		IGResetImply(*markedNodes);
		markedNodes->clear();

		// test each candidate wire
		{
			int passDst = 0;
			int passSrc = 0;
			Array<BNode *> *dst;
			Array<BNode *> *src;
			BNodeType cwFunction;
			int cwPolarity, cwStucktype;
			BNode *cwSrc, *cwDst;

			for (passDst = 0; passDst < 2; passDst++) {
				switch (passDst) {
					case 0:
						// src -----> OR gate
						dst = dstOr;
						cwFunction = BNODE_OR;
						break;

					case 1:
						// src -----> AND gate
						dst = dstAnd;
						cwFunction = BNODE_AND;
						break;

					default:
						break;
				}

				for (i = 0; i < (*dst).size; i++) {
					cwDst = array_fetch(*dst, i);

					rewireMarkFanoutCone(cwDst, *fanoutMarkedNodes);
					
					result = rewireTestCWDst(cwDst, cwFunction, *markedNodes, rlevel);

					for (passSrc = 0; passSrc < 2; passSrc++) {
						switch (passSrc) {
							case 0:
								src = src0;
								
								if (cwFunction == BNODE_OR) {
									// 0 -INV-> OR gate
									cwPolarity = 0;
								}
								else {
									// 0 -----> AND gate
									cwPolarity = 1;
								}
								
								cwStucktype = 1;
								break;

							case 1:
								src = src1;
								
								if (cwFunction == BNODE_OR) {
									// 1 -----> OR gate
									cwPolarity = 1;
								}
								else {
									// 1 -INV-> AND gate
									cwPolarity = 0;
								}
								
								cwStucktype = 0;
								break;

							default:
								break;
						}

						
						if (result == REDUNDANT) {
							for (j = 0; j < (*src).size; j++) {
								cwSrc = array_fetch(*src, j);

								// check for circuit acyclic
								if (bnode_flag(cwSrc) & BNODE_FANOUT_CONE_MARK){
									continue;
								}

								// discard target wire
								if (cwSrc == twSrc && cwDst == twDst) {
									continue;
								}

								awFound.src = cwSrc;
								awFound.dst = cwDst;
								awFound.polarity = cwPolarity;
								awFound.dstFunction = cwFunction;

								aw.insert(awFound);
							}
						}
						else {
							for (j = 0; j < (*src).size; j++) {
								cwSrc = array_fetch(*src, j);

								// check for circuit acyclic
								if (bnode_flag(cwSrc) & BNODE_FANOUT_CONE_MARK){
									continue;
								}

								// discard target wire
								if (cwSrc == twSrc && cwDst == twDst) {
									continue;
								}

								// discard wires that are from or to internal nodes
								if (cwSrc->p.linkedNode != NULL) {
									continue;
								}

								if (cwDst->p.linkedNode != NULL) {
									continue;
								}

								// check cw stuck-at faults
								if (rewireTestCWSrc(cwSrc, cwStucktype, *srcMarkedNodes, rlevel) == REDUNDANT) {
									// TODO MARK : save results in array
									//cout << "tw: " << node_name(twSrc) << " -> " << node_name(twDst) << " ";
									//cout << "aw: " << node_name(cwSrc) << (cwPolarity ? " -----> " : " -INV-> ") << node_name(cwDst) << " type: " << (cwFunction == BNODE_AND ? "AND" : "OR") << " polarity: " << cwPolarity << endl;
									awFound.src = cwSrc;
									awFound.dst = cwDst;
									awFound.polarity = cwPolarity;
									awFound.dstFunction = cwFunction;

									aw.insert(awFound);
								}

								IGResetImply(*srcMarkedNodes);
								srcMarkedNodes->clear();
							}
						}
					}

					rewireTestCWDstEnd(cwDst, cwFunction);

					IGResetImply(*markedNodes);
					markedNodes->clear();

					rewireClearFlag(*fanoutMarkedNodes, BNODE_FANOUT_CONE_MARK);
					fanoutMarkedNodes->clear();
				}
				
			}
		}
	}
	else {
		ret = REDUNDANT;
		rewireClearFlag(*omaMarkedNodes, BNODE_IS_OMA);
		rewireClearFlag(*nomaMarkedNodes, BNODE_IS_NOMA);
		IGResetImply(*markedNodes);
	}

	freeBNodelist(dstAnd);
	freeBNodelist(dstOr);
	freeBNodelist(src0);
	freeBNodelist(src1);

	freeIGNodelist(markedNodes);
	freeIGNodelist(srcMarkedNodes);
	freeBNodelist(omaMarkedNodes);
	freeBNodelist(nomaMarkedNodes);
	freeBNodelist(fanoutMarkedNodes);

    twFanoutConeSet.clear();

	return ret;
}

//determins if all fanins of twDst are located in the fanout cone of tw
inline static int isAllTwDstFaninFanoutCone(BNode* twDst, set<BNode*>& twFanoutConeSet)
{
    BNode* fanin;
    int i;
    foreach_fanin(twDst, i, fanin)
    {
        if (twFanoutConeSet.find(fanin) == twFanoutConeSet.end())
        {
            return 0;
        }
    }

    return 1;
}
