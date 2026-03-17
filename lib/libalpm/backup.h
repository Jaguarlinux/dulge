/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/



#ifndef ALPM_BACKUP_H
#define ALPM_BACKUP_H

#include "alpm_list.h"
#include "alpm.h"

int _alpm_split_backup(const char *string, alpm_backup_t **backup);
alpm_backup_t *_alpm_needbackup(const char *file, alpm_pkg_t *pkg);
void _alpm_backup_free(alpm_backup_t *backup);
alpm_backup_t *_alpm_backup_dup(const alpm_backup_t *backup);

#endif /* ALPM_BACKUP_H */
