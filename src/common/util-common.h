/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/










#ifndef PM_UTIL_COMMON_H
#define PM_UTIL_COMMON_H

#include <stdio.h>
#include <sys/stat.h> /* struct stat */

char *hex_representation(const unsigned char *bytes, size_t size);
const char *mbasename(const char *path);
char *mdirname(const char *path);

int llstat(char *path, struct stat *buf);

char *safe_fgets(char *s, int size, FILE *stream);

void wordsplit_free(char **ws);
char **wordsplit(const char *str);

size_t strtrim(char *str);

#ifndef HAVE_STRNDUP
char *strndup(const char *s, size_t n);
#endif

#define ARRAYSIZE(a) (sizeof (a) / sizeof (a[0]))

#endif /* PM_UTIL_COMMON_H */
