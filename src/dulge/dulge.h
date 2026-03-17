/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/



#ifndef PM_PACMAN_H
#define PM_PACMAN_H

#include <alpm_list.h>

#define PACMAN_CALLER_PREFIX "DULGE"

/* database.c */
int dulge_database(alpm_list_t *targets);
/* deptest.c */
int dulge_deptest(alpm_list_t *targets);
/* files.c */
int dulge_files(alpm_list_t *files);
/* query.c */
int dulge_query(alpm_list_t *targets);
/* remove.c */
int dulge_remove(alpm_list_t *targets);
/* sync.c */
int dulge_sync(alpm_list_t *targets);
int sync_prepare_execute(void);
/* upgrade.c */
int dulge_upgrade(alpm_list_t *targets);

#endif /* PM_PACMAN_H */
