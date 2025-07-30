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

#include <assert.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dulge.h>
#include "defs.h"

/*
 * Checks package integrity of an installed package.
 * The following task is accomplished in this file:
 *
 * 	o Check for target file in symlinks, so that we can check that
 * 	  they have not been modified.
 *
 * returns 0 if test ran successfully, 1 otherwise and -1 on error.
 */

int
check_pkg_symlinks(struct dulge_handle *xhp, const char *pkgname, dulge_dictionary_t filesd)
{
	dulge_array_t array;
	dulge_object_t obj;
	int rv = 0;

	array = dulge_dictionary_get(filesd, "links");
	if (array == NULL)
		return 0;

	for (unsigned int i = 0; i < dulge_array_count(array); i++) {
		const char *file = NULL, *tgt = NULL;
		char path[PATH_MAX], *lnk = NULL;

		obj = dulge_array_get(array, i);
		if (!dulge_dictionary_get_cstring_nocopy(obj, "file", &file))
			continue;

		/* skip noextract files */
		if (xhp->noextract && dulge_patterns_match(xhp->noextract, file))
			continue;

		if (!dulge_dictionary_get_cstring_nocopy(obj, "target", &tgt)) {
			dulge_warn_printf("%s: `%s' symlink with "
			    "empty target object!\n", pkgname, file);
			continue;
		}
		if (tgt[0] == '\0') {
			dulge_warn_printf("%s: `%s' symlink with "
			    "empty target object!\n", pkgname, file);
			continue;
		}
		snprintf(path, sizeof(path), "%s/%s", xhp->rootdir, file);
		if ((lnk = dulge_symlink_target(xhp, path, tgt)) == NULL) {
			dulge_error_printf("%s: broken symlink %s (target: %s)\n", pkgname, file, tgt);
			rv = -1;
			continue;
		}
		if (strcmp(lnk, tgt)) {
			dulge_warn_printf("%s: modified symlink %s "
			    "points to %s (shall be %s)\n",
			    pkgname, file, lnk, tgt);
			rv = -1;
		}
		free(lnk);
	}
	return rv;
}
