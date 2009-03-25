#include "array.h"
#include "rewire.h"

#undef array_fetch
#undef foreach_fanin
#undef foreach_fanout
#undef foreach_node

#include "sis_header/sis.h"
#include "sis_interface.h"
#include "rewire_c.h"

BNetwork *sisnet2BNetwork(network_t *net);

static network_t *currentNetwork = NULL;
static BNetwork *currentBNetwork = NULL;
static IGNetwork *currentIGNetwork = NULL;

extern "C" int rewire_find_aw(node_t *twsrc, node_t *twdst, int rlevel, array_t *aw) {
	Array<RewireAW> awArray;
	int ret;
	int i;
	BNode *src, *dst;
	BNode *currentInternal, *nextInternal;
	BNode *fanin;
	BNode *dstFound;
	rewire_aw_t aw_found;

	src = currentBNetwork->findNode(twsrc->name);
	dst = currentBNetwork->findNode(twdst->name);

	currentInternal = dst;
	nextInternal = NULL;
	dstFound = NULL;

	//trace("starting %s->%s\n", twsrc->name, twdst->name);
	while (!dstFound) {
		// check each fanin to see if src connect to it
		// else go to next internal node
		for (i = 0; i < currentInternal->in.size && ((fanin = currentInternal->in.space[i]) != NULL); i++) {
			if (fanin == src){
				dstFound = currentInternal;
			}

			//if (fanin->p.linkedNode != NULL) {
			//	cout << fanin->p.linkedNode->name->c_str() << endl;
			//}
			//cout << fanin->name->c_str() << endl;

			if (fanin->p.linkedNode == dst) {
				nextInternal = fanin;
			}
		}

		if ((!dstFound && nextInternal == currentInternal) || currentInternal == NULL) {
			trace("failed to find interal node for %s->%s\n", twsrc->name, twdst->name);
			return REDUNDANT;
		}

		currentInternal = nextInternal;
	}
	
	ret = rewireFindAW(src, dstFound, rlevel, awArray);

	for (i = 0; i < awArray.size; i++) {
		aw_found.src = network_find_node(currentNetwork, (char *) awArray.space[i].src->name->c_str());
		aw_found.dst = network_find_node(currentNetwork, (char *) awArray.space[i].dst->name->c_str());
		aw_found.polarity = awArray.space[i].polarity;
		aw_found.dst_function = awArray.space[i].dstFunction == BNODE_AND ? NODE_AND : NODE_OR;
		array_insert_last(rewire_aw_t, aw, aw_found);
	}

	return ret;
}

extern "C" void rewire_network_change(network_t *network) {
	if (currentIGNetwork != NULL) {
		delete currentIGNetwork;
	}

	if (currentBNetwork != NULL) {
		delete currentBNetwork;
	}

	currentNetwork = network;

	currentBNetwork = sisnet2BNetwork(network);
	currentBNetwork->assignDominator();
	currentIGNetwork = new IGNetwork(currentBNetwork);
}

extern "C" void rewire_end() {
	if (currentIGNetwork != NULL) {
		delete currentIGNetwork;
	}

	if (currentBNetwork != NULL) {
		delete currentBNetwork;
	}

	currentNetwork = NULL;
	currentIGNetwork = NULL;
	currentBNetwork = NULL;

	rewireEndImply();
}
