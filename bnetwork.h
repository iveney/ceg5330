#ifndef __BNETWORK_H
#define __BNETWORK_H

#include <string>
#include "array.h"
#include "graph.h"

#define DEBUG_SHOW_TRACE
//#undef DEBUG_SHOW_TRACE
#include "debug.h"

using namespace std;

class BNetworkProperty;
class BNodeProperty;
class WireProperty;

// IG Associations--------------------------------------
class IGNodeProperty;
class IGEdgeProperty;
class IGAssociateProperty;

typedef Node<IGNodeProperty, IGEdgeProperty> IGNode;

class IGAssociateProperty {
	public:
		
	IGNode *g0;
	IGNode *g1;
	IGNode *f0;
	IGNode *f1;
	Array<IGNode *> associatedNodes;

	IGAssociateProperty() {
		g0 = NULL; g1 = NULL; f0 = NULL; f1 = NULL;
	};
	~IGAssociateProperty() { }; // do nothing
};
//------------------------------------------------------

enum BNodeType_ {BNODE_PI, BNODE_PO, BNODE_AND, BNODE_OR, BNODE_INV, BNODE_BUF, BNODE_COMPLEX};
typedef enum BNodeType_ BNodeType;

typedef Node<BNodeProperty, WireProperty> BNode;

class BNetworkProperty {
};

#define BNODE_FANOUT_CONE_MARK 0x1
#define BNODE_ABSDOM_VISITED 0x2
#define BNODE_IS_OMA 0x4
#define BNODE_IS_NOMA 0x8
#define BNODE_IS_ABSDOM 0x10

class BNodeProperty {
	public:
	BNodeType type;
	IGAssociateProperty ignodes;
	int flag;
	int value;
	Array<BNode *> absDom;

	// multiple input gates
	string *oName;
	BNode *linkedNode;

	BNodeProperty() {
		flag = 0;

		oName = NULL;
		linkedNode = NULL;
	};
	~BNodeProperty() {
		if (oName != NULL && linkedNode == NULL) {
			delete oName;
		}
	};
};

class WireProperty {
	public:
	int polarity;
};

class BNetwork: public Graph<BNetworkProperty, BNodeProperty, WireProperty> {
	void assignDominatorRecur(BNode *node);
	
	public:
	map<string, BNode *> PI2Node;
	map<string, BNode *> PO2Node;

	BNetwork();
	~BNetwork();

	BNode *newNode(string &name, BNodeType type); // create a new node with "name", allocates new string
	BNode *newNode(char *name, BNodeType type);
	BNode *newNode(BNodeType type);

	BNode *newNode(string &name, string &oName, BNodeType type);
	BNode *newNode(char *name, char *oName, BNodeType type);

	BNode *newNode(string &name, BNode *linkedNode, BNodeType type);
	BNode *newNode(char *name, BNode *linkedNode, BNodeType type);

	void removeNode(string &name); // remove the node with "name" from the graph
	void removeNode(char *name);
	void removeNode(BNode *node);

	void assignDominator();
	// TODO:
	//addNodeAtOut();
	//addNodeAtIn();
	//addWire();
	//removeWire();
};

// constructors-----------------------------------------
inline BNetwork::BNetwork() {
};

inline BNetwork::~BNetwork() {
};

// member functions-------------------------------------

inline BNode *BNetwork::newNode(string &name, BNodeType type) {
	BNode *returnNode = Graph<BNetworkProperty, BNodeProperty, WireProperty>::newNode(name);
	
	if (returnNode != NULL) {
		if (type == BNODE_PI) {
			PI2Node[name] = returnNode;
		}
		else if (type == BNODE_PO) {
			PO2Node[name] = returnNode;
		}

		returnNode->p.type = type;
	}

	return returnNode;
};	// create a new node with "name", allocates new string

inline BNode *BNetwork::newNode(string &name, string &oName, BNodeType type) {
	BNode *returnNode = Graph<BNetworkProperty, BNodeProperty, WireProperty>::newNode(name);
	
	if (returnNode != NULL) {
		if (type == BNODE_PI) {
			PI2Node[name] = returnNode;
		}
		else if (type == BNODE_PO) {
			PO2Node[name] = returnNode;
		}

		returnNode->p.type = type;

		returnNode->p.oName = new string(oName);
	}

	return returnNode;
};	// create a new node with "name" and "original name", allocates new string for oName

inline BNode *BNetwork::newNode(string &name, BNode *linkedNode, BNodeType type) {
	BNode *returnNode = Graph<BNetworkProperty, BNodeProperty, WireProperty>::newNode(name);
	
	if (returnNode != NULL) {
		if (type == BNODE_PI) {
			PI2Node[name] = returnNode;
		}
		else if (type == BNODE_PO) {
			PO2Node[name] = returnNode;
		}

		returnNode->p.type = type;

		returnNode->p.linkedNode = linkedNode;
		returnNode->p.oName = linkedNode->p.oName;
	}

	return returnNode;
};	// create a new node with "name"

inline BNode *BNetwork::newNode(char *name, BNodeType type) {
	string nameStr(name);
	return newNode(nameStr, type);
};

inline BNode *BNetwork::newNode(char *name, char *oName, BNodeType type) {
	string nameStr(name);
	string oNameStr(oName);
	return newNode(nameStr, oNameStr, type);
};

inline BNode *BNetwork::newNode(char *name, BNode *linkedNode, BNodeType type) {
	string nameStr(name);
	return newNode(nameStr, linkedNode, type);
};

inline BNode *BNetwork::newNode(BNodeType type) {
	// TODO: implement
};

inline void BNetwork::removeNode(string &name) {
	BNode *node = findNode(name);
	if (node) {
		removeNode(node);
	}
	else {
		trace("error: node %s does not exist\n", name.c_str());
	}
};

inline void BNetwork::removeNode(char *name) {
	string nameStr(name);
	return removeNode(nameStr);
};

inline void BNetwork::removeNode(BNode *node) {
	if (node->belongTo == this) {
		if (node->p.type == BNODE_PI) {
			PI2Node.erase(*(node->name));
		}
		else if (node->p.type == BNODE_PO) {
			PO2Node.erase(*(node->name));
		}
	}
	else {
		trace("error: node %s does not belong to network %x\n", node->name->c_str(), this);
	}

	Graph<BNetworkProperty, BNodeProperty, WireProperty>::removeNode(node);
};	// create a new node with "name", allocates new string

//====================================================
// macro
//====================================================
#define removeWire(...) removeEdge(...)

#define foreach_node(network, node) \
	map<string, BNode *>::iterator IGL_FORALL_VAR(__LINE__); \
	for (IGL_FORALL_VAR(__LINE__)  = (network)->name2Node.begin(), (node) = IGL_FORALL_VAR(__LINE__)->second; \
			IGL_FORALL_VAR(__LINE__) != (network)->name2Node.end(); \
			IGL_FORALL_VAR(__LINE__)++, (node) = (IGL_FORALL_VAR(__LINE__) == (network)->name2Node.end())? NULL: IGL_FORALL_VAR(__LINE__)->second)

#define foreach_PI(network, node) \
	map<string, BNode *>::iterator IGL_FORALL_VAR(__LINE__); \
	for (IGL_FORALL_VAR(__LINE__)  = (network)->PI2Node.begin(), (node) = IGL_FORALL_VAR(__LINE__)->second; \
			IGL_FORALL_VAR(__LINE__) != (network)->PI2Node.end(); \
			IGL_FORALL_VAR(__LINE__)++, (node) = (IGL_FORALL_VAR(__LINE__) == (network)->PI2Node.end())? NULL: IGL_FORALL_VAR(__LINE__)->second)

#define foreach_PO(network, node) \
	map<string, BNode *>::iterator IGL_FORALL_VAR(__LINE__); \
	for (IGL_FORALL_VAR(__LINE__)  = (network)->PO2Node.begin(), (node) = IGL_FORALL_VAR(__LINE__)->second; \
			IGL_FORALL_VAR(__LINE__) != (network)->PO2Node.end(); \
			IGL_FORALL_VAR(__LINE__)++, (node) = (IGL_FORALL_VAR(__LINE__) == (network)->PO2Node.end())? NULL: IGL_FORALL_VAR(__LINE__)->second)

#ifndef node_type
#define node_type(node) ((node)->p.type)
#endif
#define bnode_bnetwork(node) ((BNetwork *) node->belongTo)
//#define edge_polarity(fanin, fanout) bnode_network(fanin)->getEdgeProperty((fanin), (fanout)).polarity
#define bnode_fanin_polarity(node, idx) node_fanin_ep(node, idx).polarity
#define bnode_flag(node) ((node)->p.flag)
#define bnode_abs_dom(node) ((node)->p.absDom)
#define bnode_ignodes(node) ((node)->p.ignodes)
#define bnode_oname(node) (*(node)->p.oName)

inline void BNetwork::assignDominator() {
	BNode *node;

	foreach_node(this, node) {
		bnode_abs_dom(node).clear();
	}

	foreach_PO(this, node) {
		// the dominator of a PO is only itself
		bnode_abs_dom(node).insert(node);
		
		bnode_flag(node) |= BNODE_ABSDOM_VISITED;
	}

	// do dominator assignment starting from PI
	foreach_PI(this, node){
		assignDominatorRecur(node);
	}

	foreach_node(this, node) {
		bnode_flag(node) &= ~(BNODE_ABSDOM_VISITED);
	}
}

#endif
