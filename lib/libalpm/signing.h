/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/



#ifndef ALPM_SIGNING_H
#define ALPM_SIGNING_H

#include "alpm.h"

char *_alpm_sigpath(alpm_handle_t *handle, const char *path);
int _alpm_gpgme_checksig(alpm_handle_t *handle, const char *path,
		const char *base64_sig, alpm_siglist_t *result);

int _alpm_check_pgp_helper(alpm_handle_t *handle, const char *path,
		const char *base64_sig, int optional, int marginal, int unknown,
		alpm_siglist_t **sigdata);
int _alpm_process_siglist(alpm_handle_t *handle, const char *identifier,
		alpm_siglist_t *siglist, int optional, int marginal, int unknown);

int _alpm_key_in_keychain(alpm_handle_t *handle, const char *fpr);
int _alpm_key_import(alpm_handle_t *handle, const char *uid, const char *fpr);

#endif /* ALPM_SIGNING_H */
