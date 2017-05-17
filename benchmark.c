/*
 * Benchmark for HTML parsers, testing different HTML parsers on a large volume of HTML files
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#define _DEFAULT_SOURCE
#include "benchmark.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Tests
#include "test_dummy.h"
#include "test_hsp.h"
#include "test_libxml2.h"
#include "test_gumbo.h"
#include "test_myhtml.h"
#include "test_haut.h"

const char progress[4] = { '/', '-', '\\', '|' };

double
div_by_largest_prefix( size_t size ) {
    double s =size;
    while( s > 1024.0 ) s /= 1024.0;
    return s;
}

const char*
str_largest_prefix( size_t size ) {
    static char* prefix[] = { "B", "KB", "MB", "GB", "TB", "PB" };
    int i =0;
    while( size > 1024) {
        size /= 1024;
        i++;
    }
    return prefix[i];
}

const char* 
str_parser( parser_t p ) {
    switch( p ) {
        case PARSER_DUMMY: return "dummy";
        case PARSER_HSP: return "htmlstreamparser";
        case PARSER_LIBXML2: return "libxml2";
        case PARSER_GUMBO: return "gumbo-parser";
        case PARSER_MYHTML: return "MyHTML";
        case PARSER_HUBBUB: return "hubbub";
        case PARSER_HAUT: return "haut";
        default: break;
    }
    return "";
}

/* Reads the entire file `filename' and copies a pointer to the buffer into the testset at index i */
int
testSetAppend( testset_t *testset, size_t index, const char* filename ) {
    FILE *file = fopen( filename, "rb" );
    if( file == NULL )
        return -1;
    
    fseek( file, 0, SEEK_END );
    long size = ftell(file);
    fseek( file, 0, SEEK_SET );
    if( size < 1 ) {
        fclose( file );
        return -1;
    }

    char *buf = malloc( size + 1 );
    fread( buf, size, 1, file );
    // We add a \0 for some parsers require it
    buf[size] = 0;

    testset->data[index].buffer =buf;
    testset->data[index].size =size + 1;
    testset->set_size +=size + 1;

    // Also copy filename
#ifdef DEBUG_PRINT_FILENAME
    buf =malloc( strlen(filename) + 1 );
    strcpy( buf, filename );
    testset->data[index].path =buf;
#endif

    fclose( file );
    return 0;
}

int
benchmark_loadTestSet( testset_t *testset, const char* path ) {
    testset->data =NULL;
    testset->count =0;
    testset->set_size =0;
    size_t index =0;

    DIR *dir =opendir( path );
    if( dir == NULL ) {
        fprintf( stderr, "%s\n", strerror( errno ) );
        return -1;
    }

    struct dirent *entry;
    struct stat sb;

    // We count all the entries first, so we can preallocate the array
    while( (entry = readdir( dir )) != NULL ) {
        if( entry->d_type != DT_DIR ) 
            testset->count++;
    }

    if( testset->count == 0 )
        return -1;

    // Allocate the array of buffers
    testset->data =malloc( testset->count * sizeof( testset_buffer_t ) );
    if( testset->data == NULL ) {
        fprintf( stderr, "malloc() failed in benchmark_loadTestSet()\n" );
        return -1;
    }
    memset( testset->data, 0, testset->count * sizeof( testset_buffer_t ) );

    rewinddir( dir );

    char fullpath[PATH_MAX];
    strcpy( fullpath, path );
    int pathlen =strlen( path );
    if( fullpath[pathlen-1] != '/' ) {
        fullpath[pathlen++] ='/';
    }

    // Iterate through the directory again and load every file into the test set
    while( index < testset->count && (entry = readdir( dir )) != NULL ) {

        fprintf( stderr, "Reading test data ... %c\r", progress[index%4] );

        strcpy( fullpath + pathlen, entry->d_name );
        if( stat( fullpath, &sb ) == 0 && S_ISREG( sb.st_mode ) ) {
            //printf( "---- %s\n", fullpath );
            if( testSetAppend( testset, index, fullpath ) != 0 ) {
                fprintf( stderr, "Warning: could not open `%s' for reading.\n", entry->d_name );
            } else
                index++;
        }
    }

    testset->count =index;

    closedir( dir );
    return 0;
}

void
benchmark_freeTestSet( testset_t *testset) {
    for( size_t i =0; i < testset->count; i++ ) {
        if( testset->data[i].buffer != NULL )
            free( testset->data[i].buffer );
    }
    free( testset->data );
    testset->count =testset->set_size =0;
}

testresult_t*
benchmark_performTests( testset_t* testset, int parsers ) {
    // Compute the number of parsers to test
    // Nicely obscure and non-portable
    int n= __builtin_popcount( parsers ); // requires GCC >= 3.4
    int index=0;

    if( n < 1 ) return NULL;


    testresult_t* results =malloc( (n+1) * sizeof( testresult_t ) );
    results[n].parser =PARSER_NONE;

    for( int i=0; i < 32; i++ ) {
        int p =1 << i;
        if( parsers & p )
            benchmark_performTest( &results[index++], testset, (parser_t)p );

    }
    return results;
}

int 
benchmark_performTest( testresult_t* result, testset_t* testset, parser_t parser ) {
    // Reset the result record
    result->total_tests =testset->count;
    result->total_href =result->total_src =result->total_alt =0;
    result->total_bytes =result->total_innertext_bytes =0;
    result->parser =parser;

    // Call the init routine for the given parser, if any
    switch( parser ) {
        case PARSER_HSP:
            test_hsp_init( );
            break;
        default: break;
    }

    clock_t tic =clock();

    // Run the parser on every buffer
    for( size_t i =0; i < testset->count; i++ ) {
#ifndef DEBUG_PRINT_FILENAME
        fprintf( stderr, "Parsing test set data using `%s' ... %c\r", str_parser( parser ), progress[i%4] );
#else
        printf( "Parsing `%s'\n", testset->data[i].path );
#endif
        switch( parser ) {
            case PARSER_DUMMY:
                test_dummy( result, testset->data[i].buffer, testset->data[i].size );
                break;
            case PARSER_HSP:
                test_hsp( result, testset->data[i].buffer, testset->data[i].size );
                break;
            case PARSER_LIBXML2:
                test_libxml2( result, testset->data[i].buffer, testset->data[i].size );
                break;
            case PARSER_GUMBO:
                test_gumbo( result, testset->data[i].buffer, testset->data[i].size );
                break;
            case PARSER_MYHTML:
                test_myhtml( result, testset->data[i].buffer, testset->data[i].size );
                break;
            case PARSER_HAUT:
                test_haut( result, testset->data[i].buffer, testset->data[i].size );
                break;
            default: break;
        }
    }
    fprintf( stderr, "Parsing test set data using `%s' ... done\n", str_parser( parser ) );

    clock_t toc =clock();
    result->total_time =(double)(toc - tic) / CLOCKS_PER_SEC;
    
    // Call the cleanup routine for the given parser, if any
    switch( parser ) {
        case PARSER_HSP:
            test_hsp_cleanup( );
            break;
        default: break;
    }
    return 0;
}

