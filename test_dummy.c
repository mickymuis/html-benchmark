/*
 * Benchmark for HTML parsers, testing different HTML parsers on a large volume of HTML files
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#include "test_dummy.h"

int
test_dummy( testresult_t* result, const char* htmlpage, size_t length ) {

    char c =0;
    for( size_t i =0; i < length; i++ )
        c += htmlpage[i]; // so that the compiler doesn't optimize it away

    return c;
}
