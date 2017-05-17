/*
 * Benchmark for HTML parsers, testing different HTML parsers on a large volume of HTML files
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#ifndef TEST_HSP_H
#define TEST_HSP_H

#include "benchmark.h"

void
test_hsp_init();

void 
test_hsp_cleanup();

int
test_hsp( testresult_t* result, const char* htmlpage, size_t length );

#endif
