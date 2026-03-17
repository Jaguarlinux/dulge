/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/




#include <stdlib.h>
#include <stdio.h> /* printf */
#include <string.h>

/* forward declaration, comes from version.o in libalpm source that is linked
 * in directly so we don't have any library deps */
int alpm_pkg_vercmp(const char *a, const char *b);

static void usage(void)
{
	fprintf(stderr, "dulge-dulge-dulge-vercmp (dulge) v" PACKAGE_VERSION "\n\n"
		"Compare package version numbers using dulge's version comparison logic.\n\n"
		"Usage: dulge-dulge-dulge-vercmp <ver1> <ver2>\n\n"
		"Output values:\n"
		"  < 0 : if ver1 < ver2\n"
		"    0 : if ver1 == ver2\n"
		"  > 0 : if ver1 > ver2\n");
}

int main(int argc, char *argv[])
{
	int ret;

	if(argc == 1) {
		usage();
		return 2;
	}
	if(argc > 1 &&
			(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
		usage();
		return 0;
	}
	if(argc != 3) {
		fprintf(stderr, "error: %d argument(s) specified\n\n"
			"Usage: dulge-dulge-dulge-vercmp <ver1> <ver2>\n", argc - 1);
		return EXIT_FAILURE;
	}

	ret = alpm_pkg_vercmp(argv[1], argv[2]);
	printf("%d\n", ret);
	return EXIT_SUCCESS;
}
