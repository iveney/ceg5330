#ifndef __NODE_H
#define __NODE_H

#include "igl.h"

#include <string>
#include "array.h"

#define DEBUG_SHOW_TRACE
//#undef DEBUG_SHOW_TRACE
#include "debug.h"

using namespace std;

template <class NP, class EP> // Node Property, Edge Property
class Node {
	public:
	class NodeEdge {
		public:
		Node<NP, EP> *node;
		EP ep;

		public:
		
		int operator== (NodeEdge &to){
			return (node == to.node);
		}

		int operator> (NodeEdge &to){
			return (node > to.node);
		}

		int operator< (NodeEdge &to){
			return (node < to.node);
		}

		int operator>= (NodeEdge &to){
			return (node >= to.node);
		}

		int operator<= (NodeEdge &to){
			return (node <= to.node);
		}
	};
	
	Node(string &inName, void *graph); // allocate a new node
	~Node();
	
	Array<Node<NP, EP> *> in;
	Array<NodeEdge> out;
	//Array<EP> ep; // edge property of in edges
	NP p; // node property
	string *name;
	void *belongTo;

	void addFanin(Node<NP, EP> *node);
	void addFanout(Node<NP, EP> *node, EP &ep);

	int numIn() { return in.size;};
	int numOut() { return in.size;};

	void removeFanin(Node<NP, EP> *node);
	void removeFanout(Node<NP, EP> *node);

	void rename(string &newName);
	void rename(char *newName);

	EP &getFaninProperty(Node<NP, EP> *fanin);
	EP &getFanoutProperty(Node<NP, EP> *fanout);
	EP &getFanoutProperty(int idx);
};

// constructors-----------------------------------------
template <class NP, class EP>
inline Node<NP, EP>::Node(string &inName, void *graph) {
	name = new string(inName);
	belongTo = graph;
};

template <class NP, class EP>
inline Node<NP, EP>::~Node(){
	//cout << "deleteing " << name << endl;
	delete name;
};

// member functions-------------------------------------
template <class NP, class EP>
inline void Node<NP, EP>::addFanin(Node<NP, EP> *node) {
	int idx = in.find(node);

	if (idx >= 0) {
		trace("error: node %s already has a fanin %s\n", name->c_str(), node->name->c_str());
		return;
	}

	in.insert(node);
	in.sort();
};

template <class NP, class EP>
inline void Node<NP, EP>::addFanout(Node<NP, EP> *node, EP &ep) {
	NodeEdge holder;
	holder.node = node;
	
	int idx = out.find(holder);

	if (idx >= 0) {
		trace("error: node %s already has a fanout %s\n", name->c_str(), node->name->c_str());
		return;
	}

	holder.ep = ep;
	
	out.insert(holder);
	out.sort();
};

template <class NP, class EP>
inline void Node<NP, EP>::removeFanin(Node<NP, EP> *node) {
	int idx = in.find(node);
	
	if (idx < 0) {
		trace("error: no such fanin %s for node %s\n", node->name->c_str(), name->c_str());
		return;
	}
	
	in.remove(idx);
};

template <class NP, class EP>
inline void Node<NP, EP>::removeFanout(Node<NP, EP> *node) {
	NodeEdge holder;
	holder.node = node;

	int idx = out.find(holder);
	if (idx < 0) {
		trace("error: no such fanout %s for node %s\n", node->name->c_str(), name->c_str());
		return;
	}
	
	out.remove(idx);
};

template <class NP, class EP>
inline EP& Node<NP, EP>::getFaninProperty(Node<NP, EP> *fanin) {
	return fanin->getFanoutProperty(this);
};

template <class NP, class EP>
inline EP& Node<NP, EP>::getFanoutProperty(Node<NP, EP> *fanout) {
	NodeEdge holder;
	holder.node = fanout;

	int idx = out.find(holder);
	if (idx < 0) {
		trace("error: no such fanout %s for node %s\n", fanout->name->c_str(), name->c_str());
		ASSERT(0, "unable to handle NULL reference");
	}

	return getFanoutProperty(idx);
};

template <class NP, class EP>
inline EP &Node<NP, EP>::getFanoutProperty(int idx) {
	return array_fetch(out, idx).ep;
};


template <class NP, class EP>
inline void Node<NP, EP>::rename(string &newName) {};
template <class NP, class EP>
inline void Node<NP, EP>::rename(char *newName) {};


//====================================================
// macro
//====================================================
#define foreach_fanin(node, i, p) \
	for((i) = 0; (i) < (node)->in.size && ((p) = array_fetch((node)->in, (i))) != NULL; (i)++)

#define foreach_fanout(node, i, p) \
	for((i) = 0; (i) < (node)->out.size && ((p) = array_fetch((node)->out, (i)).node) != NULL; (i)++)

#define node_fanin_ep(node, idx) ((node)->getFaninProperty(array_fetch((node)->in, (idx))))
#define node_fanout_ep(node, idx) (array_fetch((node)->out, (idx)).ep)
#define node_fanin(node, idx) (array_fetch((node)->in, (idx)))
#define node_fanout(node_, idx) (array_fetch((node_)->out, (idx)).node)
#define node_name(node) (*(node)->name)
#define node_num_fanin(node) ((node)->in.size)
#define node_num_fanout(node) ((node)->out.size)
#define node_get_fanin_index(node, fanin) ((node)->in.find((fanin)))
#define node_get_fanout_index(node, fanout) ((node)->out.find((fanout)))

#endif
