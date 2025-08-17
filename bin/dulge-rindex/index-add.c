/*-
 * Copyright (c) 2012-2015 Juan Romero Pardines.
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

#include <sys/stat.h>

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dulge.h>
#include "defs.h"

static int
repodata_commit(const char *repodir, const char *repoarch,
	dulge_dictionary_t index, dulge_dictionary_t stage, dulge_dictionary_t meta,
	const char *compression)
{
	dulge_object_iterator_t iter;
	dulge_object_t keysym;
	int r;
	dulge_dictionary_t oldshlibs, usedshlibs;

	if (dulge_dictionary_count(stage) == 0)
		return 0;

	/*
	 * Find old shlibs-provides
	 */
	oldshlibs = dulge_dictionary_create();
	usedshlibs = dulge_dictionary_create();

	iter = dulge_dictionary_iterator(stage);
	while ((keysym = dulge_object_iterator_next(iter))) {
		const char *pkgname = dulge_dictionary_keysym_cstring_nocopy(keysym);
		dulge_dictionary_t pkg = dulge_dictionary_get(index, pkgname);
		dulge_array_t pkgshlibs;

		pkgshlibs = dulge_dictionary_get(pkg, "shlib-provides");
		for (unsigned int i = 0; i < dulge_array_count(pkgshlibs); i++) {
			const char *shlib = NULL;
			dulge_array_get_cstring_nocopy(pkgshlibs, i, &shlib);
			dulge_dictionary_set_cstring(oldshlibs, shlib, pkgname);
		}
	}
	dulge_object_iterator_release(iter);

	/*
	 * throw away all unused shlibs
	 */
	iter = dulge_dictionary_iterator(index);
	while ((keysym = dulge_object_iterator_next(iter))) {
		const char *pkgname = dulge_dictionary_keysym_cstring_nocopy(keysym);
		dulge_dictionary_t pkg = dulge_dictionary_get(stage, pkgname);
		dulge_array_t pkgshlibs;
		if (!pkg)
			pkg = dulge_dictionary_get_keysym(index, keysym);
		pkgshlibs = dulge_dictionary_get(pkg, "shlib-requires");

		for (unsigned int i = 0; i < dulge_array_count(pkgshlibs); i++) {
			const char *shlib = NULL;
			bool alloc = false;
			dulge_array_t users;
			dulge_array_get_cstring_nocopy(pkgshlibs, i, &shlib);
			if (!dulge_dictionary_get(oldshlibs, shlib))
				continue;
			users = dulge_dictionary_get(usedshlibs, shlib);
			if (!users) {
				users = dulge_array_create();
				dulge_dictionary_set(usedshlibs, shlib, users);
				alloc = true;
			}
			dulge_array_add_cstring(users, pkgname);
			if (alloc)
				dulge_object_release(users);
		}
	}
	dulge_object_iterator_release(iter);

	/*
	 * purge all packages that are fullfilled by the index and
	 * not in the stage.
	 */
	iter = dulge_dictionary_iterator(index);
	while ((keysym = dulge_object_iterator_next(iter))) {
		dulge_dictionary_t pkg = dulge_dictionary_get_keysym(index, keysym);
		dulge_array_t pkgshlibs;


		if (dulge_dictionary_get(stage,
					dulge_dictionary_keysym_cstring_nocopy(keysym))) {
			continue;
		}

		pkgshlibs = dulge_dictionary_get(pkg, "shlib-provides");
		for (unsigned int i = 0; i < dulge_array_count(pkgshlibs); i++) {
			const char *shlib = NULL;
			dulge_array_get_cstring_nocopy(pkgshlibs, i, &shlib);
			dulge_dictionary_remove(usedshlibs, shlib);
		}
	}
	dulge_object_iterator_release(iter);

	/*
	 * purge all packages that are fullfilled by the stage
	 */
	iter = dulge_dictionary_iterator(stage);
	while ((keysym = dulge_object_iterator_next(iter))) {
		dulge_dictionary_t pkg = dulge_dictionary_get_keysym(stage, keysym);
		dulge_array_t pkgshlibs;

		pkgshlibs = dulge_dictionary_get(pkg, "shlib-provides");
		for (unsigned int i = 0; i < dulge_array_count(pkgshlibs); i++) {
			const char *shlib = NULL;
			dulge_array_get_cstring_nocopy(pkgshlibs, i, &shlib);
			dulge_dictionary_remove(usedshlibs, shlib);
		}
	}
	dulge_object_iterator_release(iter);

	if (dulge_dictionary_count(usedshlibs) != 0) {
		printf("Inconsistent shlibs:\n");
		iter = dulge_dictionary_iterator(usedshlibs);
		while ((keysym = dulge_object_iterator_next(iter))) {
			const char *shlib = dulge_dictionary_keysym_cstring_nocopy(keysym),
					*provider = NULL, *pre;
			dulge_array_t users = dulge_dictionary_get(usedshlibs, shlib);
			dulge_dictionary_get_cstring_nocopy(oldshlibs, shlib, &provider);

			printf("  %s (provided by: %s; used by: ", shlib, provider);
			pre = "";
			for (unsigned int i = 0; i < dulge_array_count(users); i++) {
				const char *user = NULL;
				dulge_array_get_cstring_nocopy(users, i, &user);
				printf("%s%s", pre, user);
				pre = ", ";
			}
			printf(")\n");
		}
		dulge_object_iterator_release(iter);
		iter = dulge_dictionary_iterator(stage);
		while ((keysym = dulge_object_iterator_next(iter))) {
			dulge_dictionary_t pkg = dulge_dictionary_get_keysym(stage, keysym);
			const char *pkgver = NULL, *arch = NULL;
			dulge_dictionary_get_cstring_nocopy(pkg, "pkgver", &pkgver);
			dulge_dictionary_get_cstring_nocopy(pkg, "architecture", &arch);
			printf("stage: added `%s' (%s)\n", pkgver, arch);
		}
		dulge_object_iterator_release(iter);
	} else {
		iter = dulge_dictionary_iterator(stage);
		while ((keysym = dulge_object_iterator_next(iter))) {
			const char *pkgname = dulge_dictionary_keysym_cstring_nocopy(keysym);
			dulge_dictionary_t pkg = dulge_dictionary_get_keysym(stage, keysym);
			const char *pkgver = NULL, *arch = NULL;
			dulge_dictionary_get_cstring_nocopy(pkg, "pkgver", &pkgver);
			dulge_dictionary_get_cstring_nocopy(pkg, "architecture", &arch);
			printf("index: added `%s' (%s).\n", pkgver, arch);
			dulge_dictionary_set(index, pkgname, pkg);
		}
		dulge_object_iterator_release(iter);
		stage = NULL;
	}

	r = repodata_flush(repodir, repoarch, index, stage, meta, compression);
	dulge_object_release(usedshlibs);
	dulge_object_release(oldshlibs);
	return r;
}

static int
index_add_pkg(struct dulge_handle *xhp, dulge_dictionary_t index, dulge_dictionary_t stage,
		const char *file, bool force)
{
	char sha256[DULGE_SHA256_SIZE];
	char pkgname[DULGE_NAME_SIZE];
	struct stat st;
	const char *arch = NULL;
	const char *pkgver = NULL;
	dulge_dictionary_t binpkgd, curpkgd;
	int r;

	/*
	 * Read metadata props plist dictionary from binary package.
	 */
	binpkgd = dulge_archive_fetch_plist(file, "/props.plist");
	if (!binpkgd) {
		dulge_error_printf("index: failed to read %s metadata for "
		    "`%s', skipping!\n", DULGE_PKGPROPS, file);
		return 0;
	}
	dulge_dictionary_get_cstring_nocopy(binpkgd, "architecture", &arch);
	dulge_dictionary_get_cstring_nocopy(binpkgd, "pkgver", &pkgver);
	if (!dulge_pkg_arch_match(xhp, arch, NULL)) {
		fprintf(stderr, "index: ignoring %s, unmatched arch (%s)\n", pkgver, arch);
		goto out;
	}
	if (!dulge_pkg_name(pkgname, sizeof(pkgname), pkgver)) {
		r = -EINVAL;
		goto err;
	}

	/*
	 * Check if this package exists already in the index, but first
	 * checking the version. If current package version is greater
	 * than current registered package, update the index; otherwise
	 * pass to the next one.
	 */
	curpkgd = dulge_dictionary_get(stage, pkgname);
	if (!curpkgd)
		curpkgd = dulge_dictionary_get(index, pkgname);

	if (curpkgd && !force) {
		const char *opkgver = NULL, *oarch = NULL;
		int cmp;

		dulge_dictionary_get_cstring_nocopy(curpkgd, "pkgver", &opkgver);
		dulge_dictionary_get_cstring_nocopy(curpkgd, "architecture", &oarch);

		cmp = dulge_cmpver(pkgver, opkgver);
		if (cmp < 0 && dulge_pkg_reverts(binpkgd, opkgver)) {
			/*
			 * If the considered package reverts the package in the index,
			 * consider the current package as the newer one.
			 */
			cmp = 1;
		} else if (cmp > 0 && dulge_pkg_reverts(curpkgd, pkgver)) {
			/*
			 * If package in the index reverts considered package, consider the
			 * package in the index as the newer one.
			 */
			cmp = -1;
		}
		if (cmp <= 0) {
			fprintf(stderr, "index: skipping `%s' (%s), already registered.\n", pkgver, arch);
			goto out;
		}
	}

	if (!dulge_file_sha256(sha256, sizeof(sha256), file))
		goto err_errno;
	if (!dulge_dictionary_set_cstring(binpkgd, "filename-sha256", sha256))
		goto err_errno;
	if (stat(file, &st) == -1)
		goto err_errno;
	if (!dulge_dictionary_set_uint64(binpkgd, "filename-size", (uint64_t)st.st_size))
		goto err_errno;

	dulge_dictionary_remove(binpkgd, "pkgname");
	dulge_dictionary_remove(binpkgd, "version");
	dulge_dictionary_remove(binpkgd, "packaged-with");

	/*
	 * Add new pkg dictionary into the stage index
	 */
	if (!dulge_dictionary_set(stage, pkgname, binpkgd))
		goto err_errno;

out:
	dulge_object_release(binpkgd);
	return 0;
err_errno:
	r = -errno;
err:
	dulge_object_release(binpkgd);
	return r;
}

int
index_add(struct dulge_handle *xhp, int args, int argc, char **argv, bool force, const char *compression)
{
	dulge_dictionary_t index, stage, meta;
	struct dulge_repo *repo;
	char *tmprepodir = NULL, *repodir = NULL;
	int lockfd;
	int r;
	const char *repoarch = xhp->target_arch ? xhp->target_arch : xhp->native_arch;

	if ((tmprepodir = strdup(argv[args])) == NULL)
		return EXIT_FAILURE;
	repodir = dirname(tmprepodir);

	lockfd = dulge_repo_lock(repodir, repoarch);
	if (lockfd < 0) {
		dulge_error_printf("dulge-rindex: cannot lock repository "
		    "%s: %s\n", repodir, strerror(-lockfd));
		free(tmprepodir);
		return EXIT_FAILURE;
	}

	repo = dulge_repo_open(xhp, repodir);
	if (!repo && errno != ENOENT) {
		free(tmprepodir);
		return EXIT_FAILURE;
	}

	if (repo) {
		index = dulge_dictionary_copy_mutable(repo->index);
		stage = dulge_dictionary_copy_mutable(repo->stage);
		meta = dulge_dictionary_copy_mutable(repo->idxmeta);
	} else {
		index = dulge_dictionary_create();
		stage = dulge_dictionary_create();
		meta = NULL;
	}

	for (int i = args; i < argc; i++) {
		r = index_add_pkg(xhp, index, stage, argv[i], force);
		if (r < 0)
			goto err2;
	}

	r = repodata_commit(repodir, repoarch, index, stage, meta, compression);
	if (r < 0) {
		dulge_error_printf("failed to write repodata: %s\n", strerror(-r));
		goto err2;
	}
	printf("index: %u packages registered.\n", dulge_dictionary_count(index));

	dulge_object_release(index);
	dulge_object_release(stage);
	if (meta)
		dulge_object_release(meta);
	dulge_repo_release(repo);
	dulge_repo_unlock(repodir, repoarch, lockfd);
	free(tmprepodir);
	return EXIT_SUCCESS;

err2:
	dulge_object_release(index);
	dulge_object_release(stage);
	if (meta)
		dulge_object_release(meta);
	dulge_repo_release(repo);
	dulge_repo_unlock(repodir, repoarch, lockfd);
	free(tmprepodir);
	return EXIT_FAILURE;
}
