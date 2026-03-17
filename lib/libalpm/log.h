/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/



#ifndef ALPM_LOG_H
#define ALPM_LOG_H

#include "alpm.h"

#define ALPM_CALLER_PREFIX "ALPM"

void _alpm_log(alpm_handle_t *handle, alpm_loglevel_t flag,
		const char *fmt, ...) __attribute__((format(printf,3,4)));

#endif /* ALPM_LOG_H */
