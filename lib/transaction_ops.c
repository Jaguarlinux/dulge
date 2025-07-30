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
#include <fnmatch.h>

#include "dulge_api_impl.h"

/**
 * @file lib/transaction_ops.c
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
trans_find_pkg(struct dulge_handle *xhp, const char *pkg, bool force)
{
	dulge_dictionary_t pkg_pkgdb = NULL, pkg_repod = NULL;
	dulge_object_t obj;
	dulge_array_t pkgs;
	pkg_state_t state = 0;
	dulge_trans_type_t ttype;
	const char *repoloc, *repopkgver, *instpkgver, *pkgname;
	char buf[DULGE_NAME_SIZE] = {0};
	bool autoinst = false;
	int rv = 0;

	assert(pkg != NULL);

	/*
	 * Find out if pkg is installed first.
	 */
	if (dulge_pkg_name(buf, sizeof(buf), pkg)) {
		pkg_pkgdb = dulge_pkgdb_get_pkg(xhp, buf);
	} else {
		pkg_pkgdb = dulge_pkgdb_get_pkg(xhp, pkg);
	}

	if (xhp->flags & DULGE_FLAG_DOWNLOAD_ONLY) {
		pkg_pkgdb = NULL;
		ttype = DULGE_TRANS_DOWNLOAD;
	}

	/*
	 * Find out if the pkg has been found in repository pool.
	 */
	if (pkg_pkgdb == NULL) {
		/* pkg not installed, perform installation */
		ttype = DULGE_TRANS_INSTALL;
		if (((pkg_repod = dulge_rpool_get_pkg(xhp, pkg)) == NULL) &&
		    ((pkg_repod = dulge_rpool_get_virtualpkg(xhp, pkg)) == NULL)) {
			/* not found */
			return ENOENT;
		}
	} else {
		if (force) {
			ttype = DULGE_TRANS_REINSTALL;
		} else {
			ttype = DULGE_TRANS_UPDATE;
		}
		if (dulge_dictionary_get(pkg_pkgdb, "repolock")) {
			struct dulge_repo *repo;
			/* find update from repo */
			dulge_dictionary_get_cstring_nocopy(pkg_pkgdb, "repository", &repoloc);
			assert(repoloc);
			if ((repo = dulge_regget_repo(xhp, repoloc)) == NULL) {
				/* not found */
				return ENOENT;
			}
			pkg_repod = dulge_repo_get_pkg(repo, pkg);
		} else {
			/* find update from rpool */
			pkg_repod = dulge_rpool_get_pkg(xhp, pkg);
		}
		if (pkg_repod == NULL) {
			/* not found */
			return ENOENT;
		}
	}

	dulge_dictionary_get_cstring_nocopy(pkg_repod, "pkgver", &repopkgver);

	if (ttype == DULGE_TRANS_UPDATE) {
		/*
		 * Compare installed version vs best pkg available in repos
		 * for pkg updates.
		 */
		dulge_dictionary_get_cstring_nocopy(pkg_pkgdb,
		    "pkgver", &instpkgver);
		if (dulge_cmpver(repopkgver, instpkgver) <= 0 &&
		    !dulge_pkg_reverts(pkg_repod, instpkgver)) {
			dulge_dictionary_get_cstring_nocopy(pkg_repod,
			    "repository", &repoloc);
			dulge_dbg_printf("[rpool] Skipping `%s' "
			    "(installed: %s) from repository `%s'\n",
			    repopkgver, instpkgver, repoloc);
			return EEXIST;
		}
	} else if (ttype == DULGE_TRANS_REINSTALL) {
		/*
		 * For reinstallation check if installed version is less than
		 * or equal to the pkg in repos, if true, continue with reinstallation;
		 * otherwise perform an update.
		 */
		dulge_dictionary_get_cstring_nocopy(pkg_pkgdb, "pkgver", &instpkgver);
		if (dulge_cmpver(repopkgver, instpkgver) == 1) {
			ttype = DULGE_TRANS_UPDATE;
		}
	}

	if (pkg_pkgdb) {
		/*
		 * If pkg is already installed, respect some properties.
		 */
		if ((obj = dulge_dictionary_get(pkg_pkgdb, "automatic-install")))
			dulge_dictionary_set(pkg_repod, "automatic-install", obj);
		if ((obj = dulge_dictionary_get(pkg_pkgdb, "hold")))
			dulge_dictionary_set(pkg_repod, "hold", obj);
		if ((obj = dulge_dictionary_get(pkg_pkgdb, "repolock")))
			dulge_dictionary_set(pkg_repod, "repolock", obj);
	}
	/*
	 * Prepare transaction dictionary.
	 */
	if ((rv = dulge_transaction_init(xhp)) != 0)
		return rv;

	pkgs = dulge_dictionary_get(xhp->transd, "packages");
	/*
	 * Find out if package being updated matches the one already
	 * in transaction, in that case ignore it.
	 */
	if (ttype == DULGE_TRANS_UPDATE) {
		if (dulge_find_pkg_in_array(pkgs, repopkgver, 0)) {
			dulge_dbg_printf("[update] `%s' already queued in "
			    "transaction.\n", repopkgver);
			return EEXIST;
		}
	}

	if (!dulge_dictionary_get_cstring_nocopy(pkg_repod, "pkgname", &pkgname)) {
		return EINVAL;
	}
	/*
	 * Set package state in dictionary with same state than the
	 * package currently uses, otherwise not-installed.
	 */
	if ((rv = dulge_pkg_state_installed(xhp, pkgname, &state)) != 0) {
		if (rv != ENOENT) {
			return rv;
		}
		/* Package not installed, don't error out */
		state = DULGE_PKG_STATE_NOT_INSTALLED;
	}
	if ((rv = dulge_set_pkg_state_dictionary(pkg_repod, state)) != 0) {
		return rv;
	}

	if (state == DULGE_PKG_STATE_NOT_INSTALLED)
		ttype = DULGE_TRANS_INSTALL;

	if (!force && dulge_dictionary_get(pkg_repod, "hold"))
		ttype = DULGE_TRANS_HOLD;

	/*
	 * Store pkgd from repo into the transaction.
	 */
	if (!dulge_transaction_pkg_type_set(pkg_repod, ttype)) {
		return EINVAL;
	}

	/*
	 * Set automatic-install to true if it was requested and this is a new install.
	 */
	if (ttype == DULGE_TRANS_INSTALL)
		autoinst = xhp->flags & DULGE_FLAG_INSTALL_AUTO;

	if (!dulge_transaction_store(xhp, pkgs, pkg_repod, autoinst)) {
		return EINVAL;
	}

	return 0;
}

/*
 * Returns 1 if there's an update, 0 if none or -1 on error.
 */
static int
dulge_autoupdate(struct dulge_handle *xhp)
{
	dulge_array_t rdeps;
	dulge_dictionary_t pkgd;
	const char *pkgver = NULL, *pkgname = NULL;
	int rv;

	/*
	 * Check if there's a new update for dulge before starting
	 * another transaction.
	 */
	if (((pkgd = dulge_pkgdb_get_pkg(xhp, "dulge")) == NULL) &&
	    ((pkgd = dulge_pkgdb_get_virtualpkg(xhp, "dulge")) == NULL))
		return 0;

	if (!dulge_dictionary_get_cstring_nocopy(pkgd, "pkgver", &pkgver)) {
		return EINVAL;
	}
	if (!dulge_dictionary_get_cstring_nocopy(pkgd, "pkgname", &pkgname)) {
		return EINVAL;
	}

	rv = trans_find_pkg(xhp, pkgname, false);

	dulge_dbg_printf("%s: trans_find_pkg dulge: %d\n", __func__, rv);

	if (rv == 0) {
		if (xhp->flags & DULGE_FLAG_DOWNLOAD_ONLY) {
			return 0;
		}
		/* a new dulge version is available, check its revdeps */
		rdeps = dulge_pkgdb_get_pkg_revdeps(xhp, "dulge");
		for (unsigned int i = 0; i < dulge_array_count(rdeps); i++)  {
			const char *curpkgver = NULL;
			char curpkgn[DULGE_NAME_SIZE] = {0};

			dulge_array_get_cstring_nocopy(rdeps, i, &curpkgver);
			dulge_dbg_printf("%s: processing revdep %s\n", __func__, curpkgver);

			if (!dulge_pkg_name(curpkgn, sizeof(curpkgn), curpkgver)) {
				abort();
			}
			rv = trans_find_pkg(xhp, curpkgn, false);
			dulge_dbg_printf("%s: trans_find_pkg revdep %s: %d\n", __func__, curpkgver, rv);
			if (rv && rv != ENOENT && rv != EEXIST && rv != ENODEV)
				return -1;
		}
		/*
		 * Set dulge_FLAG_FORCE_REMOVE_REVDEPS to ignore broken
		 * reverse dependencies in dulge_transaction_prepare().
		 *
		 * This won't skip revdeps of the dulge pkg, rather other
		 * packages in rootdir that could be broken indirectly.
		 *
		 * A sysup transaction after updating dulge should fix them
		 * again.
		 */
		xhp->flags |= DULGE_FLAG_FORCE_REMOVE_REVDEPS;
		return 1;
	} else if (rv == ENOENT || rv == EEXIST || rv == ENODEV) {
		/* no update */
		return 0;
	} else {
		/* error */
		return -1;
	}

	return 0;
}

int
dulge_transaction_update_packages(struct dulge_handle *xhp)
{
	dulge_object_t obj;
	dulge_object_iterator_t iter;
	dulge_dictionary_t pkgd;
	bool newpkg_found = false;
	int rv = 0;

	rv = dulge_autoupdate(xhp);
	switch (rv) {
	case 1:
		/* dulge needs to be updated, don't allow any other update */
		return EBUSY;
	case -1:
		/* error */
		return EINVAL;
	default:
		break;
	}

	iter = dulge_dictionary_iterator(xhp->pkgdb);
	assert(iter);

	while ((obj = dulge_object_iterator_next(iter))) {
		const char *pkgver = NULL;
		char pkgname[DULGE_NAME_SIZE] = {0};

		pkgd = dulge_dictionary_get_keysym(xhp->pkgdb, obj);
		if (!dulge_dictionary_get_cstring_nocopy(pkgd, "pkgver", &pkgver)) {
			continue;
		}
		if (!dulge_pkg_name(pkgname, sizeof(pkgname), pkgver)) {
			rv = EINVAL;
			break;
		}
		rv = trans_find_pkg(xhp, pkgname, false);
		dulge_dbg_printf("%s: trans_find_pkg %s: %d\n", __func__, pkgver, rv);
		if (rv == 0) {
			newpkg_found = true;
		} else if (rv == ENOENT || rv == EEXIST || rv == ENODEV) {
			/*
			 * missing pkg or installed version is greater than or
			 * equal than pkg in repositories.
			 */
			rv = 0;
		}
	}
	dulge_object_iterator_release(iter);

	return newpkg_found ? rv : EEXIST;
}

int
dulge_transaction_update_pkg(struct dulge_handle *xhp, const char *pkg, bool force)
{
	dulge_array_t rdeps;
	int rv;

	rv = dulge_autoupdate(xhp);
	dulge_dbg_printf("%s: dulge_autoupdate %d\n", __func__, rv);
	switch (rv) {
	case 1:
		/* dulge needs to be updated, only allow dulge to be updated */
		if (strcmp(pkg, "dulge"))
			return EBUSY;
		return 0;
	case -1:
		/* error */
		return EINVAL;
	default:
		/* no update */
		break;
	}

	/* update its reverse dependencies */
	rdeps = dulge_pkgdb_get_pkg_revdeps(xhp, pkg);
	if (xhp->flags & DULGE_FLAG_DOWNLOAD_ONLY) {
		rdeps = NULL;
	}
	for (unsigned int i = 0; i < dulge_array_count(rdeps); i++)  {
		const char *pkgver = NULL;
		char pkgname[DULGE_NAME_SIZE] = {0};

		if (!dulge_array_get_cstring_nocopy(rdeps, i, &pkgver)) {
			rv = EINVAL;
			break;
		}
		if (!dulge_pkg_name(pkgname, sizeof(pkgname), pkgver)) {
			rv = EINVAL;
			break;
		}
		rv = trans_find_pkg(xhp, pkgname, false);
		dulge_dbg_printf("%s: trans_find_pkg %s: %d\n", __func__, pkgver, rv);
		if (rv && rv != ENOENT && rv != EEXIST && rv != ENODEV) {
			return rv;
		}
	}
	/* add pkg repod */
	rv = trans_find_pkg(xhp, pkg, force);
	dulge_dbg_printf("%s: trans_find_pkg %s: %d\n", __func__, pkg, rv);
	return rv;
}

int
dulge_transaction_install_pkg(struct dulge_handle *xhp, const char *pkg, bool force)
{
	dulge_array_t rdeps;
	int rv;

	rv = dulge_autoupdate(xhp);
	switch (rv) {
	case 1:
		/* dulge needs to be updated, only allow dulge to be updated */
		if (strcmp(pkg, "dulge"))
			return EBUSY;
		return 0;
	case -1:
		/* error */
		return EINVAL;
	default:
		/* no update */
		break;
	}

	/* update its reverse dependencies */
	rdeps = dulge_pkgdb_get_pkg_revdeps(xhp, pkg);
	if (xhp->flags & DULGE_FLAG_DOWNLOAD_ONLY) {
		rdeps = NULL;
	}
	for (unsigned int i = 0; i < dulge_array_count(rdeps); i++)  {
		const char *pkgver = NULL;
		char pkgname[DULGE_NAME_SIZE] = {0};

		if (!dulge_array_get_cstring_nocopy(rdeps, i, &pkgver)) {
			rv = EINVAL;
			break;
		}
		if (!dulge_pkg_name(pkgname, sizeof(pkgname), pkgver)) {
			rv = EINVAL;
			break;
		}
		rv = trans_find_pkg(xhp, pkgname, false);
		dulge_dbg_printf("%s: trans_find_pkg %s: %d\n", __func__, pkgver, rv);
		if (rv && rv != ENOENT && rv != EEXIST && rv != ENODEV) {
			return rv;
		}
	}
	rv = trans_find_pkg(xhp, pkg, force);
	dulge_dbg_printf("%s: trans_find_pkg %s: %d\n", __func__, pkg, rv);
	return rv;
}

int
dulge_transaction_remove_pkg(struct dulge_handle *xhp,
			    const char *pkgname,
			    bool recursive)
{
	dulge_dictionary_t pkgd;
	dulge_array_t pkgs, orphans, orphans_pkg;
	dulge_object_t obj;
	int rv = 0;

	assert(xhp);
	assert(pkgname);

	if ((pkgd = dulge_pkgdb_get_pkg(xhp, pkgname)) == NULL) {
		/* pkg not installed */
		return ENOENT;
	}
	/*
	 * Prepare transaction dictionary and missing deps array.
	 */
	if ((rv = dulge_transaction_init(xhp)) != 0)
		return rv;

	pkgs = dulge_dictionary_get(xhp->transd, "packages");

	if (!recursive)
		goto rmpkg;
	/*
	 * If recursive is set, find out which packages would be orphans
	 * if the supplied package were already removed.
	 */
	if ((orphans_pkg = dulge_array_create()) == NULL)
		return ENOMEM;

	dulge_array_set_cstring_nocopy(orphans_pkg, 0, pkgname);
	orphans = dulge_find_pkg_orphans(xhp, orphans_pkg);
	dulge_object_release(orphans_pkg);
	if (dulge_object_type(orphans) != DULGE_TYPE_ARRAY)
		return EINVAL;

	for (unsigned int i = 0; i < dulge_array_count(orphans); i++) {
		obj = dulge_array_get(orphans, i);
		dulge_transaction_pkg_type_set(obj, DULGE_TRANS_REMOVE);
		if (!dulge_transaction_store(xhp, pkgs, obj, false)) {
			return EINVAL;
		}
	}
	dulge_object_release(orphans);
	return rv;

rmpkg:
	/*
	 * Add pkg dictionary into the transaction pkgs queue.
	 */
	dulge_transaction_pkg_type_set(pkgd, DULGE_TRANS_REMOVE);
	if (!dulge_transaction_store(xhp, pkgs, pkgd, false)) {
		return EINVAL;
	}
	return rv;
}

int
dulge_transaction_autoremove_pkgs(struct dulge_handle *xhp)
{
	dulge_array_t orphans, pkgs;
	dulge_object_t obj;
	int rv = 0;

	orphans = dulge_find_pkg_orphans(xhp, NULL);
	if (dulge_array_count(orphans) == 0) {
		/* no orphans? we are done */
		goto out;
	}
	/*
	 * Prepare transaction dictionary and missing deps array.
	 */
	if ((rv = dulge_transaction_init(xhp)) != 0)
		goto out;

	pkgs = dulge_dictionary_get(xhp->transd, "packages");
	/*
	 * Add pkg orphan dictionary into the transaction pkgs queue.
	 */
	for (unsigned int i = 0; i < dulge_array_count(orphans); i++) {
		obj = dulge_array_get(orphans, i);
		dulge_transaction_pkg_type_set(obj, DULGE_TRANS_REMOVE);
		if (!dulge_transaction_store(xhp, pkgs, obj, false)) {
			rv = EINVAL;
			goto out;
		}
	}
out:
	if (orphans)
		dulge_object_release(orphans);

	return rv;
}

dulge_trans_type_t
dulge_transaction_pkg_type(dulge_dictionary_t pkg_repod)
{
	uint8_t r;

	if (dulge_object_type(pkg_repod) != DULGE_TYPE_DICTIONARY)
		return 0;

	if (!dulge_dictionary_get_uint8(pkg_repod, "transaction", &r))
		return 0;

	return r;
}

bool
dulge_transaction_pkg_type_set(dulge_dictionary_t pkg_repod, dulge_trans_type_t ttype)
{
	uint8_t r;

	if (dulge_object_type(pkg_repod) != DULGE_TYPE_DICTIONARY)
		return false;

	switch (ttype) {
	case DULGE_TRANS_INSTALL:
	case DULGE_TRANS_UPDATE:
	case DULGE_TRANS_CONFIGURE:
	case DULGE_TRANS_REMOVE:
	case DULGE_TRANS_REINSTALL:
	case DULGE_TRANS_HOLD:
	case DULGE_TRANS_DOWNLOAD:
		break;
	default:
		return false;
	}
	r = ttype;
	if (!dulge_dictionary_set_uint8(pkg_repod, "transaction", r))
		return false;

	return true;
}
