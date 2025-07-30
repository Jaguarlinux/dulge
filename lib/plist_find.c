/*-
 * Copyright (c) 2008-2016 Juan Romero Pardines.
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
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>

#include "dulge_api_impl.h"

static dulge_dictionary_t
get_pkg_in_array(dulge_array_t array, const char *str, dulge_trans_type_t tt, bool virtual)
{
	dulge_object_t obj;
	dulge_object_iterator_t iter;
	dulge_trans_type_t ttype;
	bool found = false;

	assert(array);
	assert(str);

	iter = dulge_array_iterator(array);
	if (!iter)
		return NULL;

	while ((obj = dulge_object_iterator_next(iter))) {
		const char *pkgver = NULL;
		char pkgname[DULGE_NAME_SIZE] = {0};

		if (!dulge_dictionary_get_cstring_nocopy(obj, "pkgver", &pkgver)) {
			continue;
		}
		if (virtual) {
			/*
			 * Check if package pattern matches
			 * any virtual package version in dictionary.
			 */
			found = dulge_match_virtual_pkg_in_dict(obj, str);
			if (found)
				break;
		} else if (dulge_pkgpattern_version(str)) {
			/* match by pattern against pkgver */
			if (dulge_pkgpattern_match(pkgver, str)) {
				found = true;
				break;
			}
		} else if (dulge_pkg_version(str)) {
			/* match by exact pkgver */
			if (strcmp(str, pkgver) == 0) {
				found = true;
				break;
			}
		} else {
			if (!dulge_pkg_name(pkgname, sizeof(pkgname), pkgver)) {
				abort();
			}
			/* match by pkgname */
			if (strcmp(pkgname, str) == 0) {
				found = true;
				break;
			}
		}
	}
	dulge_object_iterator_release(iter);

	ttype = dulge_transaction_pkg_type(obj);
	if (found && tt && (ttype != tt)) {
		found = false;
	}
	if (!found) {
		errno = ENOENT;
		return NULL;
	}
	return obj;
}

dulge_dictionary_t HIDDEN
dulge_find_pkg_in_array(dulge_array_t a, const char *s, dulge_trans_type_t tt)
{
	assert(dulge_object_type(a) == DULGE_TYPE_ARRAY);
	assert(s);

	return get_pkg_in_array(a, s, tt, false);
}

dulge_dictionary_t HIDDEN
dulge_find_virtualpkg_in_array(struct dulge_handle *xhp,
			      dulge_array_t a,
			      const char *s,
			      dulge_trans_type_t tt)
{
	dulge_dictionary_t pkgd;
	const char *vpkg;

	assert(xhp);
	assert(dulge_object_type(a) == DULGE_TYPE_ARRAY);
	assert(s);

	if ((vpkg = vpkg_user_conf(xhp, s))) {
		if ((pkgd = get_pkg_in_array(a, vpkg, tt, true)))
			return pkgd;
	}

	return get_pkg_in_array(a, s, tt, true);
}

static dulge_dictionary_t
match_pkg_by_pkgver(dulge_dictionary_t repod, const char *p)
{
	dulge_dictionary_t d = NULL;
	const char *pkgver = NULL;
	char pkgname[DULGE_NAME_SIZE] = {0};

	assert(repod);
	assert(p);

	/* exact match by pkgver */
	if (!dulge_pkg_name(pkgname, sizeof(pkgname), p)) {
		dulge_error_printf("invalid pkgver: %s\n", p);
		errno = EINVAL;
		return NULL;
	}

	d = dulge_dictionary_get(repod, pkgname);
	if (!d) {
		errno = ENOENT;
		return NULL;
	}
	if (!dulge_dictionary_get_cstring_nocopy(d, "pkgver", &pkgver)) {
		dulge_error_printf("missing `pkgver` property\n");
		errno = EINVAL;
		return NULL;
	}
	if (strcmp(pkgver, p) != 0) {
		errno = ENOENT;
		return NULL;
	}

	return d;
}

static dulge_dictionary_t
match_pkg_by_pattern(dulge_dictionary_t repod, const char *p)
{
	dulge_dictionary_t d = NULL;
	const char *pkgver = NULL;
	char pkgname[DULGE_NAME_SIZE] = {0};

	assert(repod);
	assert(p);

	/* match by pkgpattern in pkgver */
	if (!dulge_pkgpattern_name(pkgname, sizeof(pkgname), p)) {
		if (dulge_pkg_name(pkgname, sizeof(pkgname), p)) {
			return match_pkg_by_pkgver(repod, p);
		}
		dulge_error_printf("invalid pkgpattern: %s\n", p);
		errno = EINVAL;
		return NULL;
	}

	d = dulge_dictionary_get(repod, pkgname);
	if (!d) {
		errno = ENOENT;
		return NULL;
	}
	if (!dulge_dictionary_get_cstring_nocopy(d, "pkgver", &pkgver)) {
		dulge_error_printf("missing `pkgver` property`\n");
		errno = EINVAL;
		return NULL;
	}
	if (!dulge_pkgpattern_match(pkgver, p)) {
		errno = ENOENT;
		return NULL;
	}

	return d;
}

const char HIDDEN *
vpkg_user_conf(struct dulge_handle *xhp, const char *vpkg)
{
	char namebuf[DULGE_NAME_SIZE];
	dulge_dictionary_t providers;
	dulge_object_t obj;
	dulge_object_iterator_t iter;
	const char *pkg = NULL;
	const char *pkgname;
	bool found = false;
	enum { PKGPATTERN, PKGVER, PKGNAME } match;

	assert(vpkg);


	if (dulge_pkgpattern_name(namebuf, sizeof(namebuf), vpkg)) {
		match = PKGPATTERN;
		pkgname = namebuf;
	} else if (dulge_pkg_name(namebuf, sizeof(namebuf), vpkg)) {
		match = PKGVER;
		pkgname = namebuf;
	} else {
		match = PKGNAME;
		pkgname = vpkg;
	}

	providers = dulge_dictionary_get(xhp->vpkgd, pkgname);
	if (!providers)
		return NULL;

	iter = dulge_dictionary_iterator(providers);
	assert(iter);

	while ((obj = dulge_object_iterator_next(iter))) {
		dulge_string_t rpkg;
		char buf[DULGE_NAME_SIZE] = {0};
		const char *vpkg_conf = NULL, *vpkgname = NULL;

		vpkg_conf = dulge_dictionary_keysym_cstring_nocopy(obj);
		rpkg = dulge_dictionary_get_keysym(providers, obj);
		pkg = dulge_string_cstring_nocopy(rpkg);

		if (dulge_pkg_version(vpkg_conf)) {
			if (!dulge_pkg_name(buf, sizeof(buf), vpkg_conf)) {
				abort();
			}
			vpkgname = buf;
		} else {
			vpkgname = vpkg_conf;
		}

		switch (match) {
		case PKGPATTERN:
			if (dulge_pkg_version(vpkg_conf)) {
				if (!dulge_pkgpattern_match(vpkg_conf, vpkg)) {
					continue;
				}
			} else {
				dulge_warn_printf("invalid: %s\n", vpkg_conf);
			}
		break;
		case PKGVER:
			if (strcmp(buf, vpkgname) != 0) {
				continue;
			}
			break;
		case PKGNAME:
			if (strcmp(vpkg, vpkgname) != 0) {
				continue;
			}
		break;
		}
		dulge_dbg_printf("%s: vpkg_conf %s pkg %s vpkgname %s\n", __func__, vpkg_conf, pkg, vpkgname);
		found = true;
		break;
	}
	dulge_object_iterator_release(iter);

	return found ? pkg : NULL;
}

dulge_dictionary_t HIDDEN
dulge_find_virtualpkg_in_conf(struct dulge_handle *xhp,
			dulge_dictionary_t d,
			const char *pkg)
{
	dulge_object_iterator_t iter;
	dulge_object_t obj;
	dulge_dictionary_t providers;
	dulge_dictionary_t pkgd = NULL;
	const char *cur;

	if (!xhp->vpkgd_conf)
		return NULL;

	providers = dulge_dictionary_get(xhp->vpkgd_conf, pkg);
	if (!providers)
		return NULL;

	iter = dulge_dictionary_iterator(providers);
	assert(iter);

	while ((obj = dulge_object_iterator_next(iter))) {
		dulge_string_t rpkg;
		char buf[DULGE_NAME_SIZE] = {0};
		const char *vpkg_conf = NULL, *vpkgname = NULL;

		vpkg_conf = dulge_dictionary_keysym_cstring_nocopy(obj);
		rpkg = dulge_dictionary_get_keysym(providers, obj);
		cur = dulge_string_cstring_nocopy(rpkg);
		assert(cur);
		if (dulge_pkg_version(vpkg_conf)) {
			if (!dulge_pkg_name(buf, sizeof(buf), vpkg_conf)) {
				abort();
			}
			vpkgname = buf;
		} else {
			vpkgname = vpkg_conf;
		}

		if (dulge_pkgpattern_version(pkg)) {
			if (dulge_pkg_version(vpkg_conf)) {
				if (!dulge_pkgpattern_match(vpkg_conf, pkg)) {
					continue;
				}
			} else {
				char *vpkgver = dulge_xasprintf("%s-999999_1", vpkg_conf);
				if (!dulge_pkgpattern_match(vpkgver, pkg)) {
					free(vpkgver);
					continue;
				}
				free(vpkgver);
			}
		} else if (dulge_pkg_version(pkg)) {
			// XXX: this is the old behaviour of only matching pkgname's,
			// this is kinda wrong when compared to matching patterns
			// where all variants are tried.
			if (!dulge_pkg_name(buf, sizeof(buf), pkg)) {
				abort();
			}
			if (strcmp(buf, vpkgname)) {
				continue;
			}
		} else {
			if (strcmp(pkg, vpkgname)) {
				continue;
			}
		}
		dulge_dbg_printf("%s: found: %s %s %s\n", __func__, vpkg_conf, cur, vpkgname);

		/* Try matching vpkg from configuration files */
		if (dulge_pkgpattern_version(cur))
			pkgd = match_pkg_by_pattern(d, cur);
		else if (dulge_pkg_version(cur))
			pkgd = match_pkg_by_pkgver(d, cur);
		else
			pkgd = dulge_dictionary_get(d, cur);
		break;
	}
	dulge_object_iterator_release(iter);

	return pkgd;
}

dulge_dictionary_t HIDDEN
dulge_find_virtualpkg_in_dict(struct dulge_handle *xhp,
			     dulge_dictionary_t d,
			     const char *pkg)
{
	dulge_object_t obj;
	dulge_object_iterator_t iter;
	dulge_dictionary_t pkgd = NULL;
	const char *vpkg;

	// XXX: this is bad, dict != pkgdb,
	/* Try matching vpkg via xhp->vpkgd */
	vpkg = vpkg_user_conf(xhp, pkg);
	if (vpkg != NULL) {
		if (dulge_pkgpattern_version(vpkg))
			pkgd = match_pkg_by_pattern(d, vpkg);
		else if (dulge_pkg_version(vpkg))
			pkgd = match_pkg_by_pkgver(d, vpkg);
		else
			pkgd = dulge_dictionary_get(d, vpkg);

		if (pkgd)
			return pkgd;
	}
	/* ... otherwise match the first one in dictionary */
	iter = dulge_dictionary_iterator(d);
	assert(iter);

	while ((obj = dulge_object_iterator_next(iter))) {
		pkgd = dulge_dictionary_get_keysym(d, obj);
		if (dulge_match_virtual_pkg_in_dict(pkgd, pkg)) {
			dulge_object_iterator_release(iter);
			return pkgd;
		}
	}
	dulge_object_iterator_release(iter);

	return NULL;
}

dulge_dictionary_t HIDDEN
dulge_find_pkg_in_dict(dulge_dictionary_t d, const char *pkg)
{
	dulge_dictionary_t pkgd = NULL;

	if (dulge_pkgpattern_version(pkg))
		pkgd = match_pkg_by_pattern(d, pkg);
	else if (dulge_pkg_version(pkg))
		pkgd = match_pkg_by_pkgver(d, pkg);
	else
		pkgd = dulge_dictionary_get(d, pkg);

	return pkgd;
}
