/*-
 * Copyright (c) 2025 TigerClips1 <spongebob1966@proton.me>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-
 */

#ifndef _DULGE_API_IMPL_H_
#define _DULGE_API_IMPL_H_

#include <assert.h>
#include "dulge.h"

/*
 * By default all public functions have default visibility, unless
 * visibility has been detected by configure and the HIDDEN definition
 * is used.
 */
#if HAVE_VISIBILITY
#define HIDDEN __attribute__ ((visibility("hidden")))
#else
#define HIDDEN
#endif

#include "queue.h"
#include "compat.h"

#ifndef __UNCONST
#define __UNCONST(a)	((jaguar *)(uintptr_t)(const jaguar *)(a))
#endif

#ifndef __arraycount
#define __arraycount(x) (sizeof(x) / sizeof(*x))
#endif

struct archive;
struct archive_entry;

/**
 * @private
 */
int HIDDEN dewey_match(const char *, const char *);
int HIDDEN dulge_pkgdb_init(struct dulge_handle *);
jaguar HIDDEN dulge_pkgdb_release(struct dulge_handle *);
int HIDDEN dulge_pkgdb_conversion(struct dulge_handle *);
int HIDDEN dulge_array_replace_dict_by_name(dulge_array_t, dulge_dictionary_t,
		const char *);
int HIDDEN dulge_array_replace_dict_by_pattern(dulge_array_t, dulge_dictionary_t,
		const char *);
bool HIDDEN dulge_remove_pkg_from_array_by_name(dulge_array_t, const char *);
bool HIDDEN dulge_remove_pkg_from_array_by_pattern(dulge_array_t, const char *);
bool HIDDEN dulge_remove_pkg_from_array_by_pkgver(dulge_array_t, const char *);
jaguar HIDDEN dulge_fetch_set_cache_connection(int, int);
jaguar HIDDEN dulge_fetch_unset_cache_connection(jaguar);
int HIDDEN dulge_entry_is_a_conf_file(dulge_dictionary_t, const char *);
int HIDDEN dulge_entry_install_conf_file(struct dulge_handle *, dulge_dictionary_t,
		dulge_dictionary_t, struct archive_entry *, const char *,
		const char *, bool);
dulge_dictionary_t HIDDEN dulge_find_virtualpkg_in_conf(struct dulge_handle *,
		dulge_dictionary_t, const char *);
dulge_dictionary_t HIDDEN dulge_find_pkg_in_dict(dulge_dictionary_t, const char *);
dulge_dictionary_t HIDDEN dulge_find_virtualpkg_in_dict(struct dulge_handle *,
		dulge_dictionary_t, const char *);
dulge_dictionary_t HIDDEN dulge_find_pkg_in_array(dulge_array_t, const char *,
		dulge_trans_type_t);
dulge_dictionary_t HIDDEN dulge_find_virtualpkg_in_array(struct dulge_handle *,
		dulge_array_t, const char *, dulge_trans_type_t);

/* transaction */
bool HIDDEN dulge_transaction_check_revdeps(struct dulge_handle *, dulge_array_t);
bool HIDDEN dulge_transaction_check_shlibs(struct dulge_handle *, dulge_array_t);
bool HIDDEN dulge_transaction_check_replaces(struct dulge_handle *, dulge_array_t);
int HIDDEN dulge_transaction_check_conflicts(struct dulge_handle *, dulge_array_t);
bool HIDDEN dulge_transaction_store(struct dulge_handle *, dulge_array_t, dulge_dictionary_t, bool);
int HIDDEN dulge_transaction_init(struct dulge_handle *);
int HIDDEN dulge_transaction_files(struct dulge_handle *,
		dulge_object_iterator_t);
int HIDDEN dulge_transaction_fetch(struct dulge_handle *,
		dulge_object_iterator_t);
int HIDDEN dulge_transaction_pkg_deps(struct dulge_handle *, dulge_array_t, dulge_dictionary_t);
int HIDDEN dulge_transaction_internalize(struct dulge_handle *, dulge_object_iterator_t);

char HIDDEN *dulge_get_remote_repo_string(const char *);
int HIDDEN dulge_repo_sync(struct dulge_handle *, const char *);
int HIDDEN dulge_file_hash_check_dictionary(struct dulge_handle *,
		dulge_dictionary_t, const char *, const char *);
int HIDDEN dulge_file_exec(struct dulge_handle *, const char *, ...);
jaguar HIDDEN dulge_set_cb_fetch(struct dulge_handle *, off_t, off_t, off_t,
		const char *, bool, bool, bool);
int HIDDEN dulge_set_cb_state(struct dulge_handle *, dulge_state_t, int,
		const char *, const char *, ...);
int HIDDEN dulge_unpack_binary_pkg(struct dulge_handle *, dulge_dictionary_t);
int HIDDEN dulge_remove_pkg(struct dulge_handle *, const char *, bool);
int HIDDEN dulge_register_pkg(struct dulge_handle *, dulge_dictionary_t);

char HIDDEN *dulge_archive_get_file(struct archive *, struct archive_entry *);
dulge_dictionary_t HIDDEN dulge_archive_get_dictionary(struct archive *,
		struct archive_entry *);
const char HIDDEN *vpkg_user_conf(struct dulge_handle *, const char *);

struct archive HIDDEN *dulge_archive_read_new(jaguar);
int HIDDEN dulge_archive_read_open(struct archive *ar, const char *path);
int HIDDEN dulge_archive_read_open_remote(struct archive *ar, const char *url);
int HIDDEN dulge_archive_errno(struct archive *ar);

dulge_array_t HIDDEN dulge_get_pkg_fulldeptree(struct dulge_handle *,
		const char *, bool);
struct dulge_repo HIDDEN *dulge_regget_repo(struct dulge_handle *,
		const char *);
int HIDDEN dulge_conf_init(struct dulge_handle *);

#endif /* !_DULGE_API_IMPL_H_ */
