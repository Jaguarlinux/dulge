/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/



#ifndef ALPM_REMOVE_H
#define ALPM_REMOVE_H

#include "db.h"
#include "alpm_list.h"
#include "trans.h"

int _alpm_remove_prepare(alpm_handle_t *handle, alpm_list_t **data);
int _alpm_remove_packages(alpm_handle_t *handle, int run_ldconfig);

int _alpm_remove_single_package(alpm_handle_t *handle,
		alpm_pkg_t *oldpkg, alpm_pkg_t *newpkg,
		size_t targ_count, size_t pkg_count);

#endif /* ALPM_REMOVE_H */
