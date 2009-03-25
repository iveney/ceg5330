#ifndef __EDGE_H
#define __EDGE_H

#include "node.h"

template <class NP, class EP>
class Edge {
	public:
		
	Node<NP, EP> *fanin;
	Node<NP, EP> *fanout;

	EP ep;
};

#endif
