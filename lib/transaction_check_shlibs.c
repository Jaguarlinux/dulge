/*-
 * Copyright (c) 2014-2020 Juan Romero Pardines.
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
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include "dulge_api_impl.h"

/*
 * Verify shlib-{provides,requires} for packages in transaction.
 * This will catch cases where a package update would break its reverse
 * dependencies due to an incompatible SONAME bump:
 *
 * 	- foo-1.0 is installed and provides the 'libfoo.so.0' soname.
 * 	- foo-2.0 provides the 'libfoo.so.1' soname.
 * 	- baz-1.0 requires 'libfoo.so.0'.
 * 	- foo is updated to 2.0, hence baz-1.0 is now broken.
 *
 * Abort transaction if such case is found.
 */

static void
shlib_register(dulge_dictionary_t d, const char *shlib, const char *pkgver)
{
	dulge_array_t array;
	bool alloc = false;

	if ((array = dulge_dictionary_get(d, shlib)) == NULL) {
		alloc = true;
		array = dulge_array_create();
		dulge_dictionary_set(d, shlib, array);
	}
	if (!dulge_match_string_in_array(array, pkgver))
		dulge_array_add_cstring_nocopy(array, pkgver);
	if (alloc)
		dulge_object_release(array);
}

static dulge_dictionary_t
collect_shlibs(struct dulge_handle *xhp, dulge_array_t pkgs, bool req)
{
	dulge_object_t obj;
	dulge_object_iterator_t iter;
	dulge_dictionary_t d, pd;
	const char *pkgname, *pkgver;

	d = dulge_dictionary_create();
	assert(d);

	/* copy pkgdb to out temporary dictionary */
	pd = dulge_dictionary_copy(xhp->pkgdb);
	assert(pd);

	/*
	 * copy pkgs from transaction to our dictionary, overriding them
	 * if they were there from pkgdb.
	 */
	iter = dulge_array_iterator(pkgs);
	assert(iter);
	while ((obj = dulge_object_iterator_next(iter))) {
		if (!dulge_dictionary_get_cstring_nocopy(obj, "pkgname", &pkgname))
			continue;

		/* ignore shlibs if pkg is on hold mode */
		if (dulge_transaction_pkg_type(obj) == DULGE_TRANS_HOLD) {
			continue;
		}

		dulge_dictionary_set(pd, pkgname, obj);
	}
	dulge_object_iterator_release(iter);

	/*
	 * iterate over our dictionary to collect shlib-{requires,provides}.
	 */
	iter = dulge_dictionary_iterator(pd);
	assert(iter);

	while ((obj = dulge_object_iterator_next(iter))) {
		dulge_array_t shobjs;
		dulge_dictionary_t pkgd;

		pkgd = dulge_dictionary_get_keysym(pd, obj);
		if (dulge_transaction_pkg_type(pkgd) == DULGE_TRANS_REMOVE) {
			continue;
		}
		/*
		 * If pkg does not have the required obj, pass to next one.
		 */
		dulge_dictionary_get_cstring_nocopy(pkgd, "pkgver", &pkgver);
		shobjs = dulge_dictionary_get(pkgd,
				req ? "shlib-requires" : "shlib-provides");
		if (shobjs == NULL)
			continue;

		for (unsigned int i = 0; i < dulge_array_count(shobjs); i++) {
			const char *shlib = NULL;

			dulge_array_get_cstring_nocopy(shobjs, i, &shlib);
			dulge_dbg_printf("%s: registering %s for %s\n",
			    pkgver, shlib, req ? "shlib-requires" : "shlib-provides");
			if (req)
				shlib_register(d, shlib, pkgver);
			else
				dulge_dictionary_set_cstring_nocopy(d, shlib, pkgver);
		}
	}
	dulge_object_iterator_release(iter);
	dulge_object_release(pd);
	return d;
}

bool HIDDEN
dulge_transaction_check_shlibs(struct dulge_handle *xhp, dulge_array_t pkgs)
{
	dulge_array_t array, mshlibs;
	dulge_object_t obj, obj2;
	dulge_object_iterator_t iter;
	dulge_dictionary_t shrequires, shprovides;
	const char *pkgver = NULL, *shlib = NULL;
	char *buf;
	bool broken = false;

	shrequires = collect_shlibs(xhp, pkgs, true);
	shprovides = collect_shlibs(xhp, pkgs, false);

	mshlibs = dulge_dictionary_get(xhp->transd, "missing_shlibs");
	/* iterate over shlib-requires to find unmatched shlibs */
	iter = dulge_dictionary_iterator(shrequires);
	assert(iter);

	while ((obj = dulge_object_iterator_next(iter))) {
		shlib = dulge_dictionary_keysym_cstring_nocopy(obj);
		dulge_dbg_printf("%s: checking for `%s': ", __func__, shlib);
		if ((obj2 = dulge_dictionary_get(shprovides, shlib))) {
			dulge_dbg_printf_append("provided by `%s'\n",
			    dulge_string_cstring_nocopy(obj2));
			continue;
		}
		dulge_dbg_printf_append("not found\n");

		broken = true;
		array = dulge_dictionary_get_keysym(shrequires, obj);
		for (unsigned int i = 0; i < dulge_array_count(array); i++) {
			dulge_array_get_cstring_nocopy(array, i, &pkgver);
			buf = dulge_xasprintf("%s: broken, unresolvable "
			    "shlib `%s'", pkgver, shlib);
			dulge_array_add_cstring(mshlibs, buf);
			free(buf);
		}
	}
	dulge_object_iterator_release(iter);
	if (!broken) {
		dulge_dictionary_remove(xhp->transd, "missing_shlibs");
	}
	dulge_object_release(shprovides);
	dulge_object_release(shrequires);

	return true;
}
