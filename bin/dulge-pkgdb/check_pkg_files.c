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
#include <sys/param.h>

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dulge.h>
#include "defs.h"

/*
 * Checks package integrity of an installed package.
 * The following tasks are processed in that order:
 *
 * 	o Check for missing installed files.
 *
 * 	o Check the hash for all installed files, except
 * 	  configuration files (which is expected if they are modified).
 *
 * 	o Compares stored file modification time.
 *
 * Return 0 if test ran successfully, 1 otherwise and -1 on error.
 */

int
check_pkg_files(struct dulge_handle *xhp, const char *pkgname, dulge_dictionary_t pkg_filesd)
{
	dulge_array_t array;
	dulge_object_t obj;
	dulge_object_iterator_t iter;
	const char *file = NULL, *sha256 = NULL;
	char *path;
	bool mutable, test_broken = false;
	int rv = 0, errors = 0;

	array = dulge_dictionary_get(pkg_filesd, "files");
	if (array != NULL && dulge_array_count(array) > 0) {
		iter = dulge_array_iter_from_dict(pkg_filesd, "files");
		if (iter == NULL)
			return -1;

		while ((obj = dulge_object_iterator_next(iter))) {
			dulge_dictionary_get_cstring_nocopy(obj, "file", &file);
			/* skip noextract files */
			if (xhp->noextract && dulge_patterns_match(xhp->noextract, file))
				continue;
			path = dulge_xasprintf("%s/%s", xhp->rootdir, file);
			dulge_dictionary_get_cstring_nocopy(obj,
				"sha256", &sha256);
			rv = dulge_file_sha256_check(path, sha256);
			switch (rv) {
			case 0:
				free(path);
				break;
			case ENOENT:
				dulge_error_printf("%s: unexistent file %s.\n",
				    pkgname, file);
				free(path);
				test_broken = true;
				break;
			case ERANGE:
				mutable = false;
				dulge_dictionary_get_bool(obj,
				    "mutable", &mutable);
				if (!mutable) {
					dulge_error_printf("%s: hash mismatch "
					    "for %s.\n", pkgname, file);
					test_broken = true;
				}
				free(path);
				break;
			default:
				dulge_error_printf(
				    "%s: can't check `%s' (%s)\n",
				    pkgname, file, strerror(rv));
				free(path);
				break;
			}
                }
                dulge_object_iterator_release(iter);
	}
	if (test_broken) {
		dulge_error_printf("%s: files check FAILED.\n", pkgname);
		test_broken = false;
		errors++;
	}

	/*
	 * Check for missing configuration files.
	 */
	array = dulge_dictionary_get(pkg_filesd, "conf_files");
	if (array != NULL && dulge_array_count(array) > 0) {
		iter = dulge_array_iter_from_dict(pkg_filesd, "conf_files");
		if (iter == NULL)
			return -1;

		while ((obj = dulge_object_iterator_next(iter))) {
			dulge_dictionary_get_cstring_nocopy(obj, "file", &file);
			/* skip noextract files */
			if (xhp->noextract && dulge_patterns_match(xhp->noextract, file))
				continue;
			path = dulge_xasprintf("%s/%s", xhp->rootdir, file);
			if (access(path, R_OK) == -1) {
				if (errno == ENOENT) {
					dulge_error_printf(
					    "%s: unexistent file %s\n",
					    pkgname, file);
					test_broken = true;
				} else
					dulge_error_printf(
					    "%s: can't check `%s' (%s)\n",
					    pkgname, file,
					    strerror(errno));
			}
			free(path);
		}
		dulge_object_iterator_release(iter);
	}
	if (test_broken) {
		dulge_error_printf("%s: conf files check FAILED.\n", pkgname);
		errors++;
	}

	return errors ? -1 : 0;
}
