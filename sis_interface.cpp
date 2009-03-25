#include "graph.h"
#include "bnetwork.h"

#undef array_fetch
#undef foreach_fanin
#undef foreach_fanout
#undef foreach_node
#undef node_num_fanin

#include "sis.h"
#include "sis_interface.h"

// redefine node name
#undef node_name
#define node_name(node) (node)->name

extern "C" {
	extern void init_sis(int flag);
	extern void end_sis();
}


BNetwork *sisnet2BNetwork(network_t *net) {
	int i, j;
	// topological order
	array_t *dfs = network_dfs(net);
	BNetwork *bnet = new BNetwork();

	for (i = 0; i < array_n(dfs); i++) {
		// read nodes one by one
		node_t *node = array_fetch(node_t *, dfs, i);
		node_t *fanin;
		BNodeType nodeType;

		BNode *rootNode;
		int numFanin = node_num_fanin(node);
		string oNodeName(node_name(node));
		string nodeName = oNodeName;
		string faninName;
		char numStr[16];

		switch(node_function(node)) {
			case NODE_PI:
			//printf("PI ");
			nodeType = BNODE_PI;
			break;
		
			case NODE_PO:
			//printf("PO ");
			nodeType = BNODE_PO;
			break;
			
			case NODE_AND:
			//printf("AND ");
			nodeType = BNODE_AND;
			break;

			case NODE_OR:
			//printf("OR ");
			nodeType = BNODE_OR;
			break;

			case NODE_INV:
			//printf("INV ");
			nodeType = BNODE_INV;
			break;

			case NODE_BUF:
			//printf("BUF ");
			nodeType = BNODE_BUF;
			break;

			default:
			//printf("COMPLEX ");
			nodeType = BNODE_COMPLEX;
			break;
		}

		//printf("%s\n", node_name(node));
		rootNode = bnet->newNode(nodeName, oNodeName, nodeType);

		//printf("fanins: ");
		
		// insert all fanins
		foreach_fanin(node, j, fanin){
			input_phase_t phase = POS_UNATE;
			int polarity;
			
			if (nodeType != BNODE_PO) {
				phase = node_input_phase(node, fanin);
			}

			polarity = (phase == POS_UNATE ? 1 : 0);

			if (nodeType == BNODE_INV){
				polarity = (polarity ? 0 : 1);
			}

			//printf("%s %d ", node_name(fanin), phase);

			WireProperty wp;
			wp.polarity = polarity;

			if (j == 0){
				bnet->addEdge(node_name(fanin), node_name(node), wp);
				//cout << "add edge " << node_name(fanin) << "->" << node_name(node) << endl;
			}
			else if (j >= numFanin - 1) {
				faninName = node_name(fanin);
				bnet->addEdge(faninName, nodeName, wp);
				//cout << "add edge " << faninName << "->" << nodeName << endl;
			}
			else {
				WireProperty iwp;
				iwp.polarity = 1;

				faninName = nodeName;

				snprintf(numStr, 16, "_%d", j);
				nodeName = oNodeName + numStr;

				bnet->newNode(nodeName, rootNode, nodeType);

				bnet->addEdge(nodeName, faninName, iwp);
				//cout << "add edge " << faninName << "->" << nodeName << endl;

				faninName = node_name(fanin);
				bnet->addEdge(faninName, nodeName, wp);
				//cout << "add edge " << faninName << "->" << nodeName << endl;
			}
		}

		//printf("\n");
	}

	array_free(dfs);

	return bnet;
}

BNetwork *read_blif_bnet(char *filename) {
	FILE *fp = NULL;
	network_t *net = NULL;
	BNetwork *bnet = NULL;

	fp = fopen(filename, "r");

	if (fp == NULL){
		// failed to open the file
		trace("failed to open file %s\n", filename);
		return NULL;
	}

	init_sis(0);

	if (read_blif(fp, &net) != 1) {
		// failed to read the file
		trace("fail to read file %s, %s\n", filename, error_string());
		
		fclose(fp);
		end_sis();
		return NULL;
	}
	
	fclose(fp);
	
	bnet = sisnet2BNetwork(net);

	network_free(net);

	end_sis();

	return bnet;
}
