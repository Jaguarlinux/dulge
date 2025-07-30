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

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <syslog.h>

#include <dulge.h>
#include "defs.h"

static void __attribute__((noreturn))
usage(bool fail)
{
	fprintf(stdout,
	    "Usage: dulge-reconfigure [OPTIONS] [PKGNAME...]\n\n"
	    "OPTIONS\n"
	    " -a, --all            Process all packages\n"
	    " -C, --config <dir>   Path to confdir (dulge.d)\n"
	    " -d, --debug          Debug mode shown to stderr\n"
	    " -f, --force          Force reconfiguration\n"
	    "     --fulldeptree    Full dependency tree for -x/--deps\n"
	    " -h, --help           Show usage\n"
	    " -i, --ignore PKG     Ignore PKG with -a/--all\n"
	    " -r, --rootdir <dir>  Full path to rootdir\n"
	    " -x, --deps           Also process dependencies for each package\n"
	    " -v, --verbose        Verbose messages\n"
	    " -V, --version        Show DULGE version\n");
	exit(fail ? EXIT_FAILURE : EXIT_SUCCESS);
}

static int
state_cb(const struct dulge_state_cb_data *xscd, void *cbd UNUSED)
{
	bool slog = false;

	if ((xscd->xhp->flags & DULGE_FLAG_DISABLE_SYSLOG) == 0) {
		slog = true;
		openlog("dulge-reconfigure", 0, LOG_USER);
	}

	switch (xscd->state) {
	/* notifications */
	case DULGE_STATE_CONFIGURE:
		printf("%s: configuring ...\n", xscd->arg);
		if (slog)
			syslog(LOG_NOTICE, "%s: configuring ...", xscd->arg);
		break;
	case DULGE_STATE_CONFIGURE_DONE:
		printf("%s: configured successfully.\n", xscd->arg);
		if (slog)
			syslog(LOG_NOTICE,
			    "%s: configured successfully.", xscd->arg);
		break;
	/* errors */
	case DULGE_STATE_CONFIGURE_FAIL:
		dulge_error_printf("%s\n", xscd->desc);
		if (slog)
			syslog(LOG_ERR, "%s", xscd->desc);
		break;
	default:
		break;
	}

	return 0;
}

int
main(int argc, char **argv)
{
	const char *shortopts = "aC:dfhi:r:xVv";
	const struct option longopts[] = {
		{ "all", no_argument, NULL, 'a' },
		{ "config", required_argument, NULL, 'C' },
		{ "debug", no_argument, NULL, 'd' },
		{ "force", no_argument, NULL, 'f' },
		{ "help", no_argument, NULL, 'h' },
		{ "ignore", required_argument, NULL, 'i' },
		{ "rootdir", required_argument, NULL, 'r' },
		{ "deps", no_argument, NULL, 'x' },
		{ "verbose", no_argument, NULL, 'v' },
		{ "version", no_argument, NULL, 'V' },
		{ "fulldeptree", no_argument, NULL, 1 },
		{ NULL, 0, NULL, 0 }
	};
	struct dulge_handle xh;
	const char *confdir = NULL, *rootdir = NULL;
	int c, i, rv, flags = 0;
	bool all = false, rdeps = false, fulldeptree = false;
	dulge_array_t ignpkgs = NULL, deps = NULL;

	while ((c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch (c) {
		case 'a':
			all = true;
			break;
		case 'C':
			confdir = optarg;
			break;
		case 'd':
			flags |= DULGE_FLAG_DEBUG;
			break;
		case 'f':
			flags |= DULGE_FLAG_FORCE_CONFIGURE;
			break;
		case 'h':
			usage(false);
			/* NOTREACHED */
		case 'i':
			if (ignpkgs == NULL)
				ignpkgs = dulge_array_create();

			dulge_array_add_cstring_nocopy(ignpkgs, optarg);
			break;
		case 'r':
			rootdir = optarg;
			break;
		case 'x':
			rdeps = true;
			break;
		case 'v':
			flags |= DULGE_FLAG_VERBOSE;
			break;
		case 'V':
			printf("%s\n", DULGE_RELVER);
			exit(EXIT_SUCCESS);
		case 1:
			fulldeptree = true;
			break;
		case '?':
		default:
			usage(true);
			/* NOTREACHED */
		}
	}
	if (!all && (argc == optind)) {
		usage(true);
		/* NOTREACHED */
	}

	memset(&xh, 0, sizeof(xh));
	xh.state_cb = state_cb;
	if (rootdir)
		dulge_strlcpy(xh.rootdir, rootdir, sizeof(xh.rootdir));
	if (confdir)
		dulge_strlcpy(xh.confdir, confdir, sizeof(xh.confdir));

	xh.flags = flags;

	if ((rv = dulge_init(&xh)) != 0) {
		dulge_error_printf("Failed to initialize libdulge: %s\n",
		    strerror(rv));
		exit(EXIT_FAILURE);
	}

	if ((rv = dulge_pkgdb_lock(&xh)) != 0) {
		dulge_error_printf("failed to lock pkgdb: %s\n", strerror(rv));
		exit(EXIT_FAILURE);
	}

	if (all) {
		rv = dulge_configure_packages(&xh, ignpkgs);
	} else {
		for (i = optind; i < argc; i++) {
			const char* pkg = argv[i];
			if (rdeps) {
				rv = find_pkg_deps(&xh, pkg, fulldeptree, &deps);
				if (rv != 0) {
					dulge_error_printf("failed to collect dependencies for "
						"`%s': %s\n", pkg, strerror(rv));
				}
				for (unsigned int j = 0; j < dulge_array_count(deps); j++) {
					const char *pkgdep = NULL;
					char pkgname[DULGE_NAME_SIZE];
					dulge_array_get_cstring_nocopy(deps, j, &pkgdep);

					if (fulldeptree) {
						if (!dulge_pkg_name(pkgname, sizeof(pkgname), pkgdep)) {
							dulge_error_printf(
								"unable to get package name for dependency `%s'\n", pkgdep);
							exit(EXIT_FAILURE);
						}
					} else {
						if (!dulge_pkgpattern_name(pkgname, sizeof(pkgname), pkgdep)) {
							dulge_error_printf(
								"unable to get package name for dependency `%s'\n", pkgdep);
							exit(EXIT_FAILURE);
						}
					}

					rv = dulge_configure_pkg(&xh, pkgname, true, false);
					if (rv != 0) {
						dulge_error_printf("failed to reconfigure "
							"`%s': %s\n", pkgname, strerror(rv));
					}
				}
			}
			rv = dulge_configure_pkg(&xh, pkg, true, false);
			if (rv != 0) {
				dulge_error_printf("failed to reconfigure "
				    "`%s': %s\n", pkg, strerror(rv));
			}
		}
	}
	if (rv == 0)
		dulge_pkgdb_update(&xh, true, false);

	dulge_end(&xh);
	exit(rv ? EXIT_FAILURE : EXIT_SUCCESS);
}
