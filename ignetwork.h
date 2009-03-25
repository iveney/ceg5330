#ifndef __IGNETWORK_H
#define __IGNETWORK_H

#include <string>
#include "bnetwork.h"

#define DEBUG_SHOW_TRACE
//#undef DEBUG_SHOW_TRACE
#include "debug.h"

using namespace std;

class IGNetworkProperty;
class IGNodeProperty;
class IGEdgeProperty;

enum IGNodeType_ {IGNODE_AND, IGNODE_SIGNAL};
typedef enum IGNodeType_ IGNodeType;

enum IGImplyType_ {IGNODE_G0, IGNODE_G1, IGNODE_F0, IGNODE_F1};
typedef enum IGImplyType_ IGImplyType;

enum IGEdgeType_ {IGEDGE_F, IGEDGE_B, IGEDGE_O};
typedef enum IGEdgeType_ IGEdgeType;

typedef Node<IGNodeProperty, IGEdgeProperty> IGNode;

class IGNetworkProperty{
};

#define IG_INSTACK 0
#define IG_INSTACKFORCED 1
#define IG_UNJUSTIFIED 2
#define IG_JUSTIFIED 3
class IGNodeProperty {
	public:
	IGNodeType type;
	Array<IGNode *> link; // point to the conflicting nodes for SIGNAL nodes, nodes in the same clause for AND nodes
	int mark;
	int forced;
	int flag[4]; // flag[0] flag[1]: inStack, inStackForced. flag[2]: unjustified, flag[3]: justified
	BNode *bnode;
	IGImplyType implyType;

	IGNodeProperty() { mark = 0; forced = 0; flag[0] = 0; flag[1] = 0; flag[2] = 0; flag[3] = 0;};
};

class IGEdgeProperty {
	public:
	IGEdgeType type;
};

class IGNetwork: public Graph<IGNetworkProperty, IGNodeProperty, IGEdgeProperty> {
	void addSignal(string &name, IGNode *ignodes[]);
	void addBNodeSignal(BNode *node);
	void buildAnd2Subgraph(IGNode *in1, IGNode *in1_, IGNode *in2, IGNode *in2_, IGNode *out, IGNode *out_, Array<IGNode *> &associate);
	void buildOr2Subgraph(IGNode *in1, IGNode *in1_, IGNode *in2, IGNode *in2_, IGNode *out, IGNode *out_, Array<IGNode *> &associate);
	void buildBufSubgraph(IGNode *in, IGNode *in_, IGNode *out, IGNode *out_);
	void buildInvSubgraph(IGNode *in, IGNode *in_, IGNode *out, IGNode *out_);
	void buildImplySubgraph(BNode *node);
	public:

	BNetwork *bnet;
	
	IGNetwork(BNetwork *bnet);
	~IGNetwork();
};

// constructors-----------------------------------------
inline IGNetwork::IGNetwork(BNetwork *bnet_) {
	BNode *node;

	bnet = bnet_;

	foreach_node(bnet_, node) {
		addBNodeSignal(node);
	}

	foreach_node(bnet_, node) {
		buildImplySubgraph(node);
	}
};

inline IGNetwork::~IGNetwork() {
};

// member functions-------------------------------------
inline void IGNetwork::buildImplySubgraph(BNode *node) {
	// TODO: handle gates with more than 2 inputs
	if (node_num_fanin(node) > 2) {
		trace("node %s has more than 2 inputs. Not implemented\n", node_name(node).c_str());
		return;
	}

	IGNode *outG0 = node->p.ignodes.g0;
	IGNode *outG1 = node->p.ignodes.g1;
	IGNode *outF0 = node->p.ignodes.f0;
	IGNode *outF1 = node->p.ignodes.f1;

	switch (node_type(node)) {
		case BNODE_PI:
		break;

		case BNODE_AND:
		{
			BNode *in1 = node_fanin(node, 0);
			BNode *in2 = node_fanin(node, 1);

			IGNode *in1G0 = in1->p.ignodes.g0;
			IGNode *in1G1 = in1->p.ignodes.g1;
			IGNode *in1F0 = in1->p.ignodes.f0;
			IGNode *in1F1 = in1->p.ignodes.f1;

			IGNode *in2G0 = in2->p.ignodes.g0;
			IGNode *in2G1 = in2->p.ignodes.g1;
			IGNode *in2F0 = in2->p.ignodes.f0;
			IGNode *in2F1 = in2->p.ignodes.f1;

			// complemented edge, exchange g0, g1 and f0, f1
			if (node_fanin_ep(node, 0).polarity == 0) {
				IGNode *tmp = in1G0;
				in1G0 = in1G1;
				in1G1 = tmp;

				tmp = in1F0;
				in1F0 = in1F1;
				in1F1 = tmp;
			}

			if (node_fanin_ep(node, 1).polarity == 0) {
				IGNode *tmp = in2G0;
				in2G0 = in2G1;
				in2G1 = tmp;

				tmp = in2F0;
				in2F0 = in2F1;
				in2F1 = tmp;
			}

			buildAnd2Subgraph(in1G1, in1G0, in2G1, in2G0, outG1, outG0, node->p.ignodes.associatedNodes);
			buildAnd2Subgraph(in1F1, in1F0, in2F1, in2F0, outF1, outF0, node->p.ignodes.associatedNodes);
		}
		break;

		case BNODE_OR:
		{
			BNode *in1 = node_fanin(node, 0);
			BNode *in2 = node_fanin(node, 1);

			IGNode *in1G0 = in1->p.ignodes.g0;
			IGNode *in1G1 = in1->p.ignodes.g1;
			IGNode *in1F0 = in1->p.ignodes.f0;
			IGNode *in1F1 = in1->p.ignodes.f1;

			IGNode *in2G0 = in2->p.ignodes.g0;
			IGNode *in2G1 = in2->p.ignodes.g1;
			IGNode *in2F0 = in2->p.ignodes.f0;
			IGNode *in2F1 = in2->p.ignodes.f1;

			// complemented edge, exchange g0, g1 and f0, f1
			if (node_fanin_ep(node, 0).polarity == 0) {
				IGNode *tmp = in1G0;
				in1G0 = in1G1;
				in1G1 = tmp;

				tmp = in1F0;
				in1F0 = in1F1;
				in1F1 = tmp;
			}

			if (node_fanin_ep(node, 1).polarity == 0) {
				IGNode *tmp = in2G0;
				in2G0 = in2G1;
				in2G1 = tmp;

				tmp = in2F0;
				in2F0 = in2F1;
				in2F1 = tmp;
			}

			buildOr2Subgraph(in1G1, in1G0, in2G1, in2G0, outG1, outG0, node->p.ignodes.associatedNodes);
			buildOr2Subgraph(in1F1, in1F0, in2F1, in2F0, outF1, outF0, node->p.ignodes.associatedNodes);
		}
		break;

		case BNODE_INV:
		{
			BNode *in = node_fanin(node, 0);
			IGNode *inG0 = in->p.ignodes.g0;
			IGNode *inG1 = in->p.ignodes.g1;
			IGNode *inF0 = in->p.ignodes.f0;
			IGNode *inF1 = in->p.ignodes.f1;

			// complemented edge, exchange g0, g1 and f0, f1
			if (node_fanin_ep(node, 0).polarity == 0) {
				IGNode *tmp = inG0;
				inG0 = inG1;
				inG1 = tmp;

				tmp = inF0;
				inF0 = inF1;
				inF1 = tmp;
			}

			buildInvSubgraph(inG1, inG0, outG1, outG0);
			buildInvSubgraph(inF1, inF0, outF1, outF0);
		}
		break;

		// PO is the same as buffer
		case BNODE_PO:
		case BNODE_BUF:
		{
			BNode *in = node_fanin(node, 0);
			IGNode *inG0 = in->p.ignodes.g0;
			IGNode *inG1 = in->p.ignodes.g1;
			IGNode *inF0 = in->p.ignodes.f0;
			IGNode *inF1 = in->p.ignodes.f1;

			// complemented edge, exchange g0, g1 and f0, f1
			if (node_fanin_ep(node, 0).polarity == 0) {
				IGNode *tmp = inG0;
				inG0 = inG1;
				inG1 = tmp;

				tmp = inF0;
				inF0 = inF1;
				inF1 = tmp;
			}

			buildBufSubgraph(inG1, inG0, outG1, outG0);
			buildBufSubgraph(inF1, inF0, outF1, outF0);
		}
		break;
		
		default:
		case BNODE_COMPLEX:
		trace("error: unable to handle complex node %s in implication graph\n", node_name(node).c_str());
		break;
	}
};

inline void IGNetwork::addSignal(string &name, IGNode *ignodes[]) {
	// add G0, G1, F0, F1
	string nodeNameG0 = name + "_G0";
	string nodeNameG1 = name + "_G1";
	string nodeNameF0 = name + "_F0";
	string nodeNameF1 = name + "_F1";

	IGNode *nodeG0 = newNode(nodeNameG0);
	IGNode *nodeG1 = newNode(nodeNameG1);
	IGNode *nodeF0 = newNode(nodeNameF0);
	IGNode *nodeF1 = newNode(nodeNameF1);

	nodeG0->p.type = IGNODE_SIGNAL;
	nodeG1->p.type = IGNODE_SIGNAL;
	nodeF0->p.type = IGNODE_SIGNAL;
	nodeF1->p.type = IGNODE_SIGNAL;

	nodeG0->p.implyType = IGNODE_G0;
	nodeG1->p.implyType = IGNODE_G1;
	nodeF0->p.implyType = IGNODE_F0;
	nodeF1->p.implyType = IGNODE_F1;
	
	nodeG0->p.link.insert(nodeG1);
	nodeG1->p.link.insert(nodeG0);
	nodeF0->p.link.insert(nodeF1);
	nodeF1->p.link.insert(nodeF0);

	ignodes[0] = nodeG0;
	ignodes[1] = nodeG1;
	ignodes[2] = nodeF0;
	ignodes[3] = nodeF1;
};

inline void IGNetwork::addBNodeSignal(BNode *node) {
	IGNode *ignodes[4];
	// add the out signal of the BNode
	addSignal(*(node->name), ignodes);

	node->p.ignodes.associatedNodes.insert(ignodes[0]);
	node->p.ignodes.associatedNodes.insert(ignodes[1]);
	node->p.ignodes.associatedNodes.insert(ignodes[2]);
	node->p.ignodes.associatedNodes.insert(ignodes[3]);

	// associate node's IGAssociateProperty to IG nodes just added	
	node->p.ignodes.g0 = ignodes[0];
	node->p.ignodes.g1 = ignodes[1];
	node->p.ignodes.f0 = ignodes[2];
	node->p.ignodes.f1 = ignodes[3];

	ignodes[0]->p.bnode = node;
	ignodes[1]->p.bnode = node;
	ignodes[2]->p.bnode = node;
	ignodes[3]->p.bnode = node;
};

inline void IGNetwork::buildBufSubgraph(IGNode *in, IGNode *in_, IGNode *out, IGNode *out_) {
	buildInvSubgraph(in, in_, out_, out);
}

inline void IGNetwork::buildInvSubgraph(IGNode *in, IGNode *in_, IGNode *out, IGNode *out_) {
	// forward edge
	IGEdgeProperty ep;
	ep.type = IGEDGE_F;
	addEdge(in, out_, ep);
	addEdge(in_, out, ep);

	// backward edges
	ep.type = IGEDGE_B;
	addEdge(out, in_, ep);
	addEdge(out_, in, ep);
}

inline void IGNetwork::buildAnd2Subgraph(IGNode *in1, IGNode *in1_, IGNode *in2, IGNode *in2_, IGNode *out, IGNode *out_, Array<IGNode *> &associate) {
	IGEdgeProperty ep;
	
	// add the and nodes
	string nameAnd12 = *(in1->name) + "_" + *(in2->name) + "_" + *(out->name) + "_AND";
	string nameAnd13 = *(in1->name) + "_" + *(out_->name) + "_" + *(in2_->name) + "_AND";
	string nameAnd23 = *(in2->name) + "_" + *(out_->name) + "_" + *(in1_->name) + "_AND";
	
	IGNode *and12 = newNode(nameAnd12);
	IGNode *and13 = newNode(nameAnd13);
	IGNode *and23 = newNode(nameAnd23);

	and12->p.type = IGNODE_AND;
	and13->p.type = IGNODE_AND;
	and23->p.type = IGNODE_AND;

	// link the and nodes
	and12->p.link.insert(and13);
	and12->p.link.insert(and23);

	and13->p.link.insert(and23);
	and13->p.link.insert(and12);

	and23->p.link.insert(and12);
	and23->p.link.insert(and13);

	associate.insert(and12);
	associate.insert(and13);
	associate.insert(and23);

	// add the edges
	// forward edges
	ep.type = IGEDGE_F;
	addEdge(in1_, out_, ep);
	addEdge(in2_, out_, ep);
	addEdge(in1, and12, ep);
	addEdge(in2, and12, ep);
	addEdge(and12, out, ep);

	// backward edges
	ep.type = IGEDGE_B;
	addEdge(out, in1, ep);
	addEdge(out, in2, ep);
	addEdge(out_, and13, ep);
	addEdge(out_, and23, ep);
	addEdge(and23, in1_, ep);
	addEdge(and13, in2_, ep);

	// other edges
	ep.type = IGEDGE_O;
	addEdge(in2, and23, ep);
	addEdge(in1, and13, ep);
};

inline void IGNetwork::buildOr2Subgraph(IGNode *in1, IGNode *in1_, IGNode *in2, IGNode *in2_, IGNode *out, IGNode *out_, Array<IGNode *> &associate) {
	buildAnd2Subgraph(in1_, in1, in2_, in2, out_, out, associate);
};

#ifndef node_type
#define node_type(node) ((node)->p.type)
#endif
#define node_ignetwork(node) ((IGNetwork *) node->belongTo)
#define node_mark(node) ((node)->p.mark)
#define node_flag(node, i) ((node)->p.flag[(i)])
#define node_link(node) ((node)->p.link)
#define node_forced(node) ((node)->p.forced)
#define node_bnode(node) ((node)->p.bnode)
#define node_imply_type(node) ((node)->p.implyType)

#define foreach_ignode(network, node) \
	map<string, IGNode *>::iterator IGL_FORALL_VAR(__LINE__); \
	for (IGL_FORALL_VAR(__LINE__)  = (network)->name2Node.begin(), (node) = IGL_FORALL_VAR(__LINE__)->second; \
			IGL_FORALL_VAR(__LINE__) != (network)->name2Node.end(); \
			IGL_FORALL_VAR(__LINE__)++, (node) = IGL_FORALL_VAR(__LINE__)->second)

#endif
