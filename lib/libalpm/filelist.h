/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/



#ifndef ALPM_FILELIST_H
#define ALPM_FILELIST_H

#include "alpm.h"

alpm_list_t *_alpm_filelist_difference(alpm_filelist_t *filesA,
		alpm_filelist_t *filesB);

alpm_list_t *_alpm_filelist_intersection(alpm_filelist_t *filesA,
		alpm_filelist_t *filesB);

void _alpm_filelist_sort(alpm_filelist_t *filelist);

#endif /* ALPM_FILELIST_H */
