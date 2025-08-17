/*-
 * Copyright (c) 2011-2020 Juan Romero Pardines.
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
#include <unistd.h>
#include <libgen.h>

#include "dulge_api_impl.h"

/*
 * Processes the array of pkg dictionaries in "pkgs" to
 * find matching package replacements via "replaces" pkg obj.
 *
 * This array contains the unordered list of packages in
 * the transaction dictionary.
 */
bool HIDDEN
dulge_transaction_check_replaces(struct dulge_handle *xhp, dulge_array_t pkgs)
{
	assert(xhp);
	assert(pkgs);

	for (unsigned int i = 0; i < dulge_array_count(pkgs); i++) {
		dulge_array_t replaces;
		dulge_object_t obj;
		dulge_object_iterator_t iter;
		dulge_dictionary_t instd, reppkgd;
		const char *pkgver = NULL;
		char pkgname[DULGE_NAME_SIZE] = {0};

		obj = dulge_array_get(pkgs, i);
		replaces = dulge_dictionary_get(obj, "replaces");
		if (replaces == NULL || dulge_array_count(replaces) == 0)
			continue;

		if (!dulge_dictionary_get_cstring_nocopy(obj, "pkgver", &pkgver)) {
			return false;
		}
		if (!dulge_pkg_name(pkgname, DULGE_NAME_SIZE, pkgver)) {
			return false;
		}

		iter = dulge_array_iterator(replaces);
		assert(iter);

		for (unsigned int j = 0; j < dulge_array_count(replaces); j++) {
			const char *curpkgver = NULL, *pattern = NULL;
			char curpkgname[DULGE_NAME_SIZE] = {0};
			bool instd_auto = false, hold = false;
			dulge_trans_type_t ttype;

			if(!dulge_array_get_cstring_nocopy(replaces, j, &pattern))
				abort();

			/*
			 * Find the installed package that matches the pattern
			 * to be replaced.
			 */
			if (((instd = dulge_pkgdb_get_pkg(xhp, pattern)) == NULL) &&
			    ((instd = dulge_pkgdb_get_virtualpkg(xhp, pattern)) == NULL))
				continue;

			if (!dulge_dictionary_get_cstring_nocopy(instd, "pkgver", &curpkgver)) {
				dulge_object_iterator_release(iter);
				return false;
			}
			/* ignore pkgs on hold mode */
			if (dulge_dictionary_get_bool(instd, "hold", &hold) && hold)
				continue;

			if (!dulge_pkg_name(curpkgname, DULGE_NAME_SIZE, curpkgver)) {
				dulge_object_iterator_release(iter);
				return false;
			}
			/*
			 * Check that we are not replacing the same package,
			 * due to virtual packages.
			 */
			if (strcmp(pkgname, curpkgname) == 0) {
				continue;
			}
			/*
			 * Make sure to not add duplicates.
			 */
			dulge_dictionary_get_bool(instd, "automatic-install", &instd_auto);
			reppkgd = dulge_find_pkg_in_array(pkgs, curpkgname, 0);
			if (reppkgd) {
				ttype = dulge_transaction_pkg_type(reppkgd);
				if (ttype == DULGE_TRANS_REMOVE || ttype == DULGE_TRANS_HOLD)
					continue;
				if (!dulge_dictionary_get_cstring_nocopy(reppkgd,
				    "pkgver", &curpkgver)) {
					dulge_object_iterator_release(iter);
					return false;
				}
				if (!dulge_match_virtual_pkg_in_dict(reppkgd, pattern) &&
				    !dulge_pkgpattern_match(curpkgver, pattern))
					continue;
				/*
				 * Package contains replaces="pkgpattern", but the
				 * package that should be replaced is also in the
				 * transaction and it's going to be updated.
				 */
				if (!instd_auto) {
					dulge_dictionary_remove(obj, "automatic-install");
				}
				if (!dulge_dictionary_set_bool(reppkgd, "replaced", true)) {
					dulge_object_iterator_release(iter);
					return false;
				}
				if (!dulge_transaction_pkg_type_set(reppkgd, DULGE_TRANS_REMOVE)) {
					dulge_object_iterator_release(iter);
					return false;
				}
				if (dulge_array_replace_dict_by_name(pkgs, reppkgd, curpkgname) != 0) {
					dulge_object_iterator_release(iter);
					return false;
				}
				dulge_dbg_printf(
				    "Package `%s' in transaction will be "
				    "replaced by `%s', matched with `%s'\n",
				    curpkgver, pkgver, pattern);
				continue;
			}
			/*
			 * If new package is providing a virtual package to the
			 * package that we want to replace we should respect
			 * the automatic-install object.
			 */
			if (dulge_match_virtual_pkg_in_dict(obj, pattern)) {
				if (!instd_auto) {
					dulge_dictionary_remove(obj, "automatic-install");
				}
			}
			/*
			 * Add package dictionary into the transaction and mark
			 * it as to be "removed".
			 */
			if (!dulge_transaction_pkg_type_set(instd, DULGE_TRANS_REMOVE)) {
				dulge_object_iterator_release(iter);
				return false;
			}
			if (!dulge_dictionary_set_bool(instd, "replaced", true)) {
				dulge_object_iterator_release(iter);
				return false;
			}
			if (!dulge_array_add_first(pkgs, instd)) {
				dulge_object_iterator_release(iter);
				return false;
			}
			dulge_dbg_printf(
			    "Package `%s' will be replaced by `%s', "
			    "matched with `%s'\n", curpkgver, pkgver, pattern);
		}
		dulge_object_iterator_release(iter);
	}

	return true;
}
