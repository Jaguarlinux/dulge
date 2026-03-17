/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/



#ifndef ALPM_GROUP_H
#define ALPM_GROUP_H

#include "alpm.h"

alpm_group_t *_alpm_group_new(const char *name);
void _alpm_group_free(alpm_group_t *grp);

#endif /* ALPM_GROUP_H */
