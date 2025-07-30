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

#include "dulge_api_impl.h"

/**
 * @file lib/package_orphans.c
 * @brief Package orphans handling routines
 * @defgroup pkg_orphans Package orphans handling functions
 *
 * Functions to find installed package orphans.
 *
 * Package orphans were installed automatically by another package,
 * but currently no other packages are depending on.
 *
 * The following image shown below shows the registered packages database
 * dictionary (the array returned by dulge_find_pkg_orphans() will
 * contain a package dictionary per orphan found):
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

dulge_array_t
dulge_find_pkg_orphans(struct dulge_handle *xhp, dulge_array_t orphans_user)
{
	dulge_array_t array = NULL;
	dulge_object_t obj;
	dulge_object_iterator_t iter;

	if (dulge_pkgdb_init(xhp) != 0)
		return NULL;

	if ((array = dulge_array_create()) == NULL)
		return NULL;

	if (!orphans_user) {
		/* automatic mode (dulge-query -O, dulge-remove -o) */
		iter = dulge_dictionary_iterator(xhp->pkgdb);
		assert(iter);
		/*
		 * Iterate on pkgdb until no more orphans are found.
		 */
		for (;;) {
			bool added = false;
			while ((obj = dulge_object_iterator_next(iter))) {
				dulge_array_t revdeps;
				dulge_dictionary_t pkgd;
				unsigned int cnt = 0, revdepscnt = 0;
				const char *pkgver = NULL;
				bool automatic = false;

				pkgd = dulge_dictionary_get_keysym(xhp->pkgdb, obj);
				if (!dulge_dictionary_get_cstring_nocopy(pkgd, "pkgver", &pkgver)) {
					/* _DULGE_ALTERNATIVES_ */
					continue;
				}
				dulge_dbg_printf(" %s checking %s\n", __func__, pkgver);
				dulge_dictionary_get_bool(pkgd, "automatic-install", &automatic);
				if (!automatic) {
					dulge_dbg_printf(" %s skipped (!automatic)\n", pkgver);
					continue;
				}
				if (dulge_find_pkg_in_array(array, pkgver, 0)) {
					dulge_dbg_printf(" %s orphan (queued)\n", pkgver);
					continue;
				}
				revdeps = dulge_pkgdb_get_pkg_revdeps(xhp, pkgver);
				revdepscnt = dulge_array_count(revdeps);

				if (revdepscnt == 0) {
					added = true;
					dulge_array_add(array, pkgd);
					dulge_dbg_printf(" %s orphan (automatic and !revdeps)\n", pkgver);
					continue;
				}
				/* verify all revdeps are seen */
				for (unsigned int i = 0; i < revdepscnt; i++) {
					const char *revdepver = NULL;

					dulge_array_get_cstring_nocopy(revdeps, i, &revdepver);
					if (dulge_find_pkg_in_array(array, revdepver, 0))
						cnt++;
				}
				if (cnt == revdepscnt) {
					added = true;
					dulge_array_add(array, pkgd);
					dulge_dbg_printf(" %s orphan (automatic and all revdeps)\n", pkgver);
				}

			}
			dulge_dbg_printf("orphans pkgdb iter: added %s\n", added ? "true" : "false");
			dulge_object_iterator_reset(iter);
			if (!added)
				break;
		}
		dulge_object_iterator_release(iter);

		return array;
	}

	/*
	 * Recursive removal mode (dulge-remove -R).
	 */
	for (unsigned int i = 0; i < dulge_array_count(orphans_user); i++) {
		dulge_dictionary_t pkgd;
		const char *pkgver = NULL;

		dulge_array_get_cstring_nocopy(orphans_user, i, &pkgver);
		pkgd = dulge_pkgdb_get_pkg(xhp, pkgver);
		if (pkgd == NULL)
			continue;
		dulge_array_add(array, pkgd);
	}

	for (unsigned int i = 0; i < dulge_array_count(array); i++) {
		dulge_array_t rdeps;
		dulge_dictionary_t pkgd;
		const char *pkgver = NULL;
		unsigned int cnt = 0, reqbycnt = 0;

		pkgd = dulge_array_get(array, i);
		dulge_dictionary_get_cstring_nocopy(pkgd, "pkgver", &pkgver);
		rdeps = dulge_pkgdb_get_pkg_fulldeptree(xhp, pkgver);
		if (dulge_array_count(rdeps) == 0) {
			continue;
		}

		dulge_dbg_printf(" processing rdeps for %s\n", pkgver);
		for (unsigned int x = 0; x < dulge_array_count(rdeps); x++) {
			dulge_array_t reqby;
			dulge_dictionary_t deppkgd;
			const char *deppkgver = NULL;
			bool automatic = false;

			cnt = 0;
			dulge_array_get_cstring_nocopy(rdeps, x, &deppkgver);
			if (dulge_find_pkg_in_array(array, deppkgver, 0)) {
				dulge_dbg_printf(" rdep %s already queued\n", deppkgver);
				continue;
			}
			deppkgd = dulge_pkgdb_get_pkg(xhp, deppkgver);
			dulge_dictionary_get_bool(deppkgd, "automatic-install", &automatic);
			if (!automatic) {
				dulge_dbg_printf(" rdep %s skipped (!automatic)\n", deppkgver);
				continue;
			}

			reqby = dulge_pkgdb_get_pkg_revdeps(xhp, deppkgver);
			reqbycnt = dulge_array_count(reqby);
			for (unsigned int j = 0; j < reqbycnt; j++) {
				const char *reqbydep = NULL;

				dulge_array_get_cstring_nocopy(reqby, j, &reqbydep);
				dulge_dbg_printf(" %s processing revdep %s\n", pkgver, reqbydep);
				if (dulge_find_pkg_in_array(array, reqbydep, 0))
					cnt++;
			}
			if (cnt == reqbycnt) {
				dulge_array_add(array, deppkgd);
				dulge_dbg_printf(" added %s orphan\n", deppkgver);
			}
		}
	}

	return array;
}
