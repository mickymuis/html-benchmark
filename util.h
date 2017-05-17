/*
 * Benchmark for HTML parsers, testing different HTML parsers on a large volume of HTML files
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#ifndef UTIL_H
#define UTIL_H

#include <sys/types.h>

typedef struct {
    char *data;
    size_t size;
} string_t;

typedef string_t* stringptr_t;

void string_init( stringptr_t d );

void string_free( stringptr_t d );

size_t string_grow( stringptr_t d, size_t add );

void string_append( stringptr_t d, const char* str, size_t len );

#endif
