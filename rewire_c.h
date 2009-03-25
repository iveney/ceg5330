#ifndef __REWIRE_C_H
#define __REWIRE_C_H

// constants =========================================
#define REDUNDANT 0
#define NONREDUNDANT 1

typedef struct {
	node_t *src;
	node_t *dst;
	// 1 -- not complemented, 0 -- complemented
	int polarity;
	node_function_t dst_function;
} rewire_aw_t;

#ifndef __cplusplus
int rewire_find_aw(node_t *twsrc, node_t *twdst, int rlevel, array_t *aw);
void rewire_network_change(network_t *network);
void rewire_end();
#endif

#endif
