/*-
 * Copyright (c) 2025 TigerClips1 <spongebob1966@proton.me>.
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
#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>
#include <limits.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dulge.h>
#include "defs.h"

struct ffdata {
	bool rematch;
	const char *pat, *repouri;
	regex_t regex;
	dulge_array_t allkeys;
	dulge_dictionary_t filesd;
};

static void
match_files_by_pattern(dulge_dictionary_t pkg_filesd,
		       dulge_dictionary_keysym_t key,
		       struct ffdata *ffd,
		       const char *pkgver)
{
	dulge_array_t array;
	const char *keyname = NULL, *typestr = NULL;

	keyname = dulge_dictionary_keysym_cstring_nocopy(key);

	if (strcmp(keyname, "files") == 0)
		typestr = "regular file";
	else if (strcmp(keyname, "links") == 0)
		typestr = "link";
	else if (strcmp(keyname, "conf_files") == 0)
		typestr = "configuration file";
	else
		return;

	array = dulge_dictionary_get_keysym(pkg_filesd, key);
	for (unsigned int i = 0; i < dulge_array_count(array); i++) {
		dulge_object_t obj;
		const char *filestr = NULL, *tgt = NULL;

		obj = dulge_array_get(array, i);
		dulge_dictionary_get_cstring_nocopy(obj, "file", &filestr);
		if (filestr == NULL)
			continue;
		dulge_dictionary_get_cstring_nocopy(obj, "target", &tgt);
		if (ffd->rematch) {
			if (regexec(&ffd->regex, filestr, 0, 0, 0) == 0) {
				printf("%s: %s%s%s (%s)\n",
					pkgver, filestr,
					tgt ? " -> " : "",
					tgt ? tgt : "",
					typestr);
			}
		} else {
			if ((fnmatch(ffd->pat, filestr, FNM_PERIOD)) == 0) {
				printf("%s: %s%s%s (%s)\n",
					pkgver, filestr,
					tgt ? " -> " : "",
					tgt ? tgt : "",
					typestr);
			}
		}
	}
}

static int
ownedby_pkgdb_cb(struct dulge_handle *xhp,
		dulge_object_t obj,
		const char *obj_key UNUSED,
		void *arg,
		bool *done UNUSED)
{
	dulge_dictionary_t pkgmetad;
	dulge_array_t files_keys;
	struct ffdata *ffd = arg;
	const char *pkgver = NULL;

	(void)obj_key;
	(void)done;

	dulge_dictionary_get_cstring_nocopy(obj, "pkgver", &pkgver);
	pkgmetad = dulge_pkgdb_get_pkg_files(xhp, pkgver);
	if (pkgmetad == NULL)
		return 0;

	files_keys = dulge_dictionary_all_keys(pkgmetad);
	for (unsigned int i = 0; i < dulge_array_count(files_keys); i++) {
		match_files_by_pattern(pkgmetad,
		    dulge_array_get(files_keys, i), ffd, pkgver);
	}
	dulge_object_release(pkgmetad);
	dulge_object_release(files_keys);

	return 0;
}


static int
repo_match_cb(struct dulge_handle *xhp,
		dulge_object_t obj,
		const char *key UNUSED,
		void *arg,
		bool *done UNUSED)
{
	char bfile[PATH_MAX];
	dulge_dictionary_t filesd;
	dulge_array_t files_keys;
	struct ffdata *ffd = arg;
	const char *pkgver = NULL;
	int r;

	dulge_dictionary_set_cstring_nocopy(obj, "repository", ffd->repouri);
	dulge_dictionary_get_cstring_nocopy(obj, "pkgver", &pkgver);

	r = dulge_pkg_path_or_url(xhp, bfile, sizeof(bfile), obj);
	if (r < 0) {
		dulge_error_printf("could not get package path: %s\n", strerror(-r));
		return -r;
	}
	filesd = dulge_archive_fetch_plist(bfile, "/files.plist");
	if (!filesd) {
		dulge_error_printf("%s: couldn't fetch files.plist from %s: %s\n",
		    pkgver, bfile, strerror(errno));
		return EINVAL;
	}
	files_keys = dulge_dictionary_all_keys(filesd);
	for (unsigned int i = 0; i < dulge_array_count(files_keys); i++) {
		match_files_by_pattern(filesd,
		    dulge_array_get(files_keys, i), ffd, pkgver);
	}
	dulge_object_release(files_keys);
	dulge_object_release(filesd);

	return 0;
}

static int
repo_ownedby_cb(struct dulge_repo *repo, void *arg, bool *done UNUSED)
{
	dulge_array_t allkeys;
	struct ffdata *ffd = arg;
	int rv;

	ffd->repouri = repo->uri;
	allkeys = dulge_dictionary_all_keys(repo->idx);
	rv = dulge_array_foreach_cb_multi(repo->xhp, allkeys, repo->idx, repo_match_cb, ffd);
	dulge_object_release(allkeys);

	return rv;
}

int
ownedby(struct dulge_handle *xhp, const char *pat, bool repo, bool regex)
{
	struct ffdata ffd;
	int rv;

	ffd.rematch = false;
	ffd.pat = pat;

	if (regex) {
		ffd.rematch = true;
		if (regcomp(&ffd.regex, ffd.pat, REG_EXTENDED|REG_NOSUB|REG_ICASE) != 0)
			return EINVAL;
	}
	if (repo)
		rv = dulge_rpool_foreach(xhp, repo_ownedby_cb, &ffd);
	else
		rv = dulge_pkgdb_foreach_cb(xhp, ownedby_pkgdb_cb, &ffd);

	if (regex)
		regfree(&ffd.regex);

	return rv;
}
