/*
 * Benchmark for HTML parsers, testing different HTML parsers on a large volume of HTML files
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#ifndef TEST_GUMBO_H
#define TEST_GUMBO_H

#include "benchmark.h"

int
test_gumbo( testresult_t* result, const char* htmlpage, size_t length );

#endif

