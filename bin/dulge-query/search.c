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

#include "compat.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>
#include <fnmatch.h>
#include <assert.h>
#include <regex.h>

#include <dulge.h>
#include "defs.h"

struct search_data {
	bool regex, repo_mode;
	regex_t regexp;
	unsigned int maxcols;
	const char *pat, *prop, *repourl;
	dulge_array_t results;
	char *linebuf;
};

static void
print_results(struct dulge_handle *xhp, struct search_data *sd)
{
	const char *pkgver = NULL, *desc = NULL;
	unsigned int align = 0, len;

	/* Iterate over results array and find out largest pkgver string */
	for (unsigned int i = 0; i < dulge_array_count(sd->results); i += 2) {
		dulge_array_get_cstring_nocopy(sd->results, i, &pkgver);
		if ((len = strlen(pkgver)) > align)
			align = len;
	}
	for (unsigned int i = 0; i < dulge_array_count(sd->results); i += 2) {
		dulge_array_get_cstring_nocopy(sd->results, i, &pkgver);
		dulge_array_get_cstring_nocopy(sd->results, i+1, &desc);

		if (sd->linebuf == NULL) {
			printf("[%s] %-*s %s\n",
				dulge_pkgdb_get_pkg(xhp, pkgver) ? "*" : "-",
				align, pkgver, desc);
			continue;
		}

		len = snprintf(sd->linebuf, sd->maxcols, "[%s] %-*s %s",
		    dulge_pkgdb_get_pkg(xhp, pkgver) ? "*" : "-",
		    align, pkgver, desc);
		/* add ellipsis if the line was truncated */
		if (len >= sd->maxcols && sd->maxcols > 4) {
			for (unsigned int j = 0; j < 3; j++)
				sd->linebuf[sd->maxcols-j-1] = '.';
			sd->linebuf[sd->maxcols] = '\0';
		}
		puts(sd->linebuf);
	}
}

static int
search_array_cb(struct dulge_handle *xhp UNUSED,
		dulge_object_t obj,
		const char *key UNUSED,
		void *arg,
		bool *done UNUSED)
{
	dulge_object_t obj2;
	struct search_data *sd = arg;
	const char *pkgver = NULL, *desc = NULL, *str = NULL;

	if (!dulge_dictionary_get_cstring_nocopy(obj, "pkgver", &pkgver))
		return 0;

	if (sd->prop == NULL) {
		bool vpkgfound = false;
		/* no prop set, match on pkgver/short_desc objects */
		dulge_dictionary_get_cstring_nocopy(obj, "short_desc", &desc);

		if (sd->repo_mode && dulge_match_virtual_pkg_in_dict(obj, sd->pat))
			vpkgfound = true;

		if (sd->regex) {
			if ((regexec(&sd->regexp, pkgver, 0, 0, 0) == 0) ||
			    (regexec(&sd->regexp, desc, 0, 0, 0) == 0)) {
				dulge_array_add_cstring_nocopy(sd->results, pkgver);
				dulge_array_add_cstring_nocopy(sd->results, desc);
			}
			return 0;
		}
		if (vpkgfound) {
			dulge_array_add_cstring_nocopy(sd->results, pkgver);
			dulge_array_add_cstring_nocopy(sd->results, desc);
		} else {
			if ((strcasestr(pkgver, sd->pat)) ||
			    (strcasestr(desc, sd->pat)) ||
			    (dulge_pkgpattern_match(pkgver, sd->pat))) {
				dulge_array_add_cstring_nocopy(sd->results, pkgver);
				dulge_array_add_cstring_nocopy(sd->results, desc);
			}
		}
		return 0;
	}
	/* prop set, match on prop object instead */
	obj2 = dulge_dictionary_get(obj, sd->prop);
	if (dulge_object_type(obj2) == DULGE_TYPE_ARRAY) {
		/* property is an array */
		for (unsigned int i = 0; i < dulge_array_count(obj2); i++) {
			dulge_array_get_cstring_nocopy(obj2, i, &str);
			if (sd->regex) {
				if (regexec(&sd->regexp, str, 0, 0, 0) == 0) {
					if (sd->repo_mode)
						printf("%s: %s (%s)\n", pkgver, str, sd->repourl);
					else
						printf("%s: %s\n", pkgver, str);
				}
			} else {
				if (strcasestr(str, sd->pat)) {
					if (sd->repo_mode)
						printf("%s: %s (%s)\n", pkgver, str, sd->repourl);
					else
						printf("%s: %s\n", pkgver, str);
				}
			}
		}
	} else if (dulge_object_type(obj2) == DULGE_TYPE_NUMBER) {
		/* property is a number */
		char size[8];

		if (dulge_humanize_number(size, dulge_number_integer_value(obj2)) == -1)
			exit(EXIT_FAILURE);

		if (sd->regex) {
			if (regexec(&sd->regexp, size, 0, 0, 0) == 0) {
				if (sd->repo_mode)
					printf("%s: %s (%s)\n", pkgver, size, sd->repourl);
				else
					printf("%s: %s\n", pkgver, size);
			}
		} else {
			if (strcasestr(size, sd->pat)) {
				if (sd->repo_mode)
					printf("%s: %s (%s)\n", pkgver, size, sd->repourl);
				else
					printf("%s: %s\n", pkgver, size);
			}
		}
	} else if (dulge_object_type(obj2) == DULGE_TYPE_BOOL) {
		/* property is a bool */
		if (sd->repo_mode)
			printf("%s: true (%s)\n", pkgver, sd->repourl);
		else
			printf("%s: true\n", pkgver);

	} else if (dulge_object_type(obj2) == DULGE_TYPE_STRING) {
		/* property is a string */
		str = dulge_string_cstring_nocopy(obj2);
		if (sd->regex) {
			if (regexec(&sd->regexp, str, 0, 0, 0) == 0) {
				if (sd->repo_mode)
					printf("%s: %s (%s)\n", pkgver, str, sd->repourl);
				else
					printf("%s: %s\n", pkgver, str);
			}
		} else {
			if (strcasestr(str, sd->pat)) {
				if (sd->repo_mode)
					printf("%s: %s (%s)\n", pkgver, str, sd->repourl);
				else
					printf("%s: %s\n", pkgver, str);
			}
		}
	}
	return 0;
}

static int
search_repo_cb(struct dulge_repo *repo, void *arg, bool *done UNUSED)
{
	dulge_array_t allkeys;
	struct search_data *sd = arg;
	int rv;

	if (repo->idx == NULL)
		return 0;

	sd->repourl = repo->uri;
	allkeys = dulge_dictionary_all_keys(repo->idx);
	rv = dulge_array_foreach_cb(repo->xhp, allkeys, repo->idx, search_array_cb, sd);
	dulge_object_release(allkeys);
	return rv;
}

int
search(struct dulge_handle *xhp, bool repo_mode, const char *pat, const char *prop, bool regex)
{
	struct search_data sd;
	int rv;

	sd.regex = regex;
	if (regex) {
		if (regcomp(&sd.regexp, pat, REG_EXTENDED|REG_NOSUB|REG_ICASE) != 0)
			return errno;
	}
	sd.repo_mode = repo_mode;
	sd.pat = pat;
	sd.prop = prop;
	sd.maxcols = get_maxcols();
	sd.results = dulge_array_create();
	sd.linebuf = NULL;
	if (sd.maxcols > 0) {
		sd.linebuf = malloc(sd.maxcols);
		if (sd.linebuf == NULL)
			exit(1);
	}

	if (repo_mode) {
		rv = dulge_rpool_foreach(xhp, search_repo_cb, &sd);
		if (rv != 0 && rv != ENOTSUP) {
			dulge_error_printf("Failed to initialize rpool: %s\n",
			    strerror(rv));
			return rv;
		}
	} else {
		rv = dulge_pkgdb_foreach_cb(xhp, search_array_cb, &sd);
		if (rv != 0) {
			dulge_error_printf("Failed to initialize pkgdb: %s\n",
			    strerror(rv));
			return rv;
		}
	}
	if (!prop && dulge_array_count(sd.results)) {
		print_results(xhp, &sd);
		dulge_object_release(sd.results);
	}
	if (regex)
		regfree(&sd.regexp);

	return rv;
}
