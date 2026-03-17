/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/



#ifndef ALPM_TRANS_H
#define ALPM_TRANS_H

#include "alpm.h"

typedef enum _alpm_transstate_t {
	STATE_IDLE = 0,
	STATE_INITIALIZED,
	STATE_PREPARED,
	STATE_DOWNLOADING,
	STATE_COMMITTING,
	STATE_COMMITTED,
	STATE_INTERRUPTED
} alpm_transstate_t;

/* Transaction */
typedef struct _alpm_trans_t {
	/* bitfield of alpm_transflag_t flags */
	int flags;
	alpm_transstate_t state;
	alpm_list_t *unresolvable;  /* list of (alpm_pkg_t *) */
	alpm_list_t *add;           /* list of (alpm_pkg_t *) */
	alpm_list_t *remove;        /* list of (alpm_pkg_t *) */
	alpm_list_t *skip_remove;   /* list of (char *) */
} alpm_trans_t;

void _alpm_trans_free(alpm_trans_t *trans);
/* flags is a bitfield of alpm_transflag_t flags */
int _alpm_trans_init(alpm_trans_t *trans, int flags);
int _alpm_runscriptlet(alpm_handle_t *handle, const char *filepath,
		const char *script, const char *ver, const char *oldver, int is_archive);

#endif /* ALPM_TRANS_H */
