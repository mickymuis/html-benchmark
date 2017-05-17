/*
 * Benchmark for HTML parsers, testing different HTML parsers on a large volume of HTML files
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#include "benchmark.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

void 
print_help( const char *exec ) {
    printf( "Usage: %s <DIRECTORY> [PARSER ...]\n\n\
Benchmark for HTML parsers, testing different HTML parsers on a large volume of HTML files.\n\
\n\
One or more of the following parsers can be specified \n\
\tall \t Run all parsers on the test directory (default)\n\
\tdummy \t null-parser, only iterates through all buffers\n\
\thsp \t htmlstreamparser\n\
\tlibxml2 \t HTMLParser module from libxml2\n\
\tgumbo \t Google's gumbo-parser\n\
\tmyhtml \t MyHTML parser\n\
\thaut \t Haut-HTML parser\n", exec );

}

void
print_results( testresult_t* results ) {
    int i=0;

    while( results[i].parser != PARSER_NONE ) {
        printf( "\
==========================================================================\n\
 Results for `%s'\n\n\
 Total CPU time: %.2f s\n\
 Average CPU time: %.2f ms per file\n\
 Memory allocated: %.1f %s\n\
 Total collected innertext: %.1f %s\n\
 Collected: %lu HREF's, %lu IMG-SRC's, %lu IMG-ALT's\n\
--------------------------------------------------------------------------\n\n",
            str_parser( results[i].parser ),
            results[i].total_time,
            (results[i].total_time / results[i].total_tests) * 1000.0,
            div_by_largest_prefix( results[i].total_bytes ), str_largest_prefix( results[i].total_bytes ),
            div_by_largest_prefix( results[i].total_innertext_bytes ), str_largest_prefix( results[i].total_innertext_bytes ),
            results[i].total_href, results[i].total_src, results[i].total_alt  );

        i++;
    }
}

int
main( int argc, char** argv ) {
    if( argc < 2 ) {
        print_help( argv[0] );
        return 0;
    }

    const char* test_dir = argv[1];

    struct stat sb;
    if( stat( test_dir, &sb ) != 0 || !S_ISDIR( sb.st_mode ) ) {
        fprintf( stderr, "Specified path `%s' is not a directory.\n", test_dir );
        return -1;
    }

    int parsers =PARSER_ALL;
    if( argc > 2 )
        parsers =PARSER_NONE;

    for( int i =2; i < argc; i++ ) {
        if( strcmp( argv[i], "all" ) == 0 )
            parsers |= PARSER_ALL;
        else if( strcmp( argv[i], "hsp" ) == 0 )
            parsers |= PARSER_HSP;
        else if( strcmp( argv[i], "libxml2" ) == 0 )
            parsers |= PARSER_LIBXML2;
        else if( strcmp( argv[i], "gumbo" ) == 0 )
            parsers |= PARSER_GUMBO;
        else if( strcmp( argv[i], "myhtml" ) == 0 )
            parsers |= PARSER_MYHTML;
        else if( strcmp( argv[i], "hubbub" ) == 0 )
            parsers |= PARSER_HUBBUB;
        else if( strcmp( argv[i], "dummy" ) == 0 )
            parsers |= PARSER_DUMMY;
        else if( strcmp( argv[i], "haut" ) == 0 )
            parsers |= PARSER_HAUT;
        else {
            fprintf( stderr, "Unrecognized option: `%s'\n", argv[i] );
            return -1;
        }
    }

    testset_t testset;
    if( benchmark_loadTestSet( &testset, test_dir ) != 0 ) {
        fprintf( stderr, "Unable to load test set from directory `%s'\n", test_dir );
        return -1;
    } else {
        printf( "Loaded test set: %zu files loaded with a total size of %.1f %s.\n", 
                testset.count, 
                div_by_largest_prefix( testset.set_size ), str_largest_prefix( testset.set_size ) );
    }

    testresult_t* results =benchmark_performTests( &testset, parsers );

    print_results( results );

    free( results );

    benchmark_freeTestSet( &testset );

    return 0;
}
