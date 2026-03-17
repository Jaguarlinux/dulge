/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/



#ifndef ALPM_GRAPH_H
#define ALPM_GRAPH_H

#include <sys/types.h> /* off_t */

#include "alpm_list.h"

enum _alpm_graph_vertex_state {
	ALPM_GRAPH_STATE_UNPROCESSED,
	ALPM_GRAPH_STATE_PROCESSING,
	ALPM_GRAPH_STATE_PROCESSED
};

typedef struct _alpm_graph_t {
	void *data;
	struct _alpm_graph_t *parent; /* where did we come from? */
	alpm_list_t *children;
	alpm_list_t *iterator; /* used for DFS without recursion */
	off_t weight; /* weight of the node */
	enum _alpm_graph_vertex_state state;
} alpm_graph_t;

alpm_graph_t *_alpm_graph_new(void);
void _alpm_graph_free(void *data);

#endif /* ALPM_GRAPH_H */
