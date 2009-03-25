#ifndef __GRAPH_H
#define __GRAPH_H

#include "igl.h"

#include <map>
#include "array.h"
#include "node.h"
#include "edge.h"

#define DEBUG_SHOW_TRACE
//#undef DEBUG_SHOW_TRACE
#include "debug.h"

using namespace std;

template <class GP, class NP, class EP> // GraphProperty, NodeProperty, EdgeProperty
class Graph {
	static string *getNewName();

	int nodeIsMadeupName(string &name, int *value);

	public:

	map<string, Node<NP, EP> *> name2Node;
	static int nameCount;

	GP p;

	Graph();
	Graph(Array<string>& names);
	Graph(Graph<GP, NP, EP> &graph);
	~Graph();
		
	Node<NP, EP> *newNode(string &name); // create a new node with "name", allocates new string
	Node<NP, EP> *newNode(char *name);
	Node<NP, EP> *newNode();

	Node<NP, EP> *findNode(string &name); // return the node with "name"
	Node<NP, EP> *findNode(char *name);

	void removeNode(string &name); // remove the node with "name" from the graph
	void removeNode(char *name);
	void removeNode(Node<NP, EP> *node);

	void renameNode(char *oldName, char *newName); // rename the node
	void renameNode(string &oldName, string &newName);
	void renameNode(Node<NP, EP> *node, string &newName);

	void addEdge(char *faninName, char *fanoutName, EP &ep); // add and remove edges
	void addEdge(string &faninName, string &fanoutName, EP &ep);
	void addEdge(Node<NP, EP> *fanin, Node<NP, EP> *fanout, EP &ep);
	void addEdge(Edge<NP, EP> &edge);
	
	void removeEdge(char *faninName, char *fanoutName);
	void removeEdge(string &faninName, string &fanoutName);
	void removeEdge(Node<NP, EP> *fanin, Node<NP, EP> *fanout);
	//void removeEdge(Node<NP, EP> *fanin, int fanoutIdx);
	//void removeEdge(int faninIdx, Node<NP, EP> *fanout);
	void removeEdge(Edge<NP, EP> &edge);

	EP &getEdgeProperty(char *faninName, char *fanoutName); // get the edge property of the edge
	EP &getEdgeProperty(string &faninName, string &fanoutName);
	EP &getEdgeProperty(Node<NP, EP> *fanin, Node<NP, EP> *fanout);
	EP &getEdgeProperty(Node<NP, EP> *fanin, int fanoutIdx);
	EP &getEdgeProperty(Edge<NP, EP> &edge);

	// get node property
	NP &getNodeProperty(Node<NP, EP> *node) {return node->p;};
	NP &getNodeProperty(string &name) {return getNodeProperty(findNode(name));};
	NP &getNodeProperty(char *name) {return getNodeProperty(findNode(name));};
};

// initialization of static member----------------------
template <class GP, class NP, class EP>
int Graph<GP, NP, EP>::nameCount = 0;


// constructors-----------------------------------------
template <class GP, class NP, class EP>
inline Graph<GP, NP, EP>::Graph() {
};

template <class GP, class NP, class EP>
inline Graph<GP, NP, EP>::Graph(Array<string>& names) {
	// TODO: implement
};

template <class GP, class NP, class EP>
inline Graph<GP, NP, EP>::Graph(Graph &graph) {
	// TODO: implement
};

template <class GP, class NP, class EP>
inline Graph<GP, NP, EP>::~Graph(){
	typename map<string, Node<NP, EP> *>::iterator it = name2Node.begin(); 

	while(it != name2Node.end()) {
		//cout << it->first << it->second << endl;
		delete it->second; // delete the node
		it++;
	}
};

// member functions-------------------------------------
template <class GP, class NP, class EP>
inline int Graph<GP, NP, EP>::nodeIsMadeupName(string &name, int *value) {
	if (name[0] == '[' && name[name.length() - 1] == ']') {
		if (sscanf(name.c_str(), "[%d]", value) == 1) {
			return 1;
		}
	}
	return 0; /* not a made-up name */
};

template <class GP, class NP, class EP>
inline Node<NP, EP> *Graph<GP, NP, EP>::newNode(string &name) {
	// see if the name given is used
	if (name2Node.find(name) != name2Node.end()) {
		// duplicate name
		trace("error: duplicated node name %s\n", name.c_str());
		return NULL;
	}
	
	Node<NP, EP> *node = new Node<NP, EP>(name, this);

	name2Node[name] = node;

	return node;
};	// create a new node with "name", allocates new string

template <class GP, class NP, class EP>
inline Node<NP, EP> *Graph<GP, NP, EP>::newNode(char *name) {
	string nameStr(name);
	return newNode(nameStr);
};

template <class GP, class NP, class EP>
inline Node<NP, EP> *Graph<GP, NP, EP>::newNode() {
	// TODO: implement
	return NULL;
};

//--------------

template <class GP, class NP, class EP>
inline Node<NP, EP> *Graph<GP, NP, EP>::findNode(string &name) {
	typename map<string, Node<NP, EP> *>::iterator it = name2Node.find(name);
	Node<NP, EP> *node = NULL;
	
	if (it != name2Node.end()) {
		node = it->second;
	}

	return node;
};	// return the node with "name"

template <class GP, class NP, class EP>
inline Node<NP, EP> *Graph<GP, NP, EP>::findNode(char *name) {
	string nameStr(name);
	return findNode(nameStr);
};

//--------------

template <class GP, class NP, class EP>
inline void Graph<GP, NP, EP>::removeNode(string &name) {
	Node<NP, EP> *node = findNode(name);
	if (node) {
		removeNode(node);
	}
	else {
		trace("error: node %s does not exist\n", name.c_str());
	}
};	// remove the node with "name" from the graph

template <class GP, class NP, class EP>
inline void Graph<GP, NP, EP>::removeNode(char *name) {
	string nameStr(name);
	return removeNode(nameStr);
};

template <class GP, class NP, class EP>
inline void Graph<GP, NP, EP>::removeNode(Node<NP, EP> *node) {
	if (node->belongTo == this) {
		// remove all edges
		Node<NP, EP> *fanout, *fanin;
		int i;

		foreach_fanout(node, i, fanout) {
			fanout->removeFanin(node);
		}

		foreach_fanin(node, i, fanin) {
			fanin->removeFanout(node);
		}
		
		// remove the node
		name2Node.erase(*(node->name));
		delete node;
	}
	else {
		trace("error: node %s does not belong to graph %x\n", node->name->c_str(), this);
	}
};

//--------------

template <class GP, class NP, class EP>
void Graph<GP, NP, EP>::renameNode(char *oldName, char *newName) {
	// TODO: implement
}; // rename the node

template <class GP, class NP, class EP>
void Graph<GP, NP, EP>::renameNode(string &oldName, string &newName) {
	// TODO: implement
};	// rename the node

template <class GP, class NP, class EP>
void Graph<GP, NP, EP>::renameNode(Node<NP, EP> *node, string &newName) {
	// TODO: implement
};

//--------------

template <class GP, class NP, class EP>
inline void Graph<GP, NP, EP>::addEdge(char *faninName, char *fanoutName, EP &ep) {
	string faninNameStr(faninName);
	string fanoutNameStr(fanoutName);
	addEdge(faninNameStr, fanoutNameStr, ep);
};

template <class GP, class NP, class EP>
inline void Graph<GP, NP, EP>::addEdge(string &faninName, string &fanoutName, EP &ep) {
	Node<NP, EP> *fanin = findNode(faninName);
	Node<NP, EP> *fanout = findNode(fanoutName);

	if (fanin == NULL) {
		trace("error: fanin %s not found\n", faninName.c_str());
		return;
	}

	if (fanout == NULL) {
		trace("error: fanout %s not found\n", fanoutName.c_str());
		return;
	}

	addEdge(fanin, fanout, ep);
};	// add and remove edges

template <class GP, class NP, class EP>
inline void Graph<GP, NP, EP>::addEdge(Node<NP, EP> *fanin, Node<NP, EP> *fanout, EP &ep) {
	if (fanin->belongTo != this) {
		trace("error: fanin %s does not belong to graph %x\n", fanin->name->c_str(), this);
		return;
	}

	if (fanout->belongTo != this) {
		trace("error: fanout %s does not belong to graph %x\n", fanout->name->c_str(), this);
		return;
	}

	fanin->addFanout(fanout, ep);
	fanout->addFanin(fanin);
};

template <class GP, class NP, class EP>
inline void Graph<GP, NP, EP>::addEdge(Edge<NP, EP> &edge) {
	addEdge(edge.fanin, edge.fanout, edge.ep);
};

//--------------

template <class GP, class NP, class EP>
inline void Graph<GP, NP, EP>::removeEdge(char *faninName, char *fanoutName) {
	string faninNameStr(faninName);
	string fanoutNameStr(fanoutName);
	removeEdge(faninNameStr, fanoutNameStr);
};

template <class GP, class NP, class EP>
inline void Graph<GP, NP, EP>::removeEdge(string &faninName, string &fanoutName) {
	Node<NP, EP> *fanin = findNode(faninName);
	Node<NP, EP> *fanout = findNode(fanoutName);

	if (fanin == NULL) {
		trace("error: fanin %s not found\n", faninName.c_str());
		return;
	}

	if (fanout == NULL) {
		trace("error: fanout %s not found\n", fanoutName.c_str());
		return;
	}

	removeEdge(fanin, fanout);
};

template <class GP, class NP, class EP>
inline void Graph<GP, NP, EP>::removeEdge(Node<NP, EP> *fanin, Node<NP, EP> *fanout) {
	if (fanin->belongTo != this) {
		trace("error: fanin %s does not belong to graph %x\n", fanin->name->c_str(), this);
		return;
	}

	if (fanout->belongTo != this) {
		trace("error: fanout %s does not belong to graph %x\n", fanout->name->c_str(), this);
		return;
	}

	fanin->removeFanout(fanout);
	fanout->removeFanin(fanin);
};

template <class GP, class NP, class EP>
inline void Graph<GP, NP, EP>::removeEdge(Edge<NP, EP> &edge) {
	removeEdge(edge.fanin, edge.fanout);
}

//--------------

template <class GP, class NP, class EP>
inline EP& Graph<GP, NP, EP>::getEdgeProperty(char *faninName, char *fanoutName) {
	string faninNameStr(faninName);
	string fanoutNameStr(fanoutName);
	return getEdgeProperty(faninNameStr, fanoutNameStr);
}

template <class GP, class NP, class EP>
inline EP& Graph<GP, NP, EP>::getEdgeProperty(string &faninName, string &fanoutName) {
	Node<NP, EP> *fanin = findNode(faninName);
	Node<NP, EP> *fanout = findNode(fanoutName);

	if (fanin == NULL) {
		trace("error: fanin %s not found\n", faninName.c_str());
		ASSERT(0, "unable to handle NULL reference");
	}

	if (fanout == NULL) {
		trace("error: fanout %s not found\n", fanoutName.c_str());
		ASSERT(0, "unable to handle NULL reference");
	}

	return getEdgeProperty(fanin, fanout);
};

template <class GP, class NP, class EP>
inline EP& Graph<GP, NP, EP>::getEdgeProperty(Node<NP, EP> *fanin, Node<NP, EP> *fanout) {
	if (fanin->belongTo != this) {
		trace("error: fanin %s does not belong to graph %x\n", fanin->name->c_str(), this);
		ASSERT(0, "unable to handle NULL reference");
	}

	if (fanout->belongTo != this) {
		trace("error: fanout %s does not belong to graph %x\n", fanout->name->c_str(), this);
		ASSERT(0, "unable to handle NULL reference");
	}

	return fanin->getFanoutProperty(fanout);
};

template <class GP, class NP, class EP>
inline EP& Graph<GP, NP, EP>::getEdgeProperty(Node<NP, EP> *fanin, int fanoutIdx) {
	if (fanin->belongTo != this) {
		trace("error: fanin %s does not belong to graph %x\n", fanin->name->c_str(), this);
		ASSERT(0, "unable to handle NULL reference");
	}

	return fanin->getFanoutProperty(fanoutIdx);
};

template <class GP, class NP, class EP>
inline EP& Graph<GP, NP, EP>::getEdgeProperty(Edge<NP, EP> &edge) {
	return getEdgeProperty(edge.fanin, edge.fanout);
}

//--------------

#endif
