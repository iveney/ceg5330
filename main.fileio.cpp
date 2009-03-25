#include <iostream>
#include "array.h"
#include "sis_interface.h"
#include "rewire.h"

int main(int argc, char **argv){
	BNetwork *net;
	IGNetwork *ignet;
	int rlevel;
	FILE *wireData, *outFile;
	char buf[1024];
	char *delimiter = " \n";
	char *token;
	
	if (argc != 5){
		printf("Usage: %s [blif] [target wires] [recursive learning level] [output file]\n", argv[0]);
		printf("Wires are in the format: [id] [src] [dst]\n");
		printf("Output wires are in the format: [id] [aw src] [aw dst] [dst function] [polarity]\n");
		exit(0);
	}

	rlevel = atoi(argv[3]);

	net = read_blif_bnet(argv[1]);
	net->assignDominator();
	ignet = new IGNetwork(net);

	wireData = fopen(argv[2], "r");
	outFile = fopen(argv[4], "w");

	while (fgets(buf, 1023, wireData)) {
		int wireID;
		BNode *twsrc, *twdst;
		Array<RewireAW> aw;
		RewireAW awFound;
		int i;
		
		token = strtok(buf, delimiter);
		wireID = atoi(token);
		token = strtok(NULL, delimiter);
		twsrc = net->findNode(token);
		token = strtok(NULL, delimiter);
		twdst = net->findNode(token);

		rewireFindAW(twsrc, twdst, rlevel, aw);

		for (i = 0; i < aw.size; i++) {
			awFound = array_fetch(aw, i);
			//cout << "tw: " << wireID << " " << node_name(twsrc) << " -> " << node_name(twdst) << " ";
			//cout << "aw: " << node_name(awFound.src) << (awFound.polarity ? " -----> " : " -INV-> ") << node_name(awFound.dst) << " type: " << (awFound.dstFunction == BNODE_AND ? "AND" : "OR") << " polarity: " << awFound.polarity << endl;
			fprintf(outFile, "%d %s %s %s %d\n", wireID, node_name(awFound.src).c_str(), node_name(awFound.dst).c_str(), (awFound.dstFunction == BNODE_AND ? "AND" : "OR"), awFound.polarity);
		}
		
	}

	fclose(wireData);
	fclose(outFile);

	delete net;
	delete ignet;

	rewireEndImply();

	return 0;
}
