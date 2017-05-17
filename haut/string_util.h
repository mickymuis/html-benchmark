/*
 * Haut - a lightweight html tokenizer
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <sys/types.h>

typedef struct {
    const char* data;
    size_t size;
} strfragment_t;

typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} string_t;

typedef string_t* stringptr_t;

void string_init( stringptr_t d );

void string_free( stringptr_t d );

void string_clear( stringptr_t d );

size_t string_reserve( stringptr_t d, size_t add );

void string_append( stringptr_t d, const char* str, size_t len );

void string_swap( stringptr_t str1, stringptr_t str2 );

strfragment_t string_to_fragment( string_t str );

#endif

