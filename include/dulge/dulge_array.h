/*     $NetBSD: prop_array.h,v 1.8 2008/09/11 13:15:13 haad Exp $    */

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

#ifndef _DULGE_ARRAY_H_
#define	_DULGE_ARRAY_H_

#include <stdint.h>
#include <dulge/dulge_object.h>

typedef struct _prop_array *dulge_array_t;

#ifdef __cplusplus
extern "C" {
#endif

dulge_array_t	dulge_array_create(void);
dulge_array_t	dulge_array_create_with_capacity(unsigned int);

dulge_array_t	dulge_array_copy(dulge_array_t);
dulge_array_t	dulge_array_copy_mutable(dulge_array_t);

unsigned int	dulge_array_capacity(dulge_array_t);
unsigned int	dulge_array_count(dulge_array_t);
bool		dulge_array_ensure_capacity(dulge_array_t, unsigned int);

void		dulge_array_make_immutable(dulge_array_t);
bool		dulge_array_mutable(dulge_array_t);

dulge_object_iterator_t dulge_array_iterator(dulge_array_t);

dulge_object_t	dulge_array_get(dulge_array_t, unsigned int);
bool		dulge_array_set(dulge_array_t, unsigned int, dulge_object_t);
bool		dulge_array_add(dulge_array_t, dulge_object_t);
bool		dulge_array_add_first(dulge_array_t, dulge_object_t);
void		dulge_array_remove(dulge_array_t, unsigned int);

bool		dulge_array_equals(dulge_array_t, dulge_array_t);

char *		dulge_array_externalize(dulge_array_t);
dulge_array_t	dulge_array_internalize(const char *);

bool		dulge_array_externalize_to_file(dulge_array_t, const char *);
bool		dulge_array_externalize_to_zfile(dulge_array_t, const char *);
dulge_array_t	dulge_array_internalize_from_file(const char *);
dulge_array_t	dulge_array_internalize_from_zfile(const char *);

/*
 * Utility routines to make it more convenient to work with values
 * stored in dictionaries.
 */
bool		dulge_array_get_bool(dulge_array_t, unsigned int,
					 bool *);
bool		dulge_array_set_bool(dulge_array_t, unsigned int,
					 bool);

bool		dulge_array_get_int8(dulge_array_t, unsigned int,
					 int8_t *);
bool		dulge_array_get_uint8(dulge_array_t, unsigned int,
					  uint8_t *);
bool		dulge_array_set_int8(dulge_array_t, unsigned int,
					 int8_t);
bool		dulge_array_set_uint8(dulge_array_t, unsigned int,
					  uint8_t);

bool		dulge_array_get_int16(dulge_array_t, unsigned int,
					  int16_t *);
bool		dulge_array_get_uint16(dulge_array_t, unsigned int,
					   uint16_t *);
bool		dulge_array_set_int16(dulge_array_t, unsigned int,
					  int16_t);
bool		dulge_array_set_uint16(dulge_array_t, unsigned int,
					   uint16_t);

bool		dulge_array_get_int32(dulge_array_t, unsigned int,
					  int32_t *);
bool		dulge_array_get_uint32(dulge_array_t, unsigned int,
					   uint32_t *);
bool		dulge_array_set_int32(dulge_array_t, unsigned int,
					  int32_t);
bool		dulge_array_set_uint32(dulge_array_t, unsigned int,
					   uint32_t);

bool		dulge_array_get_int64(dulge_array_t, unsigned int,
					  int64_t *);
bool		dulge_array_get_uint64(dulge_array_t, unsigned int,
					   uint64_t *);
bool		dulge_array_set_int64(dulge_array_t, unsigned int,
					  int64_t);
bool		dulge_array_set_uint64(dulge_array_t, unsigned int,
					   uint64_t);

bool		dulge_array_add_int8(dulge_array_t, int8_t);
bool		dulge_array_add_uint8(dulge_array_t, uint8_t);

bool		dulge_array_add_int16(dulge_array_t, int16_t);
bool		dulge_array_add_uint16(dulge_array_t, uint16_t);

bool		dulge_array_add_int32(dulge_array_t, int32_t);
bool		dulge_array_add_uint32(dulge_array_t, uint32_t);

bool		dulge_array_add_int64(dulge_array_t, int64_t);
bool		dulge_array_add_uint64(dulge_array_t, uint64_t);

bool		dulge_array_get_cstring(dulge_array_t, unsigned int,
					     char **);
bool		dulge_array_set_cstring(dulge_array_t, unsigned int,
					    const char *);
bool		dulge_array_add_cstring(dulge_array_t, const char *);
bool		dulge_array_add_cstring_nocopy(dulge_array_t,
						const char *);
bool		dulge_array_get_cstring_nocopy(dulge_array_t,
                                                   unsigned int,
						   const char **);
bool		dulge_array_set_cstring_nocopy(dulge_array_t,
						   unsigned int,
						   const char *);
bool		dulge_array_add_and_rel(dulge_array_t, dulge_object_t);

#ifdef __cplusplus
}
#endif

#endif /* _DULGE_ARRAY_H_ */
