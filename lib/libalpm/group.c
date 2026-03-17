/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/




#include <stdlib.h>
#include <string.h>

/* libalpm */
#include "group.h"
#include "alpm_list.h"
#include "util.h"
#include "log.h"
#include "alpm.h"

alpm_group_t *_alpm_group_new(const char *name)
{
	alpm_group_t *grp;

	CALLOC(grp, 1, sizeof(alpm_group_t), return NULL);
	STRDUP(grp->name, name, free(grp); return NULL);

	return grp;
}

void _alpm_group_free(alpm_group_t *grp)
{
	if(grp == NULL) {
		return;
	}

	FREE(grp->name);
	/* do NOT free the contents of the list, just the nodes */
	alpm_list_free(grp->packages);
	FREE(grp);
}
