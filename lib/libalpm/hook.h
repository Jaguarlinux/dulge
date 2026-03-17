/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/




#ifndef ALPM_HOOK_H
#define ALPM_HOOK_H

#include "alpm.h"

#define ALPM_HOOK_SUFFIX ".hook"

int _alpm_hook_run(alpm_handle_t *handle, alpm_hook_when_t when);

#endif /* ALPM_HOOK_H */
