/*
 * Benchmark for HTML parsers, testing different HTML parsers on a large volume of HTML files
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#ifndef TEST_DUMMY_H
#define TEST_DUMMY_H

#include "benchmark.h"

int
test_dummy( testresult_t* result, const char* htmlpage, size_t length );

#endif
