/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/




#include <stdio.h>

#include <alpm.h>
#include <alpm_list.h>

/* dulge */
#include "dulge.h"
#include "conf.h"

int dulge_deptest(alpm_list_t *targets)
{
	alpm_list_t *i;
	alpm_list_t *deps = NULL;
	alpm_db_t *localdb = alpm_get_localdb(config->handle);
	alpm_list_t *pkgcache = alpm_db_get_pkgcache(localdb);

	for(i = targets; i; i = alpm_list_next(i)) {
		char *target = i->data;

		if(!alpm_db_get_pkg(localdb, target) &&
				!alpm_find_satisfier(pkgcache, target)) {
			deps = alpm_list_add(deps, target);
		}
	}

	if(deps == NULL) {
		return 0;
	}

	for(i = deps; i; i = alpm_list_next(i)) {
		const char *dep = i->data;

		printf("%s\n", dep);
	}
	alpm_list_free(deps);
	return 127;
}
