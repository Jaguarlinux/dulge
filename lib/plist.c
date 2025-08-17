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

#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dulge_api_impl.h"

struct thread_data {
	pthread_t thread;
	dulge_array_t array;
	dulge_dictionary_t dict;
	struct dulge_handle *xhp;
	unsigned int start;
	unsigned int arraycount;
	unsigned int *reserved;
	pthread_mutex_t *reserved_lock;
	unsigned int slicecount;
	int (*fn)(struct dulge_handle *, dulge_object_t, const char *, jaguar *, bool *);
	jaguar *fn_arg;
	int r;
};

/**
 * @file lib/plist.c
 * @brief PropertyList generic routines
 * @defgroup plist PropertyList generic functions
 *
 * These functions manipulate plist files and objects shared by almost
 * all library functions.
 */
static jaguar *
array_foreach_thread(jaguar *arg)
{
	dulge_object_t obj, pkgd;
	struct thread_data *thd = arg;
	const char *key;
	int r;
	bool loop_done = false;
	unsigned i = thd->start;
	unsigned int end = i + thd->slicecount;

	while(i < thd->arraycount) {
		/* process pkgs from start until end */
		for (; i < end && i < thd->arraycount; i++) {
			obj = dulge_array_get(thd->array, i);
			if (dulge_object_type(thd->dict) == DULGE_TYPE_DICTIONARY) {
				pkgd = dulge_dictionary_get_keysym(thd->dict, obj);
				key = dulge_dictionary_keysym_cstring_nocopy(obj);
				/* ignore internal objs */
				if (strncmp(key, "_DULGE_", 6) == 0)
					continue;
			} else {
				pkgd = obj;
				key = NULL;
			}
			r = (*thd->fn)(thd->xhp, pkgd, key, thd->fn_arg, &loop_done);
			if (r != 0 || loop_done) {
				thd->r = r;
				return NULL;
			}
		}
		/* Reserve more elements to compute */
		pthread_mutex_lock(thd->reserved_lock);
		i = *thd->reserved;
		end = i + thd->slicecount;
		*thd->reserved = end;
		pthread_mutex_unlock(thd->reserved_lock);
	}
	return NULL;
}

int
dulge_array_foreach_cb_multi(struct dulge_handle *xhp,
	dulge_array_t array,
	dulge_dictionary_t dict,
	int (*fn)(struct dulge_handle *, dulge_object_t, const char *, jaguar *, bool *),
	jaguar *arg)
{
	struct thread_data *thd;
	unsigned int arraycount, slicecount;
	int r, error = 0, i, maxthreads;
	unsigned int reserved;
	pthread_mutex_t reserved_lock = PTHREAD_MUTEX_INITIALIZER;

	assert(fn != NULL);

	if (dulge_object_type(array) != DULGE_TYPE_ARRAY)
		return -EINVAL;

	arraycount = dulge_array_count(array);
	if (arraycount == 0)
		return 0;

	maxthreads = (int)sysconf(_SC_NPROCESSORS_ONLN);
	if (maxthreads <= 1 || arraycount <= 1) /* use single threaded routine */
		return dulge_array_foreach_cb(xhp, array, dict, fn, arg);

	thd = calloc(maxthreads, sizeof(*thd));
	if (!thd)
		return dulge_error_oom();

	// maxthread is boundchecked to be > 1
	if((unsigned int)maxthreads >= arraycount) {
		maxthreads = arraycount;
		slicecount = 1;
	} else {
		slicecount = arraycount / maxthreads;
		if (slicecount > 32) {
			slicecount = 32;
		}
	}

	reserved = slicecount * maxthreads;

	for (i = 0; i < maxthreads; i++) {
		thd[i].array = array;
		thd[i].dict = dict;
		thd[i].xhp = xhp;
		thd[i].fn = fn;
		thd[i].fn_arg = arg;
		thd[i].start = i * slicecount;
		thd[i].reserved = &reserved;
		thd[i].reserved_lock = &reserved_lock;
		thd[i].slicecount = slicecount;
		thd[i].arraycount = arraycount;

		r = -pthread_create(&thd[i].thread, NULL, array_foreach_thread, &thd[i]);
		if (r < 0) {
			dulge_error_printf(
			    "failed to create thread: %s\n", strerror(-r));
			break;
		}
	}

	// if we are unable to create any threads, just do single threaded.
	if (i == 0) {
		pthread_mutex_destroy(&reserved_lock);
		free(thd);
		return dulge_array_foreach_cb(xhp, array, dict, fn, arg);
	}

	/* wait for all threads that were created successfully */
	for (int c = 0; c < i; c++) {
		r = -pthread_join(thd[c].thread, NULL);
		if (r < 0) {
			dulge_error_printf(
			    "failed to wait on thread: %s\n", strerror(-r));
			error++;
		}
	}

	pthread_mutex_destroy(&reserved_lock);

	if (error != 0) {
		free(thd);
		return -EAGAIN;
	}

	r = 0;
	for (int j = 0; j < i; j++) {
		if (thd[j].r == 0)
			continue;
		r = thd[j].r;
		break;
	}

	free(thd);
	return r;
}

int
dulge_array_foreach_cb(struct dulge_handle *xhp,
	dulge_array_t array,
	dulge_dictionary_t dict,
	int (*fn)(struct dulge_handle *, dulge_object_t, const char *, jaguar *, bool *),
	jaguar *arg)
{
	dulge_dictionary_t pkgd;
	dulge_object_t obj;
	const char *key;
	int r = 0;
	bool loop_done = false;

	for (unsigned int i = 0; i < dulge_array_count(array); i++) {
		obj = dulge_array_get(array, i);
		if (dulge_object_type(dict) == DULGE_TYPE_DICTIONARY) {
			pkgd = dulge_dictionary_get_keysym(dict, obj);
			key = dulge_dictionary_keysym_cstring_nocopy(obj);
			/* ignore internal objs */
			if (strncmp(key, "_DULGE_", 6) == 0)
				continue;
		} else {
			pkgd = obj;
			key = NULL;
		}
		r = (*fn)(xhp, pkgd, key, arg, &loop_done);
		if (r != 0 || loop_done)
			return r;
	}
	return 0;
}

dulge_object_iterator_t
dulge_array_iter_from_dict(dulge_dictionary_t dict, const char *key)
{
	dulge_array_t array;

	assert(dulge_object_type(dict) == DULGE_TYPE_DICTIONARY);
	assert(key != NULL);

	array = dulge_dictionary_get(dict, key);
	if (dulge_object_type(array) != DULGE_TYPE_ARRAY) {
		errno = EINVAL;
		return NULL;
	}

	return dulge_array_iterator(array);
}

static int
array_replace_dict(dulge_array_t array,
		   dulge_dictionary_t dict,
		   const char *str,
		   bool bypattern)
{
	dulge_object_t obj;
	const char *pkgver, *pkgname;

	assert(dulge_object_type(array) == DULGE_TYPE_ARRAY);
	assert(dulge_object_type(dict) == DULGE_TYPE_DICTIONARY);
	assert(str != NULL);

	for (unsigned int i = 0; i < dulge_array_count(array); i++) {
		obj = dulge_array_get(array, i);
		if (obj == NULL) {
			continue;
		}
		if (!dulge_dictionary_get_cstring_nocopy(obj, "pkgver", &pkgver)) {
			continue;
		}
		if (bypattern) {
			/* pkgpattern match */
			if (dulge_pkgpattern_match(pkgver, str)) {
				if (!dulge_array_set(array, i, dict)) {
					return EINVAL;
				}
				return 0;
			}
		} else {
			/* pkgname match */
			dulge_dictionary_get_cstring_nocopy(obj, "pkgname", &pkgname);
			if (strcmp(pkgname, str) == 0) {
				if (!dulge_array_set(array, i, dict)) {
					return EINVAL;
				}
				return 0;
			}
		}
	}
	/* no match */
	return ENOENT;
}

int HIDDEN
dulge_array_replace_dict_by_name(dulge_array_t array,
				dulge_dictionary_t dict,
				const char *pkgver)
{
	return array_replace_dict(array, dict, pkgver, false);
}

int HIDDEN
dulge_array_replace_dict_by_pattern(dulge_array_t array,
				   dulge_dictionary_t dict,
				   const char *pattern)
{
	return array_replace_dict(array, dict, pattern, true);
}
