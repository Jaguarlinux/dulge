/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/



#ifndef ALPM_SANDBOX_FS_H
#define ALPM_SANDBOX_FS_H

#include <stdbool.h>
#include "alpm.h"

bool _alpm_sandbox_fs_restrict_writes_to(alpm_handle_t *handle, const char *path);

#endif /* ALPM_SANDBOX_FS_H */
