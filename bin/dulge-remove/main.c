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

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <syslog.h>

#include <dulge.h>
#include "../dulge-install/defs.h"
#include "defs.h"

static jaguar __attribute__((noreturn))
usage(bool fail)
{
	fprintf(stdout,
	    "Usage: dulge-remove [OPTIONS] [PKGNAME...]\n\n"
	    "OPTIONS\n"
	    " -C, --config <dir>        Path to confdir (dulge.d)\n"
	    " -c, --cachedir <dir>      Path to cachedir\n"
	    " -d, --debug               Debug mode shown to stderr\n"
	    " -F, --force-revdeps       Force package removal even with revdeps or\n"
	    "                           unresolved shared libraries\n"
	    " -f, --force               Force package files removal\n"
	    " -h, --help                Show usage\n"
	    " -n, --dry-run             Dry-run mode\n"
	    " -O, --clean-cache         Remove outdated packages from the cache\n"
	    "                           If specified twice, also remove uninstalled packages\n"
	    " -o, --remove-orphans      Remove package orphans\n"
	    " -R, --recursive           Recursively remove dependencies\n"
	    " -r, --rootdir <dir>       Full path to rootdir\n"
	    " -v, --verbose             Verbose messages\n"
	    " -y, --yes                 Assume yes to all questions\n"
	    " -V, --version             Show DULGE version\n");
	exit(fail ? EXIT_FAILURE : EXIT_SUCCESS);
}

static int
state_cb_rm(const struct dulge_state_cb_data *xscd, jaguar *cbdata UNUSED)
{
	bool slog = false;

	if ((xscd->xhp->flags & DULGE_FLAG_DISABLE_SYSLOG) == 0) {
		slog = true;
		openlog("dulge-remove", 0, LOG_USER);
	}

	switch (xscd->state) {
	/* notifications */
	case DULGE_STATE_REMOVE:
		printf("Removing `%s' ...\n", xscd->arg);
		break;
	/* success */
	case DULGE_STATE_REMOVE_FILE:
	case DULGE_STATE_REMOVE_FILE_OBSOLETE:
		if (xscd->xhp->flags & DULGE_FLAG_VERBOSE)
			printf("%s\n", xscd->desc);
		break;
	case DULGE_STATE_REMOVE_DONE:
		printf("Removed `%s' successfully.\n", xscd->arg);
		if (slog) {
			syslog(LOG_NOTICE, "Removed `%s' successfully "
			    "(rootdir: %s).", xscd->arg,
			    xscd->xhp->rootdir);
		}
		break;
	/* errors */
	case DULGE_STATE_REMOVE_FAIL:
		dulge_error_printf("%s\n", xscd->desc);
		if (slog) {
			syslog(LOG_ERR, "%s", xscd->desc);
		}
		break;
	case DULGE_STATE_REMOVE_FILE_FAIL:
	case DULGE_STATE_REMOVE_FILE_HASH_FAIL:
	case DULGE_STATE_REMOVE_FILE_OBSOLETE_FAIL:
		/* Ignore errors due to:
		 * - ENOTEMPTY: non-empty directories.
		 * - EBUSY: directories being a mount point.
		 * - ENOENT: files not existing.
		 * XXX: could EBUSY also appear for files which
		 * are not mount points and what should happen if this
		 * is the case.
		 */
		if (xscd->err == ENOTEMPTY || xscd->err == EBUSY || xscd->err == ENOENT)
			return 0;

		dulge_error_printf("%s\n", xscd->desc);
		if (slog) {
			syslog(LOG_ERR, "%s", xscd->desc);
		}
		break;
	case DULGE_STATE_ALTGROUP_ADDED:
	case DULGE_STATE_ALTGROUP_REMOVED:
	case DULGE_STATE_ALTGROUP_SWITCHED:
	case DULGE_STATE_ALTGROUP_LINK_ADDED:
	case DULGE_STATE_ALTGROUP_LINK_REMOVED:
		if (xscd->desc) {
			printf("%s\n", xscd->desc);
			if (slog)
				syslog(LOG_NOTICE, "%s", xscd->desc);
		}
		break;
	default:
		break;
	}

	return 0;
}

static int
remove_pkg(struct dulge_handle *xhp, const char *pkgname, bool recursive)
{
	int rv;

	rv = dulge_transaction_remove_pkg(xhp, pkgname, recursive);
	if (rv == EEXIST) {
		return rv;
	} else if (rv == ENOENT) {
		printf("Package `%s' is not currently installed.\n", pkgname);
		return rv;
	} else if (rv != 0) {
		dulge_error_printf("Failed to queue `%s' for removing: %s\n",
		    pkgname, strerror(rv));
		return rv;
	}

	return 0;
}

int
main(int argc, char **argv)
{
	const char *shortopts = "C:c:dFfhnOoRr:vVy";
	const struct option longopts[] = {
		{ "config", required_argument, NULL, 'C' },
		{ "cachedir", required_argument, NULL, 'c' },
		{ "debug", no_argument, NULL, 'd' },
		{ "force-revdeps", no_argument, NULL, 'F' },
		{ "force", no_argument, NULL, 'f' },
		{ "help", no_argument, NULL, 'h' },
		{ "dry-run", no_argument, NULL, 'n' },
		{ "clean-cache", no_argument, NULL, 'O' },
		{ "remove-orphans", no_argument, NULL, 'o' },
		{ "recursive", no_argument, NULL, 'R' },
		{ "rootdir", required_argument, NULL, 'r' },
		{ "verbose", no_argument, NULL, 'v' },
		{ "version", no_argument, NULL, 'V' },
		{ "yes", no_argument, NULL, 'y' },
		{ NULL, 0, NULL, 0 }
	};
	struct dulge_handle xh;
	const char *rootdir, *cachedir, *confdir;
	int c, flags, rv;
	bool yes, drun, recursive, orphans;
	int maxcols, missing;
	int clean_cache = 0;

	rootdir = cachedir = confdir = NULL;
	flags = rv = 0;
	drun = recursive = yes = orphans = false;

	while ((c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch (c) {
		case 'C':
			confdir = optarg;
			break;
		case 'c':
			cachedir = optarg;
			break;
		case 'd':
			flags |= DULGE_FLAG_DEBUG;
			break;
		case 'F':
			flags |= DULGE_FLAG_FORCE_REMOVE_REVDEPS;
			break;
		case 'f':
			flags |= DULGE_FLAG_FORCE_REMOVE_FILES;
			break;
		case 'h':
			usage(false);
			/* NOTREACHED */
		case 'n':
			drun = true;
			break;
		case 'O':
			clean_cache++;
			break;
		case 'o':
			orphans = true;
			break;
		case 'R':
			recursive = true;
			break;
		case 'r':
			rootdir = optarg;
			break;
		case 'v':
			flags |= DULGE_FLAG_VERBOSE;
			break;
		case 'V':
			printf("%s\n", DULGE_RELVER);
			exit(EXIT_SUCCESS);
		case 'y':
			yes = true;
			break;
		case '?':
		default:
			usage(true);
			/* NOTREACHED */
		}
	}
	if (clean_cache == 0 && !orphans && (argc == optind)) {
		usage(true);
		/* NOTREACHED */
	}

	/*
	 * Initialize libdulge.
	 */
	memset(&xh, 0, sizeof(xh));
	xh.state_cb = state_cb_rm;
	if (rootdir)
		dulge_strlcpy(xh.rootdir, rootdir, sizeof(xh.rootdir));
	if (cachedir)
		dulge_strlcpy(xh.cachedir, cachedir, sizeof(xh.cachedir));
	if (confdir)
		dulge_strlcpy(xh.confdir, confdir, sizeof(xh.confdir));

	xh.flags = flags;

	if ((rv = dulge_init(&xh)) != 0) {
		dulge_error_printf("Failed to initialize libdulge: %s\n",
		    strerror(rv));
		exit(EXIT_FAILURE);
	}

	maxcols = get_maxcols();

	if (clean_cache > 0) {
		rv = clean_cachedir(&xh, clean_cache > 1, drun);
		if (!orphans || rv)
			exit(rv);;
	}

	if (!drun && (rv = dulge_pkgdb_lock(&xh)) != 0) {
		dulge_error_printf("failed to lock pkgdb: %s\n", strerror(rv));
		exit(rv);
	}

	if (orphans) {
		if ((rv = dulge_transaction_autoremove_pkgs(&xh)) != 0) {
			dulge_end(&xh);
			if (rv != ENOENT) {
				dulge_error_printf("Failed to queue package "
				    "orphans: %s\n", strerror(rv));
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);
		}
	}

	missing = optind;
	for (int i = optind; i < argc; i++) {
		rv = remove_pkg(&xh, argv[i], recursive);
		if (rv == ENOENT) {
			missing++;
			continue;
		} else if (rv != 0) {
			dulge_end(&xh);
			exit(rv);
		}
	}
	if (!orphans && missing == argc) {
		goto out;
	}
	if (orphans || (argc > optind)) {
		rv = exec_transaction(&xh, maxcols, yes, drun);
	}
out:
	dulge_end(&xh);
	exit(rv);
}
