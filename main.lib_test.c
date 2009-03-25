#include <stdio.h>
#include "debug.h"
#include "sis_header/sis.h"
#include "rewire_c.h"

// function: util_readblif
// use: read a blif file
// in: char *filename -- the name of the blif file to be read
// out: network_t **net -- where the read network will be stored
// return: SUCCESS or NOT_SUCCESS
int util_readblif(char *filename, network_t **net){
	FILE *fp = NULL;

	fp = fopen(filename, "r");

	if (fp == NULL){
		// failed to open the file
		trace("failed to open file %s\n", filename);
		return -1;
	}

	if (read_blif(fp, net) != 1) {
		// failed to read the file
		trace("fail to read file %s, %s\n", filename, error_string());
		
		fclose(fp);
		return -1;
	}
	
	fclose(fp);
	
	return 0;
}
// ---------------------------------------------------

int main(int argc, char **argv) {
	network_t *net;
	
	if (argc != 3){
		printf("Usage: %s [blif] [recursive_rewire_level]\n", argv[0]);
		exit(0);
	}

	init_sis(0);

	util_readblif(argv[1], &net);

	rewire_network_change(net);

	{
		array_t *fanin_list;
		array_t *dfs = network_dfs(net);
		int i, j, k;
		node_t *node, *fanin;
		int rlevel = atoi(argv[2]);

		for (i = 0; i < array_n(dfs); i++){
			node = array_fetch(node_t *, dfs, i);

			// node is not PI / PO
			if(node_type(node) == PRIMARY_OUTPUT) continue;
			if(node_type(node) == PRIMARY_INPUT) continue;

			// node is not BUF / INV
			if(node_num_fanin(node) <= 1) continue;

			// must use an array to store the fanin since fanin change during testing
			fanin_list = array_alloc(node_t *, 0);

			foreach_fanin(node, j, fanin){
				array_insert_last(node_t *, fanin_list, fanin);
			}

			// check each target wire
			for (j = 0; j < array_n(fanin_list); j++){
				int ret;
				array_t *aw = array_alloc(rewire_aw_t, 0);
				
				fanin = array_fetch(node_t *, fanin_list, j);

				ret = rewire_find_aw(fanin, node, rlevel, aw);

				for (k = 0; k < array_n(aw); k++) {
					rewire_aw_t aw_found = array_fetch(rewire_aw_t, aw, k);
					printf("tw: %s -> %s aw: %s -%s-> %s type: %s polarity: %d\n", fanin->name, node->name, aw_found.src->name, aw_found.polarity == 1 ? "---" : "INV", aw_found.dst->name, aw_found.dst_function == NODE_AND ? "AND" : "OR", aw_found.polarity);
				}
			}

			array_free(fanin_list);
		}
	}

	array_free(dfs);

	rewire_end();
	
	network_free(net);
	
	end_sis();

	return 0;
}
