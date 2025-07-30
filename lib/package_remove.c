/*-
 * Copyright (c) 2009-2015 Juan Romero Pardines.
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

#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dulge_api_impl.h"

static bool
check_remove_pkg_files(struct dulge_handle *xhp,
	dulge_array_t obsoletes, const char *pkgver, uid_t euid)
{
	struct stat st;
	bool fail = false;

	if (euid == 0)
		return false;

	for (unsigned int i = 0; i < dulge_array_count(obsoletes); i++) {
		const char *file = NULL;
		dulge_array_get_cstring_nocopy(obsoletes, i, &file);
		/*
		 * Check if effective user ID owns the file; this is
		 * enough to ensure the user has write permissions
		 * on the directory.
		 */
		if (lstat(file, &st) == 0 && euid == st.st_uid) {
			/* success */
			continue;
		}
		if (errno != ENOENT) {
			/*
			 * only bail out if something else than ENOENT
			 * is returned.
			 */
			int rv = errno;
			if (rv == 0) {
				/* lstat succeeds but euid != uid */
				rv = EPERM;
			}
			fail = true;
			dulge_set_cb_state(xhp, DULGE_STATE_REMOVE_FILE_FAIL,
				rv, pkgver,
				"%s: cannot remove `%s': %s",
				pkgver, file, strerror(rv));
		}
		errno = 0;
	}
	return fail;
}

static int
remove_pkg_files(struct dulge_handle *xhp,
		 dulge_array_t obsoletes,
		 const char *pkgver)
{
	int rv = 0;

	for (unsigned int i = 0; i < dulge_array_count(obsoletes); i++) {
		const char *file = NULL;
		dulge_array_get_cstring_nocopy(obsoletes, i, &file);
		/*
		 * Remove the object if possible.
		 */
		if (remove(file) == -1) {
			dulge_set_cb_state(xhp, DULGE_STATE_REMOVE_FILE_FAIL,
			    errno, pkgver,
			    "%s: failed to remove `%s': %s", pkgver,
			    file, strerror(errno));
		} else {
			/* success */
			dulge_set_cb_state(xhp, DULGE_STATE_REMOVE_FILE,
			    0, pkgver, "Removed `%s'", file);
		}
	}

	return rv;
}

int HIDDEN
dulge_remove_pkg(struct dulge_handle *xhp, const char *pkgver, bool update)
{
	dulge_dictionary_t pkgd = NULL, obsd = NULL;
	dulge_array_t obsoletes = NULL;
	char pkgname[DULGE_NAME_SIZE], metafile[PATH_MAX];
	int rv = 0;
	pkg_state_t state = 0;
	uid_t euid;

	assert(xhp);
	assert(pkgver);

	if (!dulge_pkg_name(pkgname, sizeof(pkgname), pkgver)) {
		abort();
	}

	euid = geteuid();

	if ((pkgd = dulge_pkgdb_get_pkg(xhp, pkgname)) == NULL) {
		rv = errno;
		dulge_dbg_printf("[remove] cannot find %s in pkgdb: %s\n",
		    pkgver, strerror(rv));
		goto out;
	}
	if ((rv = dulge_pkg_state_dictionary(pkgd, &state)) != 0) {
		dulge_dbg_printf("[remove] cannot find %s in pkgdb: %s\n",
		    pkgver, strerror(rv));
		goto out;
	}
	dulge_dbg_printf("attempting to remove %s state %d\n", pkgver, state);

	if (!update)
		dulge_set_cb_state(xhp, DULGE_STATE_REMOVE, 0, pkgver, NULL);

	if (chdir(xhp->rootdir) == -1) {
		rv = errno;
		dulge_set_cb_state(xhp, DULGE_STATE_REMOVE_FAIL,
		    rv, pkgver,
		   "%s: [remove] failed to chdir to rootdir `%s': %s",
		    pkgver, xhp->rootdir, strerror(rv));
		goto out;
	}

	/* If package was "half-removed", remove it fully. */
	if (state == DULGE_PKG_STATE_HALF_REMOVED)
		goto purge;

	/* unregister alternatives */
	if (update)
		dulge_dictionary_set_bool(pkgd, "alternatives-update", true);

	if ((rv = dulge_alternatives_unregister(xhp, pkgd)) != 0)
		goto out;

	/*
	 * If updating a package, we just need to execute the current
	 * pre-remove action target and we are done. Its files will be
	 * overwritten later in unpack phase.
	 */
	if (update) {
		return 0;
	}

	if (dulge_dictionary_get_dict(xhp->transd, "obsolete_files", &obsd))
		obsoletes = dulge_dictionary_get(obsd, pkgname);

	if (dulge_array_count(obsoletes) > 0) {
		/*
		 * Do the removal in 2 phases:
		 * 	1- check if user has enough perms to remove all entries
		 * 	2- perform removal
		 */
		if (check_remove_pkg_files(xhp, obsoletes, pkgver, euid)) {
			rv = EPERM;
			goto out;
		}
		/* Remove links */
		if ((rv = remove_pkg_files(xhp, obsoletes, pkgver)) != 0)
			goto out;
	}

	/*
	 * Set package state to "half-removed".
	 */
	rv = dulge_set_pkg_state_dictionary(pkgd,
	     DULGE_PKG_STATE_HALF_REMOVED);
	if (rv != 0) {
		dulge_set_cb_state(xhp, DULGE_STATE_REMOVE_FAIL,
		    rv, pkgver,
		    "%s: [remove] failed to set state to half-removed: %s",
		    pkgver, strerror(rv));
		goto out;
	}
	/* XXX: setting the state and then removing the package seems useless. */

purge:
	/*
	 * Remove package metadata plist.
	 */
	snprintf(metafile, sizeof(metafile), "%s/.%s-files.plist", xhp->metadir, pkgname);
	if (remove(metafile) == -1) {
		if (errno != ENOENT) {
			dulge_set_cb_state(xhp, DULGE_STATE_REMOVE_FAIL,
			    rv, pkgver,
			    "%s: failed to remove metadata file: %s",
			    pkgver, strerror(errno));
		}
	}
	/*
	 * Unregister package from pkgdb.
	 */
	dulge_dbg_printf("[remove] unregister %s returned %d\n", pkgver, rv);
	dulge_set_cb_state(xhp, DULGE_STATE_REMOVE_DONE, 0, pkgver, NULL);
	dulge_dictionary_remove(xhp->pkgdb, pkgname);
out:
	if (rv != 0) {
		dulge_set_cb_state(xhp, DULGE_STATE_REMOVE_FAIL, rv, pkgver,
		    "%s: failed to remove package: %s", pkgver, strerror(rv));
	}

	return rv;
}
