/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/



#ifndef ALPM_DEPS_H
#define ALPM_DEPS_H

#include "db.h"
#include "sync.h"
#include "package.h"
#include "alpm.h"

alpm_depend_t *_alpm_dep_dup(const alpm_depend_t *dep);
alpm_list_t *_alpm_sortbydeps(alpm_handle_t *handle,
		alpm_list_t *targets, alpm_list_t *ignore, int reverse);
int _alpm_recursedeps(alpm_db_t *db, alpm_list_t **targs, int include_explicit);
int _alpm_resolvedeps(alpm_handle_t *handle, alpm_list_t *localpkgs, alpm_pkg_t *pkg,
		alpm_list_t *preferred, alpm_list_t **packages, alpm_list_t *remove,
		alpm_list_t **data);
int _alpm_depcmp_literal(alpm_pkg_t *pkg, alpm_depend_t *dep);
int _alpm_depcmp_provides(alpm_depend_t *dep, alpm_list_t *provisions);
int _alpm_depcmp(alpm_pkg_t *pkg, alpm_depend_t *dep);

#endif /* ALPM_DEPS_H */
