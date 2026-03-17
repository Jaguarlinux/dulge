/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/




#ifndef PM_CHECK_H
#define PM_CHECK_H

#include <alpm.h>

int check_pkg_fast(alpm_pkg_t *pkg);
int check_pkg_full(alpm_pkg_t *pkg);

#endif /* PM_CHECK_H */
