/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/




#ifndef ALPM_PKGHASH_H
#define ALPM_PKGHASH_H

#include <stdlib.h>

#include "alpm.h"
#include "alpm_list.h"


/**
 * @brief A hash table for holding alpm_pkg_t objects.
 *
 * A combination of a hash table and a list, allowing for fast look-up
 * by package name but also iteration over the packages.
 */
struct _alpm_pkghash_t {
	/** data held by the hash table */
	alpm_list_t **hash_table;
	/** head node of the hash table data in normal list format */
	alpm_list_t *list;
	/** number of buckets in hash table */
	unsigned int buckets;
	/** number of entries in hash table */
	unsigned int entries;
	/** max number of entries before a resize is needed */
	unsigned int limit;
};

typedef struct _alpm_pkghash_t alpm_pkghash_t;

alpm_pkghash_t *_alpm_pkghash_create(unsigned int size);

alpm_pkghash_t *_alpm_pkghash_add(alpm_pkghash_t **hash, alpm_pkg_t *pkg);
alpm_pkghash_t *_alpm_pkghash_add_sorted(alpm_pkghash_t **hash, alpm_pkg_t *pkg);
alpm_pkghash_t *_alpm_pkghash_remove(alpm_pkghash_t *hash, alpm_pkg_t *pkg, alpm_pkg_t **data);

void _alpm_pkghash_free(alpm_pkghash_t *hash);

alpm_pkg_t *_alpm_pkghash_find(alpm_pkghash_t *hash, const char *name);

#endif /* ALPM_PKGHASH_H */
