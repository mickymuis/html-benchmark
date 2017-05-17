/*
 * Benchmark for HTML parsers, testing different HTML parsers on a large volume of HTML files
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#include "util.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

void
string_init( stringptr_t d ) {
    d->data =malloc( 1 );
    d->data[0] =0;
    d->size =0;
}

void
string_free( stringptr_t d ) {
    free( d->data );
    d->size =0;
}

size_t
string_grow( stringptr_t d, size_t add ) {
    d->data =realloc( d->data, d->size + add + 1);
    assert( d->data != NULL );
    d->size +=add;
    d->data[d->size] =0;
    return d->size;
}

void 
string_append( stringptr_t d, const char* str, size_t len ) {

    size_t offs = d->size;
    string_grow( d, len );
    strncpy( d->data+offs, str, len );
}
