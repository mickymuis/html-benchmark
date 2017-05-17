/*
 * Benchmark for HTML parsers, testing different HTML parsers on a large volume of HTML files
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#ifndef TEST_HAUT_H
#define TEST_HAUT_H

#include "benchmark.h"
#include <sys/types.h>
#include <inttypes.h>

int
test_haut( testresult_t* result, const char* htmlpage, size_t length );

#endif

