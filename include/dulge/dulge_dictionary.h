/*	$NetBSD: prop_dictionary.h,v 1.9 2008/04/28 20:22:51 martin Exp $	*/

/*-
 * Copyright (c) 2006 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe.
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
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _DULGE_DICTIONARY_H_
#define	_DULGE_DICTIONARY_H_

#include <stdint.h>
#include <dulge/dulge_object.h>
#include <dulge/dulge_array.h>

typedef struct _prop_dictionary *dulge_dictionary_t;
typedef struct _prop_dictionary_keysym *dulge_dictionary_keysym_t;

#ifdef __cplusplus
extern "C" {
#endif

dulge_dictionary_t dulge_dictionary_create(void);
dulge_dictionary_t dulge_dictionary_create_with_capacity(unsigned int);

dulge_dictionary_t dulge_dictionary_copy(dulge_dictionary_t);
dulge_dictionary_t dulge_dictionary_copy_mutable(dulge_dictionary_t);

unsigned int	dulge_dictionary_count(dulge_dictionary_t);
bool		dulge_dictionary_ensure_capacity(dulge_dictionary_t,
						unsigned int);

void		dulge_dictionary_make_immutable(dulge_dictionary_t);

dulge_object_iterator_t dulge_dictionary_iterator(dulge_dictionary_t);
dulge_array_t	dulge_dictionary_all_keys(dulge_dictionary_t);

dulge_object_t	dulge_dictionary_get(dulge_dictionary_t, const char *);
bool		dulge_dictionary_set(dulge_dictionary_t, const char *,
				    dulge_object_t);
void		dulge_dictionary_remove(dulge_dictionary_t, const char *);

dulge_object_t	dulge_dictionary_get_keysym(dulge_dictionary_t,
					   dulge_dictionary_keysym_t);
bool		dulge_dictionary_set_keysym(dulge_dictionary_t,
					   dulge_dictionary_keysym_t,
					   dulge_object_t);
void		dulge_dictionary_remove_keysym(dulge_dictionary_t,
					      dulge_dictionary_keysym_t);

bool		dulge_dictionary_equals(dulge_dictionary_t, dulge_dictionary_t);

char *		dulge_dictionary_externalize(dulge_dictionary_t);
dulge_dictionary_t dulge_dictionary_internalize(const char *);

bool		dulge_dictionary_externalize_to_file(dulge_dictionary_t,
						    const char *);
bool		dulge_dictionary_externalize_to_zfile(dulge_dictionary_t,
						     const char *);
dulge_dictionary_t dulge_dictionary_internalize_from_file(const char *);
dulge_dictionary_t dulge_dictionary_internalize_from_zfile(const char *);

const char *	dulge_dictionary_keysym_cstring_nocopy(dulge_dictionary_keysym_t);

bool		dulge_dictionary_keysym_equals(dulge_dictionary_keysym_t,
					      dulge_dictionary_keysym_t);

/*
 * Utility routines to make it more convenient to work with values
 * stored in dictionaries.
 */
bool		dulge_dictionary_get_dict(dulge_dictionary_t, const char *,
					 dulge_dictionary_t *);
bool		dulge_dictionary_get_bool(dulge_dictionary_t, const char *,
					 bool *);
bool		dulge_dictionary_set_bool(dulge_dictionary_t, const char *,
					 bool);

bool		dulge_dictionary_get_int8(dulge_dictionary_t, const char *,
					 int8_t *);
bool		dulge_dictionary_get_uint8(dulge_dictionary_t, const char *,
					  uint8_t *);
bool		dulge_dictionary_set_int8(dulge_dictionary_t, const char *,
					 int8_t);
bool		dulge_dictionary_set_uint8(dulge_dictionary_t, const char *,
					  uint8_t);

bool		dulge_dictionary_get_int16(dulge_dictionary_t, const char *,
					  int16_t *);
bool		dulge_dictionary_get_uint16(dulge_dictionary_t, const char *,
					   uint16_t *);
bool		dulge_dictionary_set_int16(dulge_dictionary_t, const char *,
					  int16_t);
bool		dulge_dictionary_set_uint16(dulge_dictionary_t, const char *,
					   uint16_t);

bool		dulge_dictionary_get_int32(dulge_dictionary_t, const char *,
					  int32_t *);
bool		dulge_dictionary_get_uint32(dulge_dictionary_t, const char *,
					   uint32_t *);
bool		dulge_dictionary_set_int32(dulge_dictionary_t, const char *,
					  int32_t);
bool		dulge_dictionary_set_uint32(dulge_dictionary_t, const char *,
					   uint32_t);

bool		dulge_dictionary_get_int64(dulge_dictionary_t, const char *,
					  int64_t *);
bool		dulge_dictionary_get_uint64(dulge_dictionary_t, const char *,
					   uint64_t *);
bool		dulge_dictionary_set_int64(dulge_dictionary_t, const char *,
					  int64_t);
bool		dulge_dictionary_set_uint64(dulge_dictionary_t, const char *,
					   uint64_t);

bool		dulge_dictionary_get_cstring(dulge_dictionary_t, const char *,
					     char **);
bool		dulge_dictionary_set_cstring(dulge_dictionary_t, const char *,
					    const char *);

bool		dulge_dictionary_get_cstring_nocopy(dulge_dictionary_t,
						   const char *,
						   const char **);
bool		dulge_dictionary_set_cstring_nocopy(dulge_dictionary_t,
						   const char *,
						   const char *);
bool		dulge_dictionary_set_and_rel(dulge_dictionary_t,
					    const char *,
					    dulge_object_t);

#ifdef __cplusplus
}
#endif

#endif /* _DULGE_DICTIONARY_H_ */
