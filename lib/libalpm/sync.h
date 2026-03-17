/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/



#ifndef ALPM_SYNC_H
#define ALPM_SYNC_H

#include "alpm.h"

int _alpm_sync_prepare(alpm_handle_t *handle, alpm_list_t **data);
int _alpm_sync_load(alpm_handle_t *handle, alpm_list_t **data);
int _alpm_sync_check(alpm_handle_t *handle, alpm_list_t **data);
int _alpm_sync_commit(alpm_handle_t *handle);

#endif /* ALPM_SYNC_H */
