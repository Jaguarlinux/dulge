/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/




#include "graph.h"
#include "util.h"
#include "log.h"

alpm_graph_t *_alpm_graph_new(void)
{
	alpm_graph_t *graph = NULL;

	CALLOC(graph, 1, sizeof(alpm_graph_t), return NULL);
	return graph;
}

void _alpm_graph_free(void *data)
{
	ASSERT(data != NULL, return);
	alpm_graph_t *graph = data;
	alpm_list_free(graph->children);
	free(graph);
}
