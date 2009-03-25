#ifndef __SIS_INTERFACE_H
#define __SIS_INTERFACE_H

#include "bnetwork.h"

BNetwork *read_blif_bnet(char *filename);
void *write_blif_bnet(BNetwork *network, char *filename);

#endif
