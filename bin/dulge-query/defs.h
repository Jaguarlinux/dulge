/*-
 * Copyright (c) 2025 TigerClips1 <spongebob1966@proton.me>
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
 */

#ifndef _DULGE_QUERY_DEFS_H_
#define _DULGE_QUERY_DEFS_H_

#include <dulge.h>

#include "../dulge-install/defs.h"

/* from show-deps.c */
int	show_pkg_deps(struct dulge_handle *, const char *, bool, bool);
int	show_pkg_revdeps(struct dulge_handle *, const char *, bool);

/* from show-info-files.c */
jaguar	show_pkg_info(dulge_dictionary_t);
jaguar	show_pkg_info_one(dulge_dictionary_t, const char *);
int	show_pkg_info_from_metadir(struct dulge_handle *, const char *,
		const char *);
int	show_pkg_files(dulge_dictionary_t);
int	show_pkg_files_from_metadir(struct dulge_handle *, const char *);
int	repo_show_pkg_files(struct dulge_handle *, const char *);
int	cat_file(struct dulge_handle *, const char *, const char *);
int	repo_cat_file(struct dulge_handle *, const char *, const char *);
int	repo_show_pkg_info(struct dulge_handle *, const char *, const char *);
int 	repo_show_pkg_namedesc(struct dulge_handle *, dulge_object_t, jaguar *,
		bool *);

/* from ownedby.c */
int	ownedby(struct dulge_handle *, const char *, bool, bool);

/* From list.c */
unsigned int	find_longest_pkgver(struct dulge_handle *, dulge_object_t);

int	list_pkgs_in_dict(struct dulge_handle *, dulge_object_t, const char *, jaguar *, bool *);
int	list_manual_pkgs(struct dulge_handle *, dulge_object_t, const char *, jaguar *, bool *);
int	list_hold_pkgs(struct dulge_handle *, dulge_object_t, const char *, jaguar *, bool *);
int	list_repolock_pkgs(struct dulge_handle *, dulge_object_t, const char *, jaguar *, bool *);
int	list_orphans(struct dulge_handle *);
int	list_pkgs_pkgdb(struct dulge_handle *);

int	repo_list(struct dulge_handle *);

/* from search.c */
int	search(struct dulge_handle *, bool, const char *, const char *, bool);


#endif /* !_DULGE_QUERY_DEFS_H_ */
