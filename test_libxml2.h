/*
 * Benchmark for HTML parsers, testing different HTML parsers on a large volume of HTML files
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#ifndef TEST_LIBXML2_H
#define TEST_LIBXML2_H

#include "benchmark.h"

int
test_libxml2( testresult_t* result, const char* htmlpage, size_t length );

#endif
