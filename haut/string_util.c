/*
 * Haut - a lightweight html tokenizer
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#include "string_util.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define BLOCK_SIZE 64

inline void
string_init( stringptr_t d ) {
    d->data =malloc( BLOCK_SIZE );
    d->data[0] =0;
    d->size =0;
    d->capacity =BLOCK_SIZE;
}

inline void
string_free( stringptr_t d ) {
    free( d->data );
    d->size =0;
}

inline void 
string_clear( stringptr_t d ) {
    if( d->capacity > BLOCK_SIZE ) {
        d->data =realloc( d->data, BLOCK_SIZE );
        assert( d->data != NULL );
        d->capacity =BLOCK_SIZE;
    }
    d->size =0;
    d->data[0] =0;
}

inline size_t
string_reserve( stringptr_t d, size_t add ) {
    if( d->size + add + 1 > d->capacity ) {
        size_t newcap =((d->size + add + 1) / BLOCK_SIZE + 1) * BLOCK_SIZE;
        d->data =realloc( d->data, newcap );
        assert( d->data != NULL );
        d->data[d->size] =0;
        d->capacity =newcap;
    }
    return d->capacity;
}

inline void 
string_append( stringptr_t d, const char* str, size_t len ) {

    string_reserve( d, len );
    strncpy( d->data + d->size, str, len );
    d->size +=len;
    d->data[d->size] =0;
}

inline void 
string_swap( stringptr_t str1, stringptr_t str2 ) {
    string_t tmp =*str1;
    *str1 =*str2;
    *str2 =tmp;
}

inline strfragment_t 
string_to_fragment( string_t str ) {
    strfragment_t frag;
    frag.data =str.data;
    frag.size =str.size;
    return frag;
}

