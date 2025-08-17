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
#include <ctype.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>

#include <dulge.h>
#include "../dulge-install/defs.h"

static jaguar __attribute__((noreturn))
usage(bool fail)
{
	fprintf(stdout,
	    "Usage: dulge-uhelper [OPTIONS] [MODE] [ARGUMENTS]\n\n"
	    "OPTIONS\n"
	    " -C --config <dir>                    Path to confdir (dulge.d)\n"
	    " -d --debug                           Debug mode shown to stderr\n"
	    " -r --rootdir <dir>                   Full path to rootdir\n"
	    " -v --verbose                         Verbose messages\n"
	    " -V --version                         Show DULGE version\n"
	    "\n"
	    "MODE\n"
	    " arch                                 Prints the configured DULGE architecture\n"
	    " binpkgarch <binpkg...>               Prints the architecture of binpkg names\n"
	    " binpkgver <binpkg...>                Prints the pkgver of binpkg names\n"
	    " cmpver <version> <version>           Compare two version strings\n"
	    " getname <pkgver|dep...>              Prints pkgname from pkgvers or dependencies\n"
	    " getpkgdepname <dep...>               Prints pkgname from dependencies\n"
	    " getpkgdepversion <dep...>            Prints version constraint from dependencies\n"
	    " getpkgname <pkgver...>               Prints pkgname from pkgvers\n"
	    " getpkgrevision <pkgver...>           Prints revision from pkgvers\n"
	    " getpkgversion <pkgver...>            Prints version from pkgvers\n"
	    " getversion <pkgver|dep...>           Prints version from patterns or pkgvers\n"
	    " pkgmatch <pkgver> <dep>              Match pkgver against dependency\n"
	    " real-version <pkgname...>            Prints version of installed real packages\n"
	    " version <pkgname...>                 Prints version of installed packages\n"
	    " getsystemdir                         Prints the system dulge.d directory\n"
	    );
	exit(fail ? EXIT_FAILURE : EXIT_SUCCESS);
}

static char *
fname(char *url)
{
	char *filename;

	if ((filename = strrchr(url, '>'))) {
		*filename = '\0';
	} else {
		filename = strrchr(url, '/');
	}
	if (filename == NULL)
		return NULL;
	return filename + 1;
}

int
main(int argc, char **argv)
{
	dulge_dictionary_t dict;
	struct dulge_handle xh;
	struct xferstat xfer;
	const char *version, *rootdir = NULL, *confdir = NULL;
	char pkgname[DULGE_NAME_SIZE], *filename;
	int flags = 0, c, rv = 0, i = 0;
	const struct option longopts[] = {
		{ "config", required_argument, NULL, 'C' },
		{ "debug", no_argument, NULL, 'd' },
		{ "rootdir", required_argument, NULL, 'r' },
		{ "verbose", no_argument, NULL, 'v' },
		{ "version", no_argument, NULL, 'V' },
		{ NULL, 0, NULL, 0 }
	};

	while ((c = getopt_long(argc, argv, "C:dr:vV", longopts, NULL)) != -1) {
		switch (c) {
		case 'h':
			usage(false);
			/* NOTREACHED */
		case 'C':
			confdir = optarg;
			break;
		case 'r':
			/* To specify the root directory */
			rootdir = optarg;
			break;
		case 'd':
			flags |= DULGE_FLAG_DEBUG;
			break;
		case 'v':
			flags |= DULGE_FLAG_VERBOSE;
			break;
		case 'V':
			printf("%s\n", DULGE_RELVER);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		case '?':
		default:
			usage(true);
			/* NOTREACHED */
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 1) {
		usage(true);
		/* NOTREACHED */
	}

	memset(&xh, 0, sizeof(xh));

	if ((strcmp(argv[0], "version") == 0) ||
	    (strcmp(argv[0], "real-version") == 0) ||
	    (strcmp(argv[0], "arch") == 0) ||
	    (strcmp(argv[0], "fetch") == 0) ||
	    (strcmp(argv[0], "getsystemdir") == 0)) {
		/*
		* Initialize libdulge.
		*/
		xh.fetch_cb = fetch_file_progress_cb;
		xh.fetch_cb_data = &xfer;
		xh.flags = flags;
		if (rootdir)
			dulge_strlcpy(xh.rootdir, rootdir, sizeof(xh.rootdir));
		if (confdir)
			dulge_strlcpy(xh.confdir, confdir, sizeof(xh.confdir));
		if ((rv = dulge_init(&xh)) != 0) {
			dulge_error_printf("dulge-uhelper: failed to "
			    "initialize libdulge: %s.\n", strerror(rv));
			exit(EXIT_FAILURE);
		}
	}

	if (strcmp(argv[0], "version") == 0) {
		/* Prints version of installed packages */
		if (argc < 2) {
			usage(true);
			/* NOTREACHED */
		}

		for (i = 1; i < argc; i++) {
			if ((((dict = dulge_pkgdb_get_pkg(&xh, argv[i])) == NULL)) &&
				(((dict = dulge_pkgdb_get_virtualpkg(&xh, argv[i])) == NULL))) {
				dulge_error_printf("Could not find package '%s'\n", argv[i]);
				rv = 1;
			} else {
				dulge_dictionary_get_cstring_nocopy(dict, "pkgver", &version);
				printf("%s\n", dulge_pkg_version(version));
			}
		}
	} else if (strcmp(argv[0], "real-version") == 0) {
		/* Prints version of installed real packages, not virtual */
		if (argc < 2) {
			usage(true);
			/* NOTREACHED */
		}

		for (i = 1; i < argc; i++) {
			if ((dict = dulge_pkgdb_get_pkg(&xh, argv[i])) == NULL) {
				dulge_error_printf("Could not find package '%s'\n", argv[i]);
				rv = 1;
			} else {
				dulge_dictionary_get_cstring_nocopy(dict, "pkgver", &version);
				printf("%s\n", dulge_pkg_version(version));
			}
		}
	} else if (strcmp(argv[0], "getpkgversion") == 0) {
		/* Returns the version of pkg strings */
		if (argc < 2) {
			usage(true);
			/* NOTREACHED */
		}

		for (i = 1; i < argc; i++) {
			version = dulge_pkg_version(argv[i]);
			if (version == NULL) {
				dulge_error_printf(
					"Invalid string '%s', expected <string>-<version>_<revision>\n", argv[i]);
				rv = 1;
			} else {
				printf("%s\n", version);
			}
		}
	} else if (strcmp(argv[0], "getpkgname") == 0) {
		/* Returns the name of pkg strings */
		if (argc < 2) {
			usage(true);
			/* NOTREACHED */
		}

		for (i = 1; i < argc; i++) {
			if (!dulge_pkg_name(pkgname, sizeof(pkgname), argv[i])) {
				dulge_error_printf(
					"Invalid string '%s', expected <string>-<version>_<revision>\n", argv[i]);
				rv = 1;
			} else {
				printf("%s\n", pkgname);
			}
		}
	} else if (strcmp(argv[0], "getpkgrevision") == 0) {
		/* Returns the revision of pkg strings */
		if (argc < 2) {
			usage(true);
			/* NOTREACHED */
		}

		for (i = 1; i < argc; i++) {
			version = dulge_pkg_revision(argv[1]);
			if (version == NULL) {
				rv = 1;
			} else {
				printf("%s\n", version);
			}
		}
	} else if (strcmp(argv[0], "getpkgdepname") == 0) {
		/* Returns the pkgname of dependencies */
		if (argc < 2) {
			usage(true);
			/* NOTREACHED */
		}

		for (i = 1; i < argc; i++) {
			if (!dulge_pkgpattern_name(pkgname, sizeof(pkgname), argv[i])) {
				dulge_error_printf("Invalid string '%s', expected <string><comparator><version>\n", argv[i]);
				rv = 1;
			} else {
				printf("%s\n", pkgname);
			}
		}
	} else if (strcmp(argv[0], "getpkgdepversion") == 0) {
		/* returns the version of package pattern dependencies */
		if (argc < 2) {
			usage(true);
			/* NOTREACHED */
		}

		for (i = 1; i < argc; i++) {
			version = dulge_pkgpattern_version(argv[i]);
			if (version == NULL) {
				dulge_error_printf("Invalid string '%s', expected <string><comparator><version>\n", argv[i]);
				rv = 1;
			} else {
				printf("%s\n", version);
			}
		}
	} else if (strcmp(argv[0], "getname") == 0) {
		/* returns the name of a pkg strings or pkg patterns */
		if (argc < 2) {
			usage(true);
			/* NOTREACHED */
		}

		for (i = 1; i < argc; i++) {
			if (dulge_pkgpattern_name(pkgname, sizeof(pkgname), argv[i]) ||
				dulge_pkg_name(pkgname, sizeof(pkgname), argv[i])) {
				printf("%s\n", pkgname);
			} else {
				dulge_error_printf(
					"Invalid string '%s', expected <string><comparator><version> "
					"or <string>-<version>_<revision>\n", argv[i]);
				rv = 1;
			}
		}
	} else if (strcmp(argv[0], "getversion") == 0) {
		/* returns the version of a pkg strings or pkg patterns */
		if (argc < 2) {
			usage(true);
			/* NOTREACHED */
		}

		for (i = 1; i < argc; i++) {
			version = dulge_pkgpattern_version(argv[i]);
			if (version == NULL) {
				version = dulge_pkg_version(argv[i]);
				if (version == NULL) {
					dulge_error_printf(
						"Invalid string '%s', expected <string><comparator><version> "
						"or <string>-<version>_<revision>\n", argv[i]);
					rv = 1;
					continue;
				}
			}
			printf("%s\n", version);
		}
	} else if (strcmp(argv[0], "binpkgver") == 0) {
		/* Returns the pkgver of binpkg strings */
		if (argc < 2) {
			usage(true);
			/* NOTREACHED */
		}

		for (i = 1; i < argc; i++) {
			version = dulge_binpkg_pkgver(argv[i]);
			if (version == NULL) {
				dulge_error_printf(
					"Invalid string '%s', expected <pkgname>-<version>_<revision>.<arch>.dulge\n", argv[i]);
				rv = 1;
			} else {
				printf("%s\n", version);
			}
		}
	} else if (strcmp(argv[0], "binpkgarch") == 0) {
		/* Returns the arch of binpkg strings */
		if (argc < 2) {
			usage(true);
			/* NOTREACHED */
		}

		for (i = 1; i < argc; i++) {
			version = dulge_binpkg_arch(argv[i]);
			if (version == NULL) {
				dulge_error_printf(
					"Invalid string '%s', expected <pkgname>-<version>_<revision>.<arch>.dulge\n", argv[i]);
				rv = 1;
			} else {
				printf("%s\n", version);
			}
		}
	} else if (strcmp(argv[0], "pkgmatch") == 0) {
		/* Matches a pkg with a pattern */
		if (argc != 3) {
			usage(true);
			/* NOTREACHED */
		}
		rv = dulge_pkgpattern_match(argv[1], argv[2]);
		if (rv >= 0) {
			if (flags & DULGE_FLAG_VERBOSE) {
				fprintf(stderr, "%s %s %s\n",
					argv[1],
					(rv == 1) ? "matches" : "does not match",
					argv[2]);
			}
		} else if (flags & DULGE_FLAG_VERBOSE) {
			dulge_error_printf("%s: not a pattern\n", argv[2]);
		}
		exit(rv);
	} else if (strcmp(argv[0], "cmpver") == 0) {
		/* Compare two version strings, installed vs required */
		if (argc != 3) {
			usage(true);
			/* NOTREACHED */
		}

		rv = dulge_cmpver(argv[1], argv[2]);
		if (flags & DULGE_FLAG_VERBOSE) {
			fprintf(stderr, "%s %s %s\n",
				argv[1],
				(rv == 1) ? ">" : ((rv == 0) ? "=" : "<"),
				argv[2]);
		}
		exit(rv);
	} else if (strcmp(argv[0], "arch") == 0) {
		/* returns the dulge native arch */
		if (argc != 1) {
			usage(true);
			/* NOTREACHED */
		}

		if (xh.native_arch[0] && xh.target_arch && strcmp(xh.native_arch, xh.target_arch)) {
			printf("%s\n", xh.target_arch);
		} else {
			printf("%s\n", xh.native_arch);
		}
	} else if (strcmp(argv[0], "getsystemdir") == 0) {
		/* returns the dulge system directory (<sharedir>/dulge.d) */
		if (argc != 1) {
			usage(true);
			/* NOTREACHED */
		}

		printf("%s\n", DULGE_SYSDEFCONF_PATH);
	} else if (strcmp(argv[0], "digest") == 0) {
		char sha256[DULGE_SHA256_SIZE];

		/* Prints SHA256 hashes for specified files */
		if (argc < 2) {
			usage(true);
			/* NOTREACHED */
		}

		for (i = 1; i < argc; i++) {
			if (!dulge_file_sha256(sha256, sizeof sha256, argv[i])) {
				dulge_error_printf(
				    "couldn't get hash for %s (%s)\n",
				    argv[i], strerror(errno));
				exit(EXIT_FAILURE);
			}
			printf("%s\n", sha256);
		}
	} else if (strcmp(argv[0], "fetch") == 0) {
		/* Fetch a file from specified URL */
		if (argc < 2) {
			usage(true);
			/* NOTREACHED */
		}

		for (i = 1; i < argc; i++) {
			filename = fname(argv[i]);
			rv = dulge_fetch_file_dest(&xh, argv[i], filename, "v");

			if (rv == -1) {
				dulge_error_printf("%s: %s\n", argv[i],
				    dulge_fetch_error_string());
			} else if (rv == 0) {
				printf("%s: file is identical with remote.\n", argv[i]);
			} else {
				rv = 0;
			}
		}
	} else {
		usage(true);
	}

	exit(rv ? EXIT_FAILURE : EXIT_SUCCESS);
}
