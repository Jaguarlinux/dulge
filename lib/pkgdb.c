/*-
 * Copyright (c) 2012-2020 Juan Romero Pardines.
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

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dulge.h"
#include "dulge_api_impl.h"

/**
 * @file lib/pkgdb.c
 * @brief Package database handling routines
 * @defgroup pkgdb Package database handling functions
 *
 * Functions to manipulate the main package database plist file (pkgdb).
 *
 * The following image shown below shows the proplib structure used
 * by the main package database plist:
 *
 * @image html images/dulge_pkgdb_dictionary.png
 *
 * Legend:
 *  - <b>Salmon filled box</b>: \a pkgdb plist internalized.
 *  - <b>White filled box</b>: mandatory objects.
 *  - <b>Grey filled box</b>: optional objects.
 *  - <b>Green filled box</b>: possible value set in the object, only one
 *    of them is set.
 *
 * Text inside of white boxes are the key associated with the object, its
 * data type is specified on its edge, i.e array, bool, integer, string,
 * dictionary.
 */
static int pkgdb_fd = -1;
static bool pkgdb_map_names_done = false;

int
dulge_pkgdb_lock(struct dulge_handle *xhp)
{
	mode_t prev_umask;
	int rv = 0;
	/*
	 * Use a mandatory file lock to only allow one writer to pkgdb,
	 * other writers will block.
	 */
	prev_umask = umask(022);
	xhp->pkgdb_plist = dulge_xasprintf("%s/%s", xhp->metadir, DULGE_PKGDB);
	if (dulge_pkgdb_init(xhp) == ENOENT) {
		/* if metadir does not exist, create it */
		if (access(xhp->metadir, R_OK|X_OK) == -1) {
			if (errno != ENOENT) {
				rv = errno;
				goto ret;
			}
			if (dulge_mkpath(xhp->metadir, 0755) == -1) {
				rv = errno;
				dulge_dbg_printf("[pkgdb] failed to create metadir "
				    "%s: %s\n", xhp->metadir, strerror(rv));
				goto ret;
			}
		}
		/* if pkgdb is unexistent, create it with an empty dictionary */
		xhp->pkgdb = dulge_dictionary_create();
		if (!dulge_dictionary_externalize_to_file(xhp->pkgdb, xhp->pkgdb_plist)) {
			rv = errno;
			dulge_dbg_printf("[pkgdb] failed to create pkgdb "
			    "%s: %s\n", xhp->pkgdb_plist, strerror(rv));
			goto ret;
		}
	}

	if ((pkgdb_fd = open(xhp->pkgdb_plist, O_CREAT|O_RDWR|O_CLOEXEC, 0664)) == -1) {
		rv = errno;
		dulge_dbg_printf("[pkgdb] cannot open pkgdb for locking "
		    "%s: %s\n", xhp->pkgdb_plist, strerror(rv));
		free(xhp->pkgdb_plist);
		goto ret;
	}

	/*
	 * If we've acquired the file lock, then pkgdb is writable.
	 */
	if (lockf(pkgdb_fd, F_TLOCK, 0) == -1) {
		rv = errno;
		dulge_dbg_printf("[pkgdb] cannot lock pkgdb: %s\n", strerror(rv));
	}
	/*
	 * Check if rootdir is writable.
	 */
	if (access(xhp->rootdir, W_OK) == -1) {
		rv = errno;
		dulge_dbg_printf("[pkgdb] rootdir %s: %s\n", xhp->rootdir, strerror(rv));
	}

ret:
	umask(prev_umask);
	return rv;
}

void
dulge_pkgdb_unlock(struct dulge_handle *xhp UNUSED)
{
	dulge_dbg_printf("%s: pkgdb_fd %d\n", __func__, pkgdb_fd);

	if (pkgdb_fd != -1) {
		if (lockf(pkgdb_fd, F_ULOCK, 0) == -1)
			dulge_dbg_printf("[pkgdb] failed to unlock pkgdb: %s\n", strerror(errno));

		(void)close(pkgdb_fd);
		pkgdb_fd = -1;
	}
}

static int
pkgdb_map_vpkgs(struct dulge_handle *xhp)
{
	dulge_object_iterator_t iter;
	dulge_object_t obj;
	int r = 0;

	if (!dulge_dictionary_count(xhp->pkgdb))
		return 0;

	if (xhp->vpkgd == NULL) {
		xhp->vpkgd = dulge_dictionary_create();
		if (!xhp->vpkgd) {
			r = -errno;
			dulge_error_printf("failed to create dictionary\n");
			return r;
		}
	}

	/*
	 * This maps all pkgs that have virtualpkgs in pkgdb.
	 */
	iter = dulge_dictionary_iterator(xhp->pkgdb);
	if (!iter) {
		r = -errno;
		dulge_error_printf("failed to create iterator");
		return r;
	}

	while ((obj = dulge_object_iterator_next(iter))) {
		dulge_array_t provides;
		dulge_dictionary_t pkgd;
		const char *pkgver = NULL;
		const char *pkgname = NULL;
		unsigned int cnt;

		pkgd = dulge_dictionary_get_keysym(xhp->pkgdb, obj);
		provides = dulge_dictionary_get(pkgd, "provides");
		cnt = dulge_array_count(provides);
		if (!cnt)
			continue;

		dulge_dictionary_get_cstring_nocopy(pkgd, "pkgver", &pkgver);
		dulge_dictionary_get_cstring_nocopy(pkgd, "pkgname", &pkgname);
		assert(pkgname);

		for (unsigned int i = 0; i < cnt; i++) {
			char vpkgname[DULGE_NAME_SIZE];
			const char *vpkg = NULL;
			dulge_dictionary_t providers;
			bool alloc = false;

			dulge_array_get_cstring_nocopy(provides, i, &vpkg);
			if (!dulge_pkg_name(vpkgname, sizeof(vpkgname), vpkg)) {
				dulge_warn_printf("%s: invalid provides: %s\n", pkgver, vpkg);
				continue;
			}

			providers = dulge_dictionary_get(xhp->vpkgd, vpkgname);
			if (!providers) {
				providers = dulge_dictionary_create();
				if (!providers) {
					r = -errno;
					dulge_error_printf("failed to create dictionary\n");
					goto out;
				}
				if (!dulge_dictionary_set(xhp->vpkgd, vpkgname, providers)) {
					r = -errno;
					dulge_error_printf("failed to set dictionary entry\n");
					dulge_object_release(providers);
					goto out;
				}
				alloc = true;
			}

			if (!dulge_dictionary_set_cstring(providers, vpkg, pkgname)) {
				r = -errno;
				dulge_error_printf("failed to set dictionary entry\n");
				if (alloc)
					dulge_object_release(providers);
				goto out;
			}
			if (alloc)
				dulge_object_release(providers);
			dulge_dbg_printf("[pkgdb] added vpkg %s for %s\n", vpkg, pkgname);
		}
	}
out:
	dulge_object_iterator_release(iter);
	return r;
}

static int
pkgdb_map_names(struct dulge_handle *xhp)
{
	dulge_object_iterator_t iter;
	dulge_object_t obj;
	int rv = 0;

	if (pkgdb_map_names_done || !dulge_dictionary_count(xhp->pkgdb))
		return 0;

	/*
	 * This maps all pkgs in pkgdb to have the "pkgname" string property.
	 * This way we do it once and not multiple times.
	 */
	iter = dulge_dictionary_iterator(xhp->pkgdb);
	assert(iter);

	while ((obj = dulge_object_iterator_next(iter))) {
		dulge_dictionary_t pkgd;
		const char *pkgver;
		char pkgname[DULGE_NAME_SIZE] = {0};

		pkgd = dulge_dictionary_get_keysym(xhp->pkgdb, obj);
		if (!dulge_dictionary_get_cstring_nocopy(pkgd, "pkgver", &pkgver)) {
			continue;
		}
		if (!dulge_pkg_name(pkgname, sizeof(pkgname), pkgver)) {
			rv = EINVAL;
			break;
		}
		if (!dulge_dictionary_set_cstring(pkgd, "pkgname", pkgname)) {
			rv = EINVAL;
			break;
		}
	}
	dulge_object_iterator_release(iter);
	if (!rv) {
		pkgdb_map_names_done = true;
	}
	return rv;
}

int HIDDEN
dulge_pkgdb_init(struct dulge_handle *xhp)
{
	int rv;

	assert(xhp);

	if (xhp->pkgdb)
		return 0;

	if (!xhp->pkgdb_plist)
		xhp->pkgdb_plist = dulge_xasprintf("%s/%s", xhp->metadir, DULGE_PKGDB);

#if 0
	if ((rv = dulge_pkgdb_conversion(xhp)) != 0)
		return rv;
#endif


	if ((rv = dulge_pkgdb_update(xhp, false, true)) != 0) {
		if (rv != ENOENT)
			dulge_dbg_printf("[pkgdb] cannot internalize "
			    "pkgdb dictionary: %s\n", strerror(rv));

		return rv;
	}
	if ((rv = pkgdb_map_names(xhp)) != 0) {
		dulge_dbg_printf("[pkgdb] pkgdb_map_names %s\n", strerror(rv));
		return rv;
	}
	if ((rv = pkgdb_map_vpkgs(xhp)) != 0) {
		dulge_dbg_printf("[pkgdb] pkgdb_map_vpkgs %s\n", strerror(rv));
		return rv;
	}
	assert(xhp->pkgdb);
	dulge_dbg_printf("[pkgdb] initialized ok.\n");

	return 0;
}

int
dulge_pkgdb_update(struct dulge_handle *xhp, bool flush, bool update)
{
	dulge_dictionary_t pkgdb_storage;
	mode_t prev_umask;
	static int cached_rv;
	int rv = 0;

	if (cached_rv && !flush)
		return cached_rv;

	if (xhp->pkgdb && flush) {
		pkgdb_storage = dulge_dictionary_internalize_from_file(xhp->pkgdb_plist);
		if (pkgdb_storage == NULL ||
		    !dulge_dictionary_equals(xhp->pkgdb, pkgdb_storage)) {
			/* flush dictionary to storage */
			prev_umask = umask(022);
			if (!dulge_dictionary_externalize_to_file(xhp->pkgdb, xhp->pkgdb_plist)) {
				umask(prev_umask);
				return errno;
			}
			umask(prev_umask);
		}
		if (pkgdb_storage)
			dulge_object_release(pkgdb_storage);

		dulge_object_release(xhp->pkgdb);
		xhp->pkgdb = NULL;
		cached_rv = 0;
	}
	if (!update)
		return rv;

	/* update copy in memory */
	if ((xhp->pkgdb = dulge_dictionary_internalize_from_file(xhp->pkgdb_plist)) == NULL) {
		rv = errno;
		if (!rv)
			rv = EINVAL;

		if (rv == ENOENT)
			xhp->pkgdb = dulge_dictionary_create();
		else
			dulge_error_printf("cannot access to pkgdb: %s\n", strerror(rv));

		cached_rv = rv = errno;
	}

	return rv;
}

void HIDDEN
dulge_pkgdb_release(struct dulge_handle *xhp)
{
	assert(xhp);

	dulge_pkgdb_unlock(xhp);
	if (xhp->pkgdb)
		dulge_object_release(xhp->pkgdb);
	dulge_dbg_printf("[pkgdb] released ok.\n");
}

int
dulge_pkgdb_foreach_cb(struct dulge_handle *xhp,
		int (*fn)(struct dulge_handle *, dulge_object_t, const char *, void *, bool *),
		void *arg)
{
	dulge_array_t allkeys;
	int rv;

	if ((rv = dulge_pkgdb_init(xhp)) != 0)
		return rv;

	allkeys = dulge_dictionary_all_keys(xhp->pkgdb);
	assert(allkeys);
	rv = dulge_array_foreach_cb(xhp, allkeys, xhp->pkgdb, fn, arg);
	dulge_object_release(allkeys);
	return rv;
}

int
dulge_pkgdb_foreach_cb_multi(struct dulge_handle *xhp,
		int (*fn)(struct dulge_handle *, dulge_object_t, const char *, void *, bool *),
		void *arg)
{
	dulge_array_t allkeys;
	int rv;

	if ((rv = dulge_pkgdb_init(xhp)) != 0)
		return rv;

	allkeys = dulge_dictionary_all_keys(xhp->pkgdb);
	assert(allkeys);
	rv = dulge_array_foreach_cb_multi(xhp, allkeys, xhp->pkgdb, fn, arg);
	dulge_object_release(allkeys);
	return rv;
}

dulge_dictionary_t
dulge_pkgdb_get_pkg(struct dulge_handle *xhp, const char *pkg)
{
	dulge_dictionary_t pkgd;

	if (dulge_pkgdb_init(xhp) != 0)
		return NULL;

	pkgd = dulge_find_pkg_in_dict(xhp->pkgdb, pkg);
	if (!pkgd)
		errno = ENOENT;
	return pkgd;
}

dulge_dictionary_t
dulge_pkgdb_get_virtualpkg(struct dulge_handle *xhp, const char *vpkg)
{
	if (dulge_pkgdb_init(xhp) != 0)
		return NULL;

	return dulge_find_virtualpkg_in_dict(xhp, xhp->pkgdb, vpkg);
}

static void
generate_full_revdeps_tree(struct dulge_handle *xhp)
{
	dulge_object_t obj;
	dulge_object_iterator_t iter;
	dulge_dictionary_t vpkg_cache;

	if (xhp->pkgdb_revdeps)
		return;

	xhp->pkgdb_revdeps = dulge_dictionary_create();
	assert(xhp->pkgdb_revdeps);

	vpkg_cache = dulge_dictionary_create();
	assert(vpkg_cache);

	iter = dulge_dictionary_iterator(xhp->pkgdb);
	assert(iter);

	while ((obj = dulge_object_iterator_next(iter))) {
		dulge_array_t rundeps;
		dulge_dictionary_t pkgd;
		const char *pkgver = NULL;

		pkgd = dulge_dictionary_get_keysym(xhp->pkgdb, obj);
		rundeps = dulge_dictionary_get(pkgd, "run_depends");
		if (!dulge_array_count(rundeps))
			continue;

		dulge_dictionary_get_cstring_nocopy(pkgd, "pkgver", &pkgver);
		for (unsigned int i = 0; i < dulge_array_count(rundeps); i++) {
			dulge_array_t pkg;
			const char *pkgdep = NULL, *v;
			char curpkgname[DULGE_NAME_SIZE];
			bool alloc = false;

			dulge_array_get_cstring_nocopy(rundeps, i, &pkgdep);
			if ((!dulge_pkgpattern_name(curpkgname, sizeof(curpkgname), pkgdep)) &&
			    (!dulge_pkg_name(curpkgname, sizeof(curpkgname), pkgdep))) {
					abort();
			}

			/* TODO: this is kind of a workaround, to avoid calling vpkg_user_conf
			 * over and over again for the same packages which is slow. A better
			 * solution for itself vpkg_user_conf being slow should probably be
			 * implemented at some point.
			 */
			if (!dulge_dictionary_get_cstring_nocopy(vpkg_cache, curpkgname, &v)) {
				const char *vpkgname = vpkg_user_conf(xhp, curpkgname);
				if (vpkgname) {
					v = vpkgname;
				} else {
					v = curpkgname;
				}
				errno = 0;
				if (!dulge_dictionary_set_cstring_nocopy(vpkg_cache, curpkgname, v)) {
					dulge_error_printf("%s\n", strerror(errno ? errno : ENOMEM));
					abort();
				}
			}

			pkg = dulge_dictionary_get(xhp->pkgdb_revdeps, v);
			if (pkg == NULL) {
				alloc = true;
				pkg = dulge_array_create();
			}
			if (!dulge_match_string_in_array(pkg, pkgver)) {
				dulge_array_add_cstring_nocopy(pkg, pkgver);
				dulge_dictionary_set(xhp->pkgdb_revdeps, v, pkg);
			}
			if (alloc)
				dulge_object_release(pkg);
		}
	}
	dulge_object_iterator_release(iter);
	dulge_object_release(vpkg_cache);
}

dulge_array_t
dulge_pkgdb_get_pkg_revdeps(struct dulge_handle *xhp, const char *pkg)
{
	dulge_dictionary_t pkgd;
	const char *pkgver = NULL;
	char pkgname[DULGE_NAME_SIZE];

	if ((pkgd = dulge_pkgdb_get_pkg(xhp, pkg)) == NULL)
		return NULL;

	generate_full_revdeps_tree(xhp);
	dulge_dictionary_get_cstring_nocopy(pkgd, "pkgver", &pkgver);
	if (!dulge_pkg_name(pkgname, sizeof(pkgname), pkgver)) 
		return NULL;

	return dulge_dictionary_get(xhp->pkgdb_revdeps, pkgname);
}

dulge_array_t
dulge_pkgdb_get_pkg_fulldeptree(struct dulge_handle *xhp, const char *pkg)
{
	return dulge_get_pkg_fulldeptree(xhp, pkg, false);
}

dulge_dictionary_t
dulge_pkgdb_get_pkg_files(struct dulge_handle *xhp, const char *pkg)
{
	dulge_dictionary_t pkgd;
	const char *pkgver = NULL;
	char pkgname[DULGE_NAME_SIZE], plist[PATH_MAX];

	if (pkg == NULL)
		return NULL;

	pkgd = dulge_pkgdb_get_pkg(xhp, pkg);
	if (pkgd == NULL)
		return NULL;

	dulge_dictionary_get_cstring_nocopy(pkgd, "pkgver", &pkgver);
	if (!dulge_pkg_name(pkgname, sizeof(pkgname), pkgver))
		return NULL;

	snprintf(plist, sizeof(plist)-1, "%s/.%s-files.plist", xhp->metadir, pkgname);
	return dulge_plist_dictionary_from_file(plist);
}
