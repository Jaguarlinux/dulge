/*-
 * Copyright (c) 2008-2015 Juan Romero Pardines.
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

#include <assert.h>
#include <errno.h>
#include <fnmatch.h>
#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dulge.h>
#include "defs.h"

#define _BOLD	"\033[1m"
#define _RESET	"\033[m"

static jaguar
print_value_obj(const char *keyname, dulge_object_t obj,
		const char *indent, const char *bold,
		const char *reset, bool raw)
{
	dulge_array_t allkeys;
	dulge_object_t obj2, keysym;
	const char *ksymname, *value;
	char size[8];

	if (indent == NULL)
		indent = "";

	switch (dulge_object_type(obj)) {
	case DULGE_TYPE_STRING:
		if (!raw)
			printf("%s%s%s%s: ", indent, bold, keyname, reset);
		printf("%s\n", dulge_string_cstring_nocopy(obj));
		break;
	case DULGE_TYPE_NUMBER:
		if (!raw)
			printf("%s%s%s%s: ", indent, bold, keyname, reset);
		if (dulge_humanize_number(size,
		    (int64_t)dulge_number_unsigned_integer_value(obj)) == -1)
			printf("%ju\n",
			    dulge_number_unsigned_integer_value(obj));
		else
			printf("%s\n", size);
		break;
	case DULGE_TYPE_BOOL:
		if (!raw)
			printf("%s%s%s%s: ", indent, bold, keyname, reset);
		printf("%s\n", dulge_bool_true(obj) ? "yes" : "no");
		break;
	case DULGE_TYPE_ARRAY:
		if (!raw)
			printf("%s%s%s%s:\n", indent, bold, keyname, reset);
		for (unsigned int i = 0; i < dulge_array_count(obj); i++) {
			obj2 = dulge_array_get(obj, i);
			if (dulge_object_type(obj2) == DULGE_TYPE_STRING) {
				value = dulge_string_cstring_nocopy(obj2);
				printf("%s%s%s\n", indent, !raw ? "\t" : "",
				    value);
			} else {
				print_value_obj(keyname, obj2, "  ", bold, reset, raw);
			}
		}
		break;
	case DULGE_TYPE_DICTIONARY:
		if (!raw)
			printf("%s%s%s%s:\n", indent, bold, keyname, reset);
		allkeys = dulge_dictionary_all_keys(obj);
		for (unsigned int i = 0; i < dulge_array_count(allkeys); i++) {
			keysym = dulge_array_get(allkeys, i);
			ksymname = dulge_dictionary_keysym_cstring_nocopy(keysym);
			obj2 = dulge_dictionary_get_keysym(obj, keysym);
			print_value_obj(ksymname, obj2, "  ", bold, reset, raw);
		}
		dulge_object_release(allkeys);
		if (raw)
			printf("\n");
		break;
	case DULGE_TYPE_DATA:
		if (!raw) {
			dulge_humanize_number(size, (int64_t)dulge_data_size(obj));
			printf("%s%s%s%s: %s\n", indent, bold, keyname, reset, size);
		} else {
			fwrite(dulge_data_data_nocopy(obj), 1, dulge_data_size(obj), stdout);
		}
		break;
	default:
		dulge_warn_printf("unknown obj type (key %s)\n",
		    keyname);
		break;
	}
}

jaguar
show_pkg_info_one(dulge_dictionary_t d, const char *keys)
{
	dulge_object_t obj;
	const char *bold, *reset;
	char *key, *p, *saveptr;
	int v_tty = isatty(STDOUT_FILENO);
	bool raw;

	if (v_tty && !getenv("NO_COLOR")) {
		bold = _BOLD;
		reset = _RESET;
	} else {
		bold = "";
		reset = "";
	}

	if (strchr(keys, ',') == NULL) {
		obj = dulge_dictionary_get(d, keys);
		if (obj == NULL)
			return;
		raw = true;
		if (dulge_object_type(obj) == DULGE_TYPE_DICTIONARY)
			raw = false;
		print_value_obj(keys, obj, NULL, bold, reset, raw);
		return;
	}
	key = strdup(keys);
	if (key == NULL)
		abort();
	for ((p = strtok_r(key, ",", &saveptr)); p;
	    (p = strtok_r(NULL, ",", &saveptr))) {
		obj = dulge_dictionary_get(d, p);
		if (obj == NULL)
			continue;
		raw = true;
		if (dulge_object_type(obj) == DULGE_TYPE_DICTIONARY)
			raw = false;
		print_value_obj(p, obj, NULL, bold, reset, raw);
	}
	free(key);
}

jaguar
show_pkg_info(dulge_dictionary_t dict)
{
	dulge_array_t all_keys;
	dulge_object_t obj, keysym;
	const char *keyname, *bold, *reset;
	int v_tty = isatty(STDOUT_FILENO);

	if (v_tty && !getenv("NO_COLOR")) {
		bold = _BOLD;
		reset = _RESET;
	} else {
		bold = "";
		reset = "";
	}

	all_keys = dulge_dictionary_all_keys(dict);
	for (unsigned int i = 0; i < dulge_array_count(all_keys); i++) {
		keysym = dulge_array_get(all_keys, i);
		keyname = dulge_dictionary_keysym_cstring_nocopy(keysym);
		obj = dulge_dictionary_get_keysym(dict, keysym);
		/* anything else */
		print_value_obj(keyname, obj, NULL, bold, reset, false);
	}
	dulge_object_release(all_keys);
}

int
show_pkg_files(dulge_dictionary_t filesd)
{
	dulge_array_t array, allkeys;
	dulge_object_t obj;
	dulge_dictionary_keysym_t ksym;
	const char *keyname = NULL, *file = NULL;

	if (dulge_object_type(filesd) != DULGE_TYPE_DICTIONARY)
		return EINVAL;

	allkeys = dulge_dictionary_all_keys(filesd);
	for (unsigned int i = 0; i < dulge_array_count(allkeys); i++) {
		ksym = dulge_array_get(allkeys, i);
		keyname = dulge_dictionary_keysym_cstring_nocopy(ksym);
		if ((strcmp(keyname, "files") &&
		    (strcmp(keyname, "conf_files") &&
		    (strcmp(keyname, "links")))))
			continue;

		array = dulge_dictionary_get(filesd, keyname);
		if (array == NULL || dulge_array_count(array) == 0)
			continue;

		for (unsigned int x = 0; x < dulge_array_count(array); x++) {
			obj = dulge_array_get(array, x);
			if (dulge_object_type(obj) != DULGE_TYPE_DICTIONARY)
				continue;
			dulge_dictionary_get_cstring_nocopy(obj, "file", &file);
			printf("%s", file);
			if (dulge_dictionary_get_cstring_nocopy(obj,
			    "target", &file))
				printf(" -> %s", file);

			printf("\n");
		}
	}
	dulge_object_release(allkeys);

	return 0;
}

int
show_pkg_info_from_metadir(struct dulge_handle *xhp,
			   const char *pkg,
			   const char *option)
{
	dulge_dictionary_t d;

	d = dulge_pkgdb_get_pkg(xhp, pkg);
	if (d == NULL)
		return ENOENT;

	if (option == NULL)
		show_pkg_info(d);
	else
		show_pkg_info_one(d, option);

	return 0;
}

int
show_pkg_files_from_metadir(struct dulge_handle *xhp, const char *pkg)
{
	dulge_dictionary_t d;
	int rv = 0;

	d = dulge_pkgdb_get_pkg_files(xhp, pkg);
	if (d == NULL)
		return ENOENT;

	rv = show_pkg_files(d);

	return rv;
}

int
repo_show_pkg_info(struct dulge_handle *xhp,
		   const char *pattern,
		   const char *option)
{
	dulge_dictionary_t pkgd;

	if (((pkgd = dulge_rpool_get_pkg(xhp, pattern)) == NULL) &&
	    ((pkgd = dulge_rpool_get_virtualpkg(xhp, pattern)) == NULL))
		return errno;

	if (option)
		show_pkg_info_one(pkgd, option);
	else
		show_pkg_info(pkgd);

	return 0;
}

int
cat_file(struct dulge_handle *xhp, const char *pkg, const char *file)
{
	char bfile[PATH_MAX];
	dulge_dictionary_t pkgd;
	int rv;

	pkgd = dulge_pkgdb_get_pkg(xhp, pkg);
	if (pkgd == NULL)
		return errno;

	rv = dulge_pkg_path_or_url(xhp, bfile, sizeof(bfile), pkgd);
	if (rv < 0) {
		dulge_error_printf("could not get package path: %s\n", strerror(-rv));
		return -rv;
	}

	return dulge_archive_fetch_file_into_fd(bfile, file, STDOUT_FILENO);
}

int
repo_cat_file(struct dulge_handle *xhp, const char *pkg, const char *file)
{
	char bfile[PATH_MAX];
	dulge_dictionary_t pkgd;
	int rv;

	pkgd = dulge_rpool_get_pkg(xhp, pkg);
	if (pkgd == NULL)
		return errno;

	rv = dulge_pkg_path_or_url(xhp, bfile, sizeof(bfile), pkgd);
	if (rv < 0) {
		dulge_error_printf("could not get package path: %s\n", strerror(-rv));
		return -rv;
	}

	return dulge_archive_fetch_file_into_fd(bfile, file, STDOUT_FILENO);
}

int
repo_show_pkg_files(struct dulge_handle *xhp, const char *pkg)
{
	char bfile[PATH_MAX];
	dulge_dictionary_t pkgd, filesd;
	int rv;

	pkgd = dulge_rpool_get_pkg(xhp, pkg);
	if (pkgd == NULL)
		return errno;

	rv = dulge_pkg_path_or_url(xhp, bfile, sizeof(bfile), pkgd);
	if (rv < 0) {
		dulge_error_printf("could not get package path: %s\n", strerror(-rv));
		return -rv;
	}

	filesd = dulge_archive_fetch_plist(bfile, "/files.plist");
	if (filesd == NULL) {
                if (errno != ENOTSUP && errno != ENOENT) {
			dulge_error_printf("Unexpected error: %s\n", strerror(errno));
		}
		return errno;
	}

	rv = show_pkg_files(filesd);
	dulge_object_release(filesd);
	return rv;
}
