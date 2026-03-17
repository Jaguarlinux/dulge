/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/



#ifndef ALPM_CONFLICT_H
#define ALPM_CONFLICT_H

#include "alpm.h"
#include "db.h"
#include "package.h"

alpm_conflict_t *_alpm_conflict_dup(const alpm_conflict_t *conflict);
alpm_list_t *_alpm_innerconflicts(alpm_handle_t *handle, alpm_list_t *packages);
alpm_list_t *_alpm_outerconflicts(alpm_db_t *db, alpm_list_t *packages);
alpm_list_t *_alpm_db_find_fileconflicts(alpm_handle_t *handle,
		alpm_list_t *upgrade, alpm_list_t *remove);

#endif /* ALPM_CONFLICT_H */
