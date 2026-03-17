/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/




/* These functions are borrowed from libcurl's lib/rawstr.c with minor
 * modifications to style and naming. Curl_raw_equal and Curl_raw_nequal are
 * further modified to be true cmp style functions, returning negative, zero,
 * or positive. */

#include <stdlib.h>

#include "util.h"

/* Portable, consistent toupper (remember EBCDIC). Do not use toupper() because
	 its behavior is altered by the current locale. */
static char raw_toupper(char in)
{
	switch(in) {
	case 'a':
		return 'A';
	case 'b':
		return 'B';
	case 'c':
		return 'C';
	case 'd':
		return 'D';
	case 'e':
		return 'E';
	case 'f':
		return 'F';
	case 'g':
		return 'G';
	case 'h':
		return 'H';
	case 'i':
		return 'I';
	case 'j':
		return 'J';
	case 'k':
		return 'K';
	case 'l':
		return 'L';
	case 'm':
		return 'M';
	case 'n':
		return 'N';
	case 'o':
		return 'O';
	case 'p':
		return 'P';
	case 'q':
		return 'Q';
	case 'r':
		return 'R';
	case 's':
		return 'S';
	case 't':
		return 'T';
	case 'u':
		return 'U';
	case 'v':
		return 'V';
	case 'w':
		return 'W';
	case 'x':
		return 'X';
	case 'y':
		return 'Y';
	case 'z':
		return 'Z';
	}
	return in;
}

/*
 * _alpm_raw_cmp() is for doing "raw" case insensitive strings. This is meant
 * to be locale independent and only compare strings we know are safe for
 * this.  See http://daniel.haxx.se/blog/2008/10/15/strcasecmp-in-turkish/ for
 * some further explanation to why this function is necessary.
 *
 * The function is capable of comparing a-z case insensitively even for
 * non-ascii.
 */

int _alpm_raw_cmp(const char *first, const char *second)
{
	while(*first && *second) {
		if(raw_toupper(*first) != raw_toupper(*second)) {
			/* get out of the loop as soon as they don't match */
			break;
		}
		first++;
		second++;
	}
	/* we do the comparison here (possibly again), just to make sure that if the
		 loop above is skipped because one of the strings reached zero, we must not
		 return this as a successful match */
	return (raw_toupper(*first) - raw_toupper(*second));
}

int _alpm_raw_ncmp(const char *first, const char *second, size_t max)
{
	while(*first && *second && max) {
		if(raw_toupper(*first) != raw_toupper(*second)) {
			break;
		}
		max--;
		first++;
		second++;
	}
	if(0 == max) {
		/* they are equal this far */
		return 0;
	}

	return (raw_toupper(*first) - raw_toupper(*second));
}
