/*-
 * Copyright (c) 2013-2015 Juan Romero Pardines.
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
#include <errno.h>

#include "dulge_api_impl.h"
#include <prop/proplib.h>

/* prop_array */

dulge_array_t
dulge_array_create(void)
{
	return prop_array_create();
}

dulge_array_t
dulge_array_create_with_capacity(unsigned int capacity)
{
	return prop_array_create_with_capacity(capacity);
}

dulge_array_t
dulge_array_copy(dulge_array_t a)
{
	return prop_array_copy(a);
}

dulge_array_t
dulge_array_copy_mutable(dulge_array_t a)
{
	return prop_array_copy_mutable(a);
}

unsigned int
dulge_array_capacity(dulge_array_t a)
{
	return prop_array_capacity(a);
}

unsigned int
dulge_array_count(dulge_array_t a)
{
	return prop_array_count(a);
}

bool
dulge_array_ensure_capacity(dulge_array_t a, unsigned int i)
{
	return prop_array_ensure_capacity(a, i);
}

void
dulge_array_make_immutable(dulge_array_t a)
{
	prop_array_make_immutable(a);
}

bool
dulge_array_mutable(dulge_array_t a)
{
	return prop_array_mutable(a);
}

dulge_object_iterator_t
dulge_array_iterator(dulge_array_t a)
{
	return prop_array_iterator(a);
}

dulge_object_t
dulge_array_get(dulge_array_t a, unsigned int i)
{
	return prop_array_get(a, i);
}

bool
dulge_array_set(dulge_array_t a, unsigned int i, dulge_object_t obj)
{
	return prop_array_set(a, i, obj);
}

bool
dulge_array_add(dulge_array_t a, dulge_object_t obj)
{
	return prop_array_add(a, obj);
}

bool
dulge_array_add_first(dulge_array_t a, dulge_object_t obj)
{
	return prop_array_add_first(a, obj);
}

void
dulge_array_remove(dulge_array_t a, unsigned int i)
{
	prop_array_remove(a, i);
}

bool
dulge_array_equals(dulge_array_t a, dulge_array_t b)
{
	return prop_array_equals(a, b);
}

char *
dulge_array_externalize(dulge_array_t a)
{
	return prop_array_externalize(a);
}

dulge_array_t
dulge_array_internalize(const char *s)
{
	return prop_array_internalize(s);
}

bool
dulge_array_externalize_to_file(dulge_array_t a, const char *s)
{
	return prop_array_externalize_to_file(a, s);
}

bool
dulge_array_externalize_to_zfile(dulge_array_t a, const char *s)
{
	return prop_array_externalize_to_zfile(a, s);
}

dulge_array_t
dulge_array_internalize_from_file(const char *s)
{
	return prop_array_internalize_from_file(s);
}

dulge_array_t
dulge_array_internalize_from_zfile(const char *s)
{
	return prop_array_internalize_from_zfile(s);
}

/*
 * Utility routines to make it more convenient to work with values
 * stored in dictionaries.
 */
bool
dulge_array_get_bool(dulge_array_t a, unsigned int i, bool *b)
{
	return prop_array_get_bool(a, i, b);
}

bool
dulge_array_set_bool(dulge_array_t a, unsigned int i, bool b)
{
	return prop_array_set_bool(a, i, b);
}

bool
dulge_array_get_int8(dulge_array_t a, unsigned int i, int8_t *v)
{
	return prop_array_get_int8(a, i, v);
}

bool
dulge_array_get_uint8(dulge_array_t a, unsigned int i, uint8_t *v)
{
	return prop_array_get_uint8(a, i, v);
}

bool
dulge_array_set_int8(dulge_array_t a, unsigned int i, int8_t v)
{
	return prop_array_set_int8(a, i, v);
}

bool
dulge_array_set_uint8(dulge_array_t a, unsigned int i, uint8_t v)
{
	return prop_array_set_uint8(a, i, v);
}

bool
dulge_array_get_int16(dulge_array_t a, unsigned int i, int16_t *v)
{
	return prop_array_get_int16(a, i, v);
}

bool
dulge_array_get_uint16(dulge_array_t a, unsigned int i, uint16_t *v)
{
	return prop_array_get_uint16(a, i, v);
}

bool
dulge_array_set_int16(dulge_array_t a, unsigned int i, int16_t v)
{
	return prop_array_set_int16(a, i, v);
}

bool
dulge_array_set_uint16(dulge_array_t a, unsigned int i, uint16_t v)
{
	return prop_array_set_uint16(a, i, v);
}

bool
dulge_array_get_int32(dulge_array_t a, unsigned int i, int32_t *v)
{
	return prop_array_get_int32(a, i, v);
}

bool
dulge_array_get_uint32(dulge_array_t a, unsigned int i, uint32_t *v)
{
	return prop_array_get_uint32(a, i, v);
}

bool
dulge_array_set_int32(dulge_array_t a, unsigned int i, int32_t v)
{
	return prop_array_set_int32(a, i, v);
}

bool
dulge_array_set_uint32(dulge_array_t a, unsigned int i, uint32_t v)
{
	return prop_array_set_uint32(a, i, v);
}

bool
dulge_array_get_int64(dulge_array_t a, unsigned int i, int64_t *v)
{
	return prop_array_get_int64(a, i, v);
}

bool
dulge_array_get_uint64(dulge_array_t a, unsigned int i, uint64_t *v)
{
	return prop_array_get_uint64(a, i, v);
}

bool
dulge_array_set_int64(dulge_array_t a, unsigned int i, int64_t v)
{
	return prop_array_set_int64(a, i, v);
}

bool
dulge_array_set_uint64(dulge_array_t a, unsigned int i, uint64_t v)
{
	return prop_array_set_uint64(a, i, v);
}

bool
dulge_array_add_int8(dulge_array_t a, int8_t v)
{
	return prop_array_add_int8(a, v);
}

bool
dulge_array_add_uint8(dulge_array_t a, uint8_t v)
{
	return prop_array_add_uint8(a, v);
}

bool
dulge_array_add_int16(dulge_array_t a, int16_t v)
{
	return prop_array_add_int16(a, v);
}

bool
dulge_array_add_uint16(dulge_array_t a, uint16_t v)
{
	return prop_array_add_uint16(a, v);
}

bool
dulge_array_add_int32(dulge_array_t a, int32_t v)
{
	return prop_array_add_int32(a, v);
}

bool
dulge_array_add_uint32(dulge_array_t a, uint32_t v)
{
	return prop_array_add_uint32(a, v);
}

bool
dulge_array_add_int64(dulge_array_t a, int64_t v)
{
	return prop_array_add_int64(a, v);
}

bool
dulge_array_add_uint64(dulge_array_t a, uint64_t v)
{
	return prop_array_add_uint64(a, v);
}

bool
dulge_array_get_cstring(dulge_array_t a, unsigned int i, char **s)
{
	return prop_array_get_cstring(a, i, s);
}

bool
dulge_array_set_cstring(dulge_array_t a, unsigned int i, const char *s)
{
	return prop_array_set_cstring(a, i, s);
}

bool
dulge_array_add_cstring(dulge_array_t a, const char *s)
{
	return prop_array_add_cstring(a, s);
}

bool
dulge_array_add_cstring_nocopy(dulge_array_t a, const char *s)
{
	return prop_array_add_cstring_nocopy(a, s);
}

bool
dulge_array_get_cstring_nocopy(dulge_array_t a, unsigned int i, const char **s)
{
	return prop_array_get_cstring_nocopy(a, i, s);
}

bool
dulge_array_set_cstring_nocopy(dulge_array_t a, unsigned int i, const char *s)
{
	return prop_array_set_cstring_nocopy(a, i, s);
}

bool
dulge_array_add_and_rel(dulge_array_t a, dulge_object_t o)
{
	return prop_array_add_and_rel(a, o);
}

/* prop_bool */

dulge_bool_t
dulge_bool_create(bool v)
{
	return prop_bool_create(v);
}

dulge_bool_t
dulge_bool_copy(dulge_bool_t b)
{
	return prop_bool_copy(b);
}

bool
dulge_bool_true(dulge_bool_t b)
{
	return prop_bool_true(b);
}

bool
dulge_bool_equals(dulge_bool_t a, dulge_bool_t b)
{
	return prop_bool_equals(a, b);
}

/* prop_data */

dulge_data_t
dulge_data_create_data(const void *v, size_t s)
{
	return prop_data_create_data(v, s);
}

dulge_data_t
dulge_data_create_data_nocopy(const void *v, size_t s)
{
	return prop_data_create_data_nocopy(v, s);
}

dulge_data_t
dulge_data_copy(dulge_data_t d)
{
	return prop_data_copy(d);
}

size_t
dulge_data_size(dulge_data_t d)
{
	return prop_data_size(d);
}

void *
dulge_data_data(dulge_data_t d)
{
	return prop_data_data(d);
}

const void *
dulge_data_data_nocopy(dulge_data_t d)
{
	return prop_data_data_nocopy(d);
}

bool
dulge_data_equals(dulge_data_t a, dulge_data_t b)
{
	return prop_data_equals(a, b);
}

bool
dulge_data_equals_data(dulge_data_t d, const void *v, size_t s)
{
	return prop_data_equals_data(d, v, s);
}

/* prop_dictionary */

dulge_dictionary_t
dulge_dictionary_create(void)
{
	return prop_dictionary_create();
}

dulge_dictionary_t
dulge_dictionary_create_with_capacity(unsigned int i)
{
	return prop_dictionary_create_with_capacity(i);
}

dulge_dictionary_t
dulge_dictionary_copy(dulge_dictionary_t d)
{
	return prop_dictionary_copy(d);
}

dulge_dictionary_t
dulge_dictionary_copy_mutable(dulge_dictionary_t d)
{
	return prop_dictionary_copy_mutable(d);
}

unsigned int
dulge_dictionary_count(dulge_dictionary_t d)
{
	return prop_dictionary_count(d);
}

bool
dulge_dictionary_ensure_capacity(dulge_dictionary_t d, unsigned int i)
{
	return prop_dictionary_ensure_capacity(d, i);
}

void
dulge_dictionary_make_immutable(dulge_dictionary_t d)
{
	prop_dictionary_make_immutable(d);
}

dulge_object_iterator_t
dulge_dictionary_iterator(dulge_dictionary_t d)
{
	return prop_dictionary_iterator(d);
}

dulge_array_t
dulge_dictionary_all_keys(dulge_dictionary_t d)
{
	return prop_dictionary_all_keys(d);
}

dulge_object_t
dulge_dictionary_get(dulge_dictionary_t d, const char *s)
{
	return prop_dictionary_get(d, s);
}

bool
dulge_dictionary_set(dulge_dictionary_t d, const char *s, dulge_object_t o)
{
	return prop_dictionary_set(d, s, o);
}

void
dulge_dictionary_remove(dulge_dictionary_t d, const char *s)
{
	prop_dictionary_remove(d, s);
}

dulge_object_t
dulge_dictionary_get_keysym(dulge_dictionary_t d, dulge_dictionary_keysym_t k)
{
	return prop_dictionary_get_keysym(d, k);
}

bool
dulge_dictionary_set_keysym(dulge_dictionary_t d, dulge_dictionary_keysym_t k,
					   dulge_object_t o)
{
	return prop_dictionary_set_keysym(d, k, o);
}

void
dulge_dictionary_remove_keysym(dulge_dictionary_t d, dulge_dictionary_keysym_t k)
{
	prop_dictionary_remove_keysym(d, k);
}

bool
dulge_dictionary_equals(dulge_dictionary_t a, dulge_dictionary_t b)
{
	return prop_dictionary_equals(a, b);
}

char *
dulge_dictionary_externalize(dulge_dictionary_t d)
{
	return prop_dictionary_externalize(d);
}

dulge_dictionary_t
dulge_dictionary_internalize(const char *s)
{
	return prop_dictionary_internalize(s);
}

bool
dulge_dictionary_externalize_to_file(dulge_dictionary_t d, const char *s)
{
	return prop_dictionary_externalize_to_file(d, s);
}

bool
dulge_dictionary_externalize_to_zfile(dulge_dictionary_t d, const char *s)
{
	return prop_dictionary_externalize_to_zfile(d, s);
}

dulge_dictionary_t
dulge_dictionary_internalize_from_file(const char *s)
{
	return prop_dictionary_internalize_from_file(s);
}

dulge_dictionary_t
dulge_dictionary_internalize_from_zfile(const char *s)
{
	return prop_dictionary_internalize_from_zfile(s);
}

const char *
dulge_dictionary_keysym_cstring_nocopy(dulge_dictionary_keysym_t k)
{
	return prop_dictionary_keysym_cstring_nocopy(k);
}

bool
dulge_dictionary_keysym_equals(dulge_dictionary_keysym_t a, dulge_dictionary_keysym_t b)
{
	return prop_dictionary_keysym_equals(a, b);
}

/*
 * Utility routines to make it more convenient to work with values
 * stored in dictionaries.
 */
bool
dulge_dictionary_get_dict(dulge_dictionary_t d, const char *s,
					 dulge_dictionary_t *rd)
{
	return prop_dictionary_get_dict(d, s, rd);
}

bool
dulge_dictionary_get_bool(dulge_dictionary_t d, const char *s, bool *b)
{
	return prop_dictionary_get_bool(d, s, b);
}

bool
dulge_dictionary_set_bool(dulge_dictionary_t d, const char *s, bool b)
{
	return prop_dictionary_set_bool(d, s, b);
}

bool
dulge_dictionary_get_int8(dulge_dictionary_t d, const char *s, int8_t *v)
{
	return prop_dictionary_get_int8(d, s, v);
}

bool
dulge_dictionary_get_uint8(dulge_dictionary_t d, const char *s, uint8_t *v)
{
	return prop_dictionary_get_uint8(d, s, v);
}

bool
dulge_dictionary_set_int8(dulge_dictionary_t d, const char *s, int8_t v)
{
	return prop_dictionary_set_int8(d, s, v);
}

bool
dulge_dictionary_set_uint8(dulge_dictionary_t d, const char *s, uint8_t v)
{
	return prop_dictionary_set_uint8(d, s, v);
}

bool
dulge_dictionary_get_int16(dulge_dictionary_t d, const char *s, int16_t *v)
{
	return prop_dictionary_get_int16(d, s, v);
}

bool
dulge_dictionary_get_uint16(dulge_dictionary_t d, const char *s, uint16_t *v)
{
	return prop_dictionary_get_uint16(d, s, v);
}

bool
dulge_dictionary_set_int16(dulge_dictionary_t d, const char *s, int16_t v)
{
	return prop_dictionary_set_int16(d, s, v);
}

bool
dulge_dictionary_set_uint16(dulge_dictionary_t d, const char *s, uint16_t v)
{
	return prop_dictionary_set_uint16(d, s, v);
}

bool
dulge_dictionary_get_int32(dulge_dictionary_t d, const char *s, int32_t *v)
{
	return prop_dictionary_get_int32(d, s, v);
}

bool
dulge_dictionary_get_uint32(dulge_dictionary_t d, const char *s, uint32_t *v)
{
	return prop_dictionary_get_uint32(d, s, v);
}

bool
dulge_dictionary_set_int32(dulge_dictionary_t d, const char *s, int32_t v)
{
	return prop_dictionary_set_int32(d, s, v);
}

bool
dulge_dictionary_set_uint32(dulge_dictionary_t d, const char *s, uint32_t v)
{
	return prop_dictionary_set_uint32(d, s, v);
}

bool
dulge_dictionary_get_int64(dulge_dictionary_t d, const char *s, int64_t *v)
{
	return prop_dictionary_get_int64(d, s, v);
}

bool
dulge_dictionary_get_uint64(dulge_dictionary_t d, const char *s, uint64_t *v)
{
	return prop_dictionary_get_uint64(d, s, v);
}

bool
dulge_dictionary_set_int64(dulge_dictionary_t d, const char *s, int64_t v)
{
	return prop_dictionary_set_int64(d, s, v);
}

bool
dulge_dictionary_set_uint64(dulge_dictionary_t d, const char *s, uint64_t v)
{
	return prop_dictionary_set_uint64(d, s, v);
}

bool
dulge_dictionary_get_cstring(dulge_dictionary_t d, const char *s, char **ss)
{
	return prop_dictionary_get_cstring(d, s, ss);
}

bool
dulge_dictionary_set_cstring(dulge_dictionary_t d, const char *s, const char *ss)
{
	return prop_dictionary_set_cstring(d, s, ss);
}

bool
dulge_dictionary_get_cstring_nocopy(dulge_dictionary_t d, const char *s, const char **ss)
{
	return prop_dictionary_get_cstring_nocopy(d, s, ss);
}

bool
dulge_dictionary_set_cstring_nocopy(dulge_dictionary_t d, const char *s, const char *ss)
{
	return prop_dictionary_set_cstring_nocopy(d, s, ss);
}

bool
dulge_dictionary_set_and_rel(dulge_dictionary_t d, const char *s, dulge_object_t o)
{
	return prop_dictionary_set_and_rel(d, s, o);
}

/* prop_number */

dulge_number_t
dulge_number_create_integer(int64_t v)
{
	return prop_number_create_integer(v);
}

dulge_number_t
dulge_number_create_unsigned_integer(uint64_t v)
{
	return prop_number_create_unsigned_integer(v);
}

dulge_number_t
dulge_number_copy(dulge_number_t n)
{
	return prop_number_copy(n);
}

int
dulge_number_size(dulge_number_t n)
{
	return prop_number_size(n);
}

bool
dulge_number_unsigned(dulge_number_t n)
{
	return prop_number_unsigned(n);
}

int64_t
dulge_number_integer_value(dulge_number_t n)
{
	return prop_number_integer_value(n);
}

uint64_t
dulge_number_unsigned_integer_value(dulge_number_t n)
{
	return prop_number_unsigned_integer_value(n);
}

bool
dulge_number_equals(dulge_number_t n, dulge_number_t nn)
{
	return prop_number_equals(n, nn);
}

bool
dulge_number_equals_integer(dulge_number_t n, int64_t v)
{
	return prop_number_equals_integer(n, v);
}

bool
dulge_number_equals_unsigned_integer(dulge_number_t n, uint64_t v)
{
	return prop_number_equals_unsigned_integer(n, v);
}

/* prop_object */

void
dulge_object_retain(dulge_object_t o)
{
	prop_object_retain(o);
}

void
dulge_object_release(dulge_object_t o)
{
	prop_object_release(o);
}

dulge_type_t
dulge_object_type(dulge_object_t o)
{
	return (dulge_type_t)prop_object_type(o);
}

bool
dulge_object_equals(dulge_object_t o, dulge_object_t oo)
{
	return prop_object_equals(o, oo);
}

bool
dulge_object_equals_with_error(dulge_object_t o, dulge_object_t oo, bool *b)
{
	return prop_object_equals_with_error(o, oo, b);
}

dulge_object_t
dulge_object_iterator_next(dulge_object_iterator_t o)
{
	return prop_object_iterator_next(o);
}

void
dulge_object_iterator_reset(dulge_object_iterator_t o)
{
	prop_object_iterator_reset(o);
}

void
dulge_object_iterator_release(dulge_object_iterator_t o)
{
	prop_object_iterator_release(o);
}

/* prop_string */

dulge_string_t
dulge_string_create(void)
{
	return prop_string_create();
}

dulge_string_t
dulge_string_create_cstring(const char *s)
{
	return prop_string_create_cstring(s);
}

dulge_string_t
dulge_string_create_cstring_nocopy(const char *s)
{
	return prop_string_create_cstring_nocopy(s);
}

dulge_string_t
dulge_string_copy(dulge_string_t s)
{
	return prop_string_copy(s);
}

dulge_string_t
dulge_string_copy_mutable(dulge_string_t s)
{
	return prop_string_copy_mutable(s);
}

size_t
dulge_string_size(dulge_string_t s)
{
	return prop_string_size(s);
}

bool
dulge_string_mutable(dulge_string_t s)
{
	return prop_string_mutable(s);
}

char *
dulge_string_cstring(dulge_string_t s)
{
	return prop_string_cstring(s);
}

const char *
dulge_string_cstring_nocopy(dulge_string_t s)
{
	return prop_string_cstring_nocopy(s);
}

bool
dulge_string_append(dulge_string_t s, dulge_string_t ss)
{
	return prop_string_append(s, ss);
}

bool
dulge_string_append_cstring(dulge_string_t s, const char *ss)
{
	return prop_string_append_cstring(s, ss);
}

bool
dulge_string_equals(dulge_string_t s, dulge_string_t ss)
{
	return prop_string_equals(s, ss);
}

bool
dulge_string_equals_cstring(dulge_string_t s, const char *ss)
{
	return prop_string_equals_cstring(s, ss);
}

/* dulge specific helpers */
dulge_array_t
dulge_plist_array_from_file(const char *path)
{
	dulge_array_t a;

	a = dulge_array_internalize_from_zfile(path);
	if (dulge_object_type(a) != DULGE_TYPE_ARRAY) {
		dulge_dbg_printf(
		    "dulge: failed to internalize array from %s\n", path);
	}
	return a;
}

dulge_dictionary_t
dulge_plist_dictionary_from_file(const char *path)
{
	dulge_dictionary_t d;

	d = dulge_dictionary_internalize_from_zfile(path);
	if (dulge_object_type(d) != DULGE_TYPE_DICTIONARY) {
		dulge_dbg_printf(
		    "dulge: failed to internalize dict from %s\n", path);
	}
	return d;
}
