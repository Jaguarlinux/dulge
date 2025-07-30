/*-
 * Copyright (c) 2009-2020 Juan Romero Pardines.
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

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/statvfs.h>

#include "dulge_api_impl.h"

/**
 * @file lib/transaction_prepare.c
 * @brief Transaction handling routines
 * @defgroup transaction Transaction handling functions
 *
 * The following image shows off the full transaction dictionary returned
 * by dulge_transaction_prepare().
 *
 * @image html images/dulge_transaction_dictionary.png
 *
 * Legend:
 *  - <b>Salmon bg box</b>: The transaction dictionary.
 *  - <b>White bg box</b>: mandatory objects.
 *  - <b>Grey bg box</b>: optional objects.
 *  - <b>Green bg box</b>: possible value set in the object, only one of them
 *    will be set.
 *
 * Text inside of white boxes are the key associated with the object, its
 * data type is specified on its edge, i.e string, array, integer, dictionary.
 */

static int
compute_transaction_stats(struct dulge_handle *xhp)
{
	dulge_dictionary_t pkg_metad;
	dulge_object_iterator_t iter;
	dulge_object_t obj;
	struct statvfs svfs;
	uint64_t rootdir_free_size, tsize, dlsize, instsize, rmsize;
	uint32_t inst_pkgcnt, up_pkgcnt, cf_pkgcnt, rm_pkgcnt, dl_pkgcnt;
	uint32_t hold_pkgcnt;

	inst_pkgcnt = up_pkgcnt = cf_pkgcnt = rm_pkgcnt = 0;
	hold_pkgcnt = dl_pkgcnt = 0;
	tsize = dlsize = instsize = rmsize = 0;

	iter = dulge_array_iter_from_dict(xhp->transd, "packages");
	if (iter == NULL)
		return EINVAL;

	while ((obj = dulge_object_iterator_next(iter)) != NULL) {
		const char *pkgver = NULL, *repo = NULL, *pkgname = NULL;
		bool preserve = false;
		dulge_trans_type_t ttype;
		/*
		 * Count number of pkgs to be removed, configured,
		 * installed and updated.
		 */
		dulge_dictionary_get_cstring_nocopy(obj, "pkgver", &pkgver);
		dulge_dictionary_get_cstring_nocopy(obj, "pkgver", &pkgname);
		dulge_dictionary_get_cstring_nocopy(obj, "repository", &repo);
		dulge_dictionary_get_bool(obj, "preserve", &preserve);
		ttype = dulge_transaction_pkg_type(obj);

		if (ttype == DULGE_TRANS_REMOVE) {
			rm_pkgcnt++;
		} else if (ttype == DULGE_TRANS_CONFIGURE) {
			cf_pkgcnt++;
		} else if (ttype == DULGE_TRANS_INSTALL || ttype == DULGE_TRANS_REINSTALL) {
			inst_pkgcnt++;
		} else if (ttype == DULGE_TRANS_UPDATE) {
			up_pkgcnt++;
		} else if (ttype == DULGE_TRANS_HOLD) {
			hold_pkgcnt++;
		}

		if ((ttype != DULGE_TRANS_CONFIGURE) && (ttype != DULGE_TRANS_REMOVE) &&
		    (ttype != DULGE_TRANS_HOLD) &&
		    dulge_repository_is_remote(repo) && !dulge_binpkg_exists(xhp, obj)) {
			dulge_dictionary_get_uint64(obj, "filename-size", &tsize);
			tsize += 512;
			dlsize += tsize;
			dl_pkgcnt++;
			dulge_dictionary_set_bool(obj, "download", true);
		}
		if (xhp->flags & DULGE_FLAG_DOWNLOAD_ONLY) {
			continue;
		}
		/* installed_size from repo */
		if (ttype != DULGE_TRANS_REMOVE && ttype != DULGE_TRANS_HOLD &&
		    ttype != DULGE_TRANS_CONFIGURE) {
			dulge_dictionary_get_uint64(obj, "installed_size", &tsize);
			instsize += tsize;
		}
		/*
		 * If removing or updating a package without preserve,
		 * get installed_size from pkgdb instead.
		 */
		if (ttype == DULGE_TRANS_REMOVE ||
		   ((ttype == DULGE_TRANS_UPDATE) && !preserve)) {
			pkg_metad = dulge_pkgdb_get_pkg(xhp, pkgname);
			if (pkg_metad == NULL)
				continue;
			dulge_dictionary_get_uint64(pkg_metad,
			    "installed_size", &tsize);
			rmsize += tsize;
		}
	}
	dulge_object_iterator_release(iter);

	if (instsize > rmsize) {
		instsize -= rmsize;
		rmsize = 0;
	} else if (rmsize > instsize) {
		rmsize -= instsize;
		instsize = 0;
	} else {
		instsize = rmsize = 0;
	}

	if (!dulge_dictionary_set_uint32(xhp->transd,
				"total-install-pkgs", inst_pkgcnt))
		return EINVAL;
	if (!dulge_dictionary_set_uint32(xhp->transd,
				"total-update-pkgs", up_pkgcnt))
		return EINVAL;
	if (!dulge_dictionary_set_uint32(xhp->transd,
				"total-configure-pkgs", cf_pkgcnt))
		return EINVAL;
	if (!dulge_dictionary_set_uint32(xhp->transd,
				"total-remove-pkgs", rm_pkgcnt))
		return EINVAL;
	if (!dulge_dictionary_set_uint32(xhp->transd,
				"total-download-pkgs", dl_pkgcnt))
		return EINVAL;
	if (!dulge_dictionary_set_uint32(xhp->transd,
				"total-hold-pkgs", hold_pkgcnt))
		return EINVAL;
	if (!dulge_dictionary_set_uint64(xhp->transd,
				"total-installed-size", instsize))
		return EINVAL;
	if (!dulge_dictionary_set_uint64(xhp->transd,
				"total-download-size", dlsize))
		return EINVAL;
	if (!dulge_dictionary_set_uint64(xhp->transd,
				"total-removed-size", rmsize))
		return EINVAL;

	/* Get free space from target rootdir: return ENOSPC if there's not enough space */
	if (statvfs(xhp->rootdir, &svfs) == -1) {
		dulge_dbg_printf("%s: statvfs failed: %s\n", __func__, strerror(errno));
		return 0;
	}
	/* compute free space on disk */
	rootdir_free_size = svfs.f_bfree * svfs.f_bsize;

	if (!dulge_dictionary_set_uint64(xhp->transd,
				"disk-free-size", rootdir_free_size))
		return EINVAL;

	if (instsize > rootdir_free_size)
		return ENOSPC;

	return 0;
}

int HIDDEN
dulge_transaction_init(struct dulge_handle *xhp)
{
	dulge_array_t array;
	dulge_dictionary_t dict;

	if (xhp->transd != NULL)
		return 0;

	if ((xhp->transd = dulge_dictionary_create()) == NULL)
		return ENOMEM;

	if ((array = dulge_array_create()) == NULL) {
		dulge_object_release(xhp->transd);
		xhp->transd = NULL;
		return ENOMEM;
	}
	if (!dulge_dictionary_set(xhp->transd, "packages", array)) {
		dulge_object_release(xhp->transd);
		xhp->transd = NULL;
		return EINVAL;
	}
	dulge_object_release(array);

	if ((array = dulge_array_create()) == NULL) {
		dulge_object_release(xhp->transd);
		xhp->transd = NULL;
		return ENOMEM;
	}
	if (!dulge_dictionary_set(xhp->transd, "missing_deps", array)) {
		dulge_object_release(xhp->transd);
		xhp->transd = NULL;
		return EINVAL;
	}
	dulge_object_release(array);

	if ((array = dulge_array_create()) == NULL) {
		dulge_object_release(xhp->transd);
		xhp->transd = NULL;
		return ENOMEM;
	}
	if (!dulge_dictionary_set(xhp->transd, "missing_shlibs", array)) {
		dulge_object_release(xhp->transd);
		xhp->transd = NULL;
		return EINVAL;
	}
	dulge_object_release(array);

	if ((array = dulge_array_create()) == NULL) {
		dulge_object_release(xhp->transd);
		xhp->transd = NULL;
		return ENOMEM;
	}
	if (!dulge_dictionary_set(xhp->transd, "conflicts", array)) {
		dulge_object_release(xhp->transd);
		xhp->transd = NULL;
		return EINVAL;
	}
	dulge_object_release(array);

	if ((dict = dulge_dictionary_create()) == NULL) {
		dulge_object_release(xhp->transd);
		xhp->transd = NULL;
		return ENOMEM;
	}
	if (!dulge_dictionary_set(xhp->transd, "obsolete_files", dict)) {
		dulge_object_release(xhp->transd);
		xhp->transd = NULL;
		return EINVAL;
	}
	dulge_object_release(dict);

	if ((dict = dulge_dictionary_create()) == NULL) {
		dulge_object_release(xhp->transd);
		xhp->transd = NULL;
		return ENOMEM;
	}
	if (!dulge_dictionary_set(xhp->transd, "remove_files", dict)) {
		dulge_object_release(xhp->transd);
		xhp->transd = NULL;
		return EINVAL;
	}
	dulge_object_release(dict);

	return 0;
}

int
dulge_transaction_prepare(struct dulge_handle *xhp)
{
	dulge_array_t pkgs, edges;
	dulge_dictionary_t tpkgd;
	dulge_trans_type_t ttype;
	unsigned int i, cnt;
	int rv = 0;
	bool all_on_hold = true;

	if ((rv = dulge_transaction_init(xhp)) != 0)
		return rv;

	if (xhp->transd == NULL)
		return ENXIO;

	/*
	 * Collect dependencies for pkgs in transaction.
	 */
	if ((edges = dulge_array_create()) == NULL)
		return ENOMEM;

	dulge_dbg_printf("%s: processing deps\n", __func__);
	/*
	 * The edges are also appended after its dependencies have been
	 * collected; the edges at the original array are removed later.
	 */
	pkgs = dulge_dictionary_get(xhp->transd, "packages");
	assert(dulge_object_type(pkgs) == DULGE_TYPE_ARRAY);
	cnt = dulge_array_count(pkgs);
	for (i = 0; i < cnt; i++) {
		dulge_dictionary_t pkgd;
		dulge_string_t str;

		pkgd = dulge_array_get(pkgs, i);
		str = dulge_dictionary_get(pkgd, "pkgver");
		ttype = dulge_transaction_pkg_type(pkgd);

		if (ttype == DULGE_TRANS_REMOVE || ttype == DULGE_TRANS_HOLD)
			continue;

		assert(dulge_object_type(str) == DULGE_TYPE_STRING);

		if (!dulge_array_add(edges, str)) {
			dulge_object_release(edges);
			return ENOMEM;
		}
		if ((rv = dulge_transaction_pkg_deps(xhp, pkgs, pkgd)) != 0) {
			dulge_object_release(edges);
			return rv;
		}
		if (!dulge_array_add(pkgs, pkgd)) {
			dulge_object_release(edges);
			return ENOMEM;
		}
	}
	/* ... remove dup edges at head */
	for (i = 0; i < dulge_array_count(edges); i++) {
		const char *pkgver = NULL;
		dulge_array_get_cstring_nocopy(edges, i, &pkgver);
		dulge_remove_pkg_from_array_by_pkgver(pkgs, pkgver);
	}
	dulge_object_release(edges);

	/*
	 * Do not perform any checks if DULGE_FLAG_DOWNLOAD_ONLY
	 * is set. We just need to download the archives (dependencies).
	 */
	if (xhp->flags & DULGE_FLAG_DOWNLOAD_ONLY)
		goto out;

	/*
	 * If all pkgs in transaction are on hold, no need to check
	 * for anything else.
	 */
	dulge_dbg_printf("%s: checking on hold pkgs\n", __func__);
	for (i = 0; i < cnt; i++) {
		tpkgd = dulge_array_get(pkgs, i);
		if (dulge_transaction_pkg_type(tpkgd) != DULGE_TRANS_HOLD) {
			all_on_hold = false;
			break;
		}
	}
	if (all_on_hold)
		goto out;

	/*
	 * Check for packages to be replaced.
	 */
	dulge_dbg_printf("%s: checking replaces\n", __func__);
	if (!dulge_transaction_check_replaces(xhp, pkgs)) {
		dulge_object_release(xhp->transd);
		xhp->transd = NULL;
		return EINVAL;
	}
	/*
	 * Check if there are missing revdeps.
	 */
	dulge_dbg_printf("%s: checking revdeps\n", __func__);
	if (!dulge_transaction_check_revdeps(xhp, pkgs)) {
		dulge_object_release(xhp->transd);
		xhp->transd = NULL;
		return EINVAL;
	}
	if (dulge_dictionary_get(xhp->transd, "missing_deps")) {
		if (xhp->flags & DULGE_FLAG_FORCE_REMOVE_REVDEPS) {
			dulge_dbg_printf("[trans] continuing with broken reverse dependencies!");
		} else {
			return ENODEV;
		}
	}
	/*
	 * Check for package conflicts.
	 */
	dulge_dbg_printf("%s: checking conflicts\n", __func__);
	if (!dulge_transaction_check_conflicts(xhp, pkgs)) {
		dulge_object_release(xhp->transd);
		xhp->transd = NULL;
		return EINVAL;
	}
	if (dulge_dictionary_get(xhp->transd, "conflicts")) {
		return EAGAIN;
	}
	/*
	 * Check for unresolved shared libraries.
	 */
	dulge_dbg_printf("%s: checking shlibs\n", __func__);
	if (!dulge_transaction_check_shlibs(xhp, pkgs)) {
		dulge_object_release(xhp->transd);
		xhp->transd = NULL;
		return EINVAL;
	}
	if (dulge_dictionary_get(xhp->transd, "missing_shlibs")) {
		if (xhp->flags & DULGE_FLAG_FORCE_REMOVE_REVDEPS) {
			dulge_dbg_printf("[trans] continuing with unresolved shared libraries!");
		} else {
			return ENOEXEC;
		}
	}
out:
	/*
	 * Add transaction stats for total download/installed size,
	 * number of packages to be installed, updated, configured
	 * and removed to the transaction dictionary.
	 */
	dulge_dbg_printf("%s: computing stats\n", __func__);
	if ((rv = compute_transaction_stats(xhp)) != 0) {
		return rv;
	}
	/*
	 * Make transaction dictionary immutable.
	 */
	dulge_dictionary_make_immutable(xhp->transd);

	return 0;
}
