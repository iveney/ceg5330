#include <iostream>
#include <string>
#include "array.h"
#include "sis_interface.h"
#include "rewire.h"
#include "stdlib.h"

const int MAXSTR=80;

//IGNetwork *global_ignet;

int main(int argc, char **argv){
	BNetwork *net;
	IGNetwork *ignet;
	int rlevel;
	string bench_name, blif_name, unreach_file, system_call;
	char blif_name_char[MAXSTR];
	int i;
	if (argc != 3){
		printf("Usage: %s [bench] [recursive_rewire_level]\n", argv[0]);
		exit(0);
	}

	rlevel = atoi(argv[2]);

	bench_name = argv[1];

	i = bench_name.rfind(".");
	blif_name = bench_name.substr(0, i)+".blif";
	
	unreach_file = bench_name.substr(0, i)+"f.report";

	system_call = "./abc -c \"read " + bench_name;
	system_call += "; strash; write_blif ";
	system_call += blif_name;
	system_call += "\"";

	cout<<system_call<<endl;

	cout<<"call abc"<<endl;
	system(system_call.c_str());

	system_call = "./unreachable " + bench_name.substr(0,i);

	cout<<system_call<<endl;

	cout<<"call unreachable"<<endl;
	system(system_call.c_str()); 

	strcpy(blif_name_char, blif_name.c_str());
	net = read_blif_bnet(blif_name_char);

	
	net->assignDominator();
	ignet = new IGNetwork(net);

	{
		int i, j, stucktype;
		BNode *node, *fanin;
		Array<BNode *> fanoutMarkedNodes, omaMarkedNodes, nomaMarkedNodes;
		Array<IGNode *> markedNodes;
		int alterwires = 0;
		foreach_node(net, node) {
			// node is not PI / PO
			if (node_type(node) == BNODE_PI) continue;
			if (node_type(node) == BNODE_PO) continue;

			// node is not BUF / INV
			if (node_num_fanin(node) <= 1) continue;

			foreach_fanin(node, i, fanin){
				Array<RewireAW> aw;
				RewireAW awFound;

				rewireFindAW(fanin, node, rlevel, aw);

				for (j = 0; j < aw.size; j++) {
					awFound = array_fetch(aw, j);
					cout << "tw: " << node_name(fanin) << " -> " << bnode_oname(node) << " ";
					cout << "aw: " << node_name(awFound.src) << (awFound.polarity ? " -----> " : " -INV-> ") << node_name(awFound.dst) << " type: " << (awFound.dstFunction == BNODE_AND ? "AND" : "OR") << " polarity: " << awFound.polarity << endl;
				}
				alterwires += aw.size;	
			}
		}
		printf(" %d alternative wires are found\n", alterwires);
	}

	delete net;
	delete ignet;

	rewireEndImply();
	
	return 0;
}
