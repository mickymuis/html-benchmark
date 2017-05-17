/*
 * Benchmark for HTML parsers, testing different HTML parsers on a large volume of HTML files
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <sys/types.h>
#include <inttypes.h>

//#define DEBUG_PRINT_FILENAME

typedef enum {
    PARSER_NONE =0,
    PARSER_DUMMY =1,  // baseline, no parsing
    PARSER_HSP =1<<1,    // htmlstreamparser
    PARSER_LIBXML2 =1<<2,// HTMLParser module from libxml2
    PARSER_GUMBO =1<<3,  // Google's gumbo-parser
    PARSER_MYHTML=1<<4, // MyHTML parser
    PARSER_HAUT=1<<5,
    PARSER_HUBBUB=1<<6, // Hubbub parser / disabled
    PARSER_ALL =(1<<6)-1
} parser_t;

typedef struct {
    char* buffer;
    size_t size;
#ifdef DEBUG_PRINT_FILENAME
    char* path;
#endif
} testset_buffer_t;

typedef struct {
    testset_buffer_t* data;
    size_t count;
    size_t set_size;
} testset_t;

struct testresult {
    double total_time;
    size_t total_bytes;
    size_t total_tests;
    uint64_t total_href;
    uint64_t total_src;
    uint64_t total_alt;
    size_t total_innertext_bytes;
    parser_t parser;
}; 

typedef struct testresult testresult_t;

double
div_by_largest_prefix( size_t );

const char*
str_largest_prefix( size_t );

const char* 
str_parser( parser_t p );

int
benchmark_loadTestSet( testset_t* testset, const char* path );

void
benchmark_freeTestSet( testset_t* testset);

testresult_t*
benchmark_performTests( testset_t* testset, int parsers );

int 
benchmark_performTest( testresult_t* result, testset_t* testset, parser_t parser );

#endif
