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
 *-
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <dulge.h>
#include "defs.h"

int
show_pkg_deps(struct dulge_handle *xhp, const char *pkgname, bool repomode, bool full)
{
	dulge_array_t rdeps;
	dulge_dictionary_t pkgd;

	if (repomode) {
		if (((pkgd = dulge_rpool_get_pkg(xhp, pkgname)) == NULL) &&
		    ((pkgd = dulge_rpool_get_virtualpkg(xhp, pkgname)) == NULL))
			return errno;
	} else {
		if ((pkgd = dulge_pkgdb_get_pkg(xhp, pkgname)) == NULL)
			return errno;
	}
	if (full) {
		if (repomode)
			rdeps = dulge_rpool_get_pkg_fulldeptree(xhp, pkgname);
		else
			rdeps = dulge_pkgdb_get_pkg_fulldeptree(xhp, pkgname);

		if (rdeps == NULL)
			return errno;
	} else {
		rdeps = dulge_dictionary_get(pkgd, "run_depends");
	}
	for (unsigned int i = 0; i < dulge_array_count(rdeps); i++) {
		const char *pkgdep = NULL;
		dulge_array_get_cstring_nocopy(rdeps, i, &pkgdep);
		puts(pkgdep);
	}
	return 0;
}

int
show_pkg_revdeps(struct dulge_handle *xhp, const char *pkg, bool repomode)
{
	dulge_array_t revdeps;
	const char *pkgdep = NULL;

	if (repomode)
		revdeps = dulge_rpool_get_pkg_revdeps(xhp, pkg);
	else
		revdeps = dulge_pkgdb_get_pkg_revdeps(xhp, pkg);

	if (revdeps == NULL)
		return errno;

	for (unsigned int i = 0; i < dulge_array_count(revdeps); i++) {
		dulge_array_get_cstring_nocopy(revdeps, i, &pkgdep);
		puts(pkgdep);
	}
	dulge_object_release(revdeps);
	return 0;
}
